// csafmt_indent.cpp                                                  -*-C++-*-

#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_filenames.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_util.h>
#include <csabase_visitor.h>
#include <llvm/Support/Regex.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/TokenKinds.h>
#include <clang/Lex/MacroArgs.h>
#include <algorithm>
#include <map>
#include <string>
#include <vector>

#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("indentation");

// ----------------------------------------------------------------------------

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::ast_matchers::internal;
using namespace csabase;

namespace
{

struct indent
{
    indent(int offset = 0);

    bool d_right_justified : 1;
    bool d_accept_any      : 1;
    bool d_macro           : 1;
    bool d_dotdot          : 1;
    int  d_offset;
};

indent::indent(int offset)
: d_right_justified(false)
, d_accept_any(false)
, d_macro(false)
, d_dotdot(false)
, d_offset(offset)
{
}

struct data
    // Data for indentation checking.
{
    typedef std::multimap<unsigned, indent> IndentMap;  // offset -> indent
    std::map<std::string, IndentMap> d_indent;

    typedef std::map<size_t, bool> DoneMap;  // line -> d_done
    std::map<std::string, DoneMap> d_done;

    std::map<std::string, bool> d_in_dotdot;

    typedef std::pair<Decl *, Range> Consecutive;
    std::vector<Consecutive> d_consecutive;
};

struct report : public RecursiveASTVisitor<report>
    // Callback object for indentation checking.
{
    Analyser& d_analyser;  // analyser object
    data& d_data;          // indentation data

    report(Analyser& analyser);
        // Create a 'report' object, accessing the specified 'analyser'.

    void add_indent(SourceLocation sloc, indent ind, bool sing = false);

    void operator()();
        // Check indentation.

    void operator()(const Token &token,
                    const MacroDirective *md,
                    SourceRange,
                    const MacroArgs *);
        // Macro expansion.

    void operator()(SourceRange comment);
        // Comments.

    bool VisitStmt(Stmt *stmt);
    bool VisitDecl(Decl *decl);
    void process(Range r, bool greater);

    bool WalkUpFromCompoundStmt(CompoundStmt *stmt);
    bool WalkUpFromTagDecl(TagDecl *tag);
    bool WalkUpFromFunctionTypeLoc(FunctionTypeLoc func);
    bool WalkUpFromTemplateDecl(TemplateDecl *tplt);
    bool WalkUpFromAccessSpecDecl(AccessSpecDecl *as);
    bool WalkUpFromCallExpr(CallExpr *call);
    bool WalkUpFromParenListExpr(ParenListExpr *list);
    bool WalkUpFromSwitchCase(SwitchCase *sc);

    bool VisitDeclStmt(DeclStmt *ds);
    bool VisitFieldDecl(FieldDecl *fd);

    void do_consecutive();
        // Issue warnings for misaligned declarators and clear the existing
        // declarator set.

    void add_consecutive(Decl *decl, SourceRange sr);
        // Add the specified 'decl' and 'sr' to the accumulating set of
        // consecutive declarators if 'decl' is not null.  If 'decl' is null or
        // not consecutive with the existing set, call 'do_consecutive' to
        // process that set first.
};

report::report(Analyser& analyser)
: d_analyser(analyser)
, d_data(analyser.attachment<data>())
{
}

void report::add_indent(SourceLocation sloc, indent ind, bool sing)
{
    Location loc(d_analyser.manager(), sloc);
    sing = sing || !loc.location().isValid();
    d_data.d_indent[loc.file()].insert(std::make_pair(
        d_analyser.manager().getFileOffset(loc.location()), ind));
    if (sing) {
        ERRS() << loc.file() << " "
               << d_analyser.manager().getFileOffset(loc.location()) << " "
               << ind.d_offset << " " << ind.d_right_justified;
        ERNL();
    }
}

bool report::VisitStmt(Stmt *stmt)
{
    if (d_analyser.is_component(stmt)) {
        Range r(d_analyser.manager(), stmt->getSourceRange());
        if (r) {
            process(r, llvm::dyn_cast<Expr>(stmt) != 0);
        }
    }
    return true;
}

bool report::VisitDecl(Decl *decl)
{
    if (d_analyser.is_component(decl)) {
        Range r(d_analyser.manager(), decl->getSourceRange());
        if (r) {
            process(r, false);
        }
    }
    return true;
}

bool report::WalkUpFromCompoundStmt(CompoundStmt *stmt)
{
    if (!stmt->getLBracLoc().isMacroID()) {
        add_indent(stmt->getLBracLoc().getLocWithOffset(1), +4);
        add_indent(stmt->getRBracLoc(), -4);
    }
    return RecursiveASTVisitor<report>::WalkUpFromCompoundStmt(stmt);
}

bool report::WalkUpFromTagDecl(TagDecl *tag)
{
    if (tag->getRBraceLoc().isValid() && !tag->getRBraceLoc().isMacroID()) {
        add_indent(tag->getLocation().getLocWithOffset(1), +4);
        add_indent(tag->getRBraceLoc(), -4);
    }
    return RecursiveASTVisitor<report>::WalkUpFromTagDecl(tag);
}

bool report::WalkUpFromFunctionTypeLoc(FunctionTypeLoc func)
{
    unsigned n = func.getNumArgs();
    if (n > 0 &&
        func.getLocalRangeBegin().isValid() &&
        !func.getLocalRangeBegin().isMacroID()) {
        Location f(d_analyser.manager(), func.getLocalRangeBegin());
        Location arg1(
            d_analyser.manager(), func.getArg(0)->getLocStart());
        Location argn(
            d_analyser.manager(), func.getArg(n - 1)->getLocStart());

        if (n > 1) {
            bool one_per_line = true;
            bool all_on_one_line = true;
            size_t line = arg1.line();
            SourceLocation bad = arg1.location();
            for (size_t i = 1; i < n; ++i) {
                ParmVarDecl *parm = func.getArg(i);
                Location arg(d_analyser.manager(), parm->getLocStart());
                if (arg.line() != line) {
                    all_on_one_line = false;
                    if (!one_per_line) {
                        bad = arg.location();
                        break;
                    }
                }
                else {
                    one_per_line = false;
                    if (!all_on_one_line) {
                        bad = arg.location();
                        break;
                    }
                }
                line = arg.line();

            }

            if (!one_per_line && !all_on_one_line) {
                d_analyser.report(bad, check_name, "IND02",
                                  "Function parameters should be all on a "
                                  "single line or each on a separate line");
            }
            else if (one_per_line) {
                Location an1;
                for (size_t i = 0; i < n; ++i) {
                    ParmVarDecl *parm = func.getArg(i);
                    if (parm->getIdentifier()) {
                        Location an(d_analyser.manager(), parm->getLocation());
                        if (!an1) {
                            an1 = an;
                        }
                        else if (an.column() != an1.column()) {
                            d_analyser.report(an.location(), check_name,
                                              "IND03",
                                              "Function parameter names "
                                              "should align vertically");
                            d_analyser.report(an1.location(), check_name,
                                              "IND03",
                                              "Starting alignment was here",
                                              false, DiagnosticsEngine::Note);
                            break;
                        }
                    }
                }
            }
        }

        size_t level;
        if (f.line() == arg1.line()) {
            Range tr(d_analyser.manager(),
                     d_analyser.get_trim_line_range(f.location()));
            level = arg1.column() - tr.from().column();
            add_indent(arg1.location(), level);
            add_indent(func.getArg(n - 1)->getLocEnd(), -level);
        } else {
            size_t length = 0;
            for (size_t i = f.line() == arg1.line() ? 1 : 0; i < n; ++i) {
                size_t line_length =
                    llvm::StringRef(d_analyser.get_source_line(
                                        func.getArg(i)->getLocStart()))
                        .trim().size();
                if (length < line_length) {
                    length = line_length;
                }
            }
            level = 79 - length;
            indent in(level);
            in.d_right_justified = true;
            add_indent(arg1.location(), in);
            in.d_right_justified = false;
            in.d_offset = -level;
            add_indent(func.getArg(n - 1)->getLocEnd(), in);
        }
    }
    return RecursiveASTVisitor<report>::WalkUpFromFunctionTypeLoc(func);
}

bool report::WalkUpFromTemplateDecl(TemplateDecl *tplt)
{
    TemplateParameterList *tpl = tplt->getTemplateParameters();
    unsigned n = tpl->size();
    if (n > 0 && !tplt->getLocation().isMacroID()) {
        Location t(d_analyser.manager(), tplt->getLocStart());
        Location l(d_analyser.manager(), tpl->getParam(0)->getLocStart());
        Location r(d_analyser.manager(), tpl->getParam(n - 1)->getLocStart());
        Range tr(d_analyser.manager(),
                 d_analyser.get_trim_line_range(t.location()));
        size_t level =
            t.line() == l.line() ? l.column() - tr.from().column() : 4;
        add_indent(l.location(), level);
        add_indent(tpl->getParam(n - 1)->getLocEnd(), -level);
    }
    return RecursiveASTVisitor<report>::WalkUpFromTemplateDecl(tplt);
}

bool report::WalkUpFromAccessSpecDecl(AccessSpecDecl *as)
{
    if (!as->getLocation().isMacroID()) {
        Location l(d_analyser.manager(), as->getAccessSpecifierLoc());
        add_indent(l.location(), -2);
        add_indent(as->getLocEnd(), +2);
    }
    return RecursiveASTVisitor<report>::WalkUpFromAccessSpecDecl(as);
}

bool report::WalkUpFromCallExpr(CallExpr *call)
{
    unsigned n = call->getNumArgs();
    if (n > 0 &&
        !call->getLocStart().isMacroID() &&
        !llvm::dyn_cast<CXXOperatorCallExpr>(call)) {
        Location c(d_analyser.manager(), call->getLocStart());
        Location l(d_analyser.manager(),
                   call->getArg(0)->getSourceRange().getBegin());
        Range tr(d_analyser.manager(),
                 d_analyser.get_trim_line_range(call->getLocStart()));
        size_t level =
            c.line() == l.line() ? l.column() - tr.from().column() : 4;
        add_indent(c.location().getLocWithOffset(1), level);
        add_indent(call->getRParenLoc(), -level);
    }
    return RecursiveASTVisitor<report>::WalkUpFromCallExpr(call);
}

bool report::WalkUpFromParenListExpr(ParenListExpr *list)
{
    unsigned n = list->getNumExprs();
    if (n > 0 && !list->getLocStart().isMacroID()) {
        Location l(d_analyser.manager(), list->getLParenLoc());
        Location r(d_analyser.manager(), list->getRParenLoc());
        Location a1(d_analyser.manager(), list->getExpr(0)->getLocStart());
        Location an(d_analyser.manager(), list->getExpr(n - 1)->getLocStart());
        if (l.line() == a1.line()) {
            Range tr(d_analyser.manager(),
                     d_analyser.get_trim_line_range(l.location()));
            size_t level = a1.column() - tr.from().column();
            add_indent(a1.location(), level);
            add_indent(r.location(), -level);
        } else {
            size_t length = 0;
            for (size_t i = 0; i < n; ++i) {
                size_t line_length =
                    llvm::StringRef(d_analyser.get_source_line(
                                        list->getExpr(i)->getLocStart()))
                        .trim().size();
                if (length < line_length) {
                    length = line_length;
                }
            }
            if (length + 4 > 79) {
                indent in(std::max(79 - int(length), 0));
                in.d_right_justified = true;
                add_indent(a1.location(), in);
                add_indent(list->getRParenLoc(), -in.d_offset);
            } else {
                add_indent(a1.location(), 4);
                add_indent(list->getRParenLoc(), -4);
            }
        }
    }
    return RecursiveASTVisitor<report>::WalkUpFromParenListExpr(list);
}

bool report::WalkUpFromSwitchCase(SwitchCase *stmt)
{
    if (!stmt->getLocStart().isMacroID()) {
        add_indent(stmt->getKeywordLoc(), -2);
        add_indent(stmt->getColonLoc(), +2);
    }
    if (CompoundStmt *sub = llvm::dyn_cast<CompoundStmt>(stmt->getSubStmt())) {
        add_indent(sub->getLBracLoc().getLocWithOffset(1), -4);
        add_indent(sub->getRBracLoc(), +4);
    }
    return RecursiveASTVisitor<report>::WalkUpFromSwitchCase(stmt);
}

void report::do_consecutive()
{
    if (d_data.d_consecutive.size() > 1) {
        SourceLocation sl = d_data.d_consecutive.front().first->getLocation();
        Location loc(d_analyser.manager(), sl);
        for (size_t i = 1; i < d_data.d_consecutive.size(); ++i) {
            SourceLocation sli = d_data.d_consecutive[i].first->getLocation();
            Location loci(d_analyser.manager(), sli);
            if (loci.column() > loc.column()) {
                sl = sli;
                loc = loci;
            }
        }
        for (size_t i = 0; i < d_data.d_consecutive.size(); ++i) {
            SourceLocation sli = d_data.d_consecutive[i].first->getLocation();
            Location loci(d_analyser.manager(), sli);
            if (loc.column() != loci.column()) {
                d_analyser.report(sli, check_name, "IND04",
                                  "Declarators on consecutive lines must be "
                                  "aligned");
                d_analyser.report(sl, check_name, "IND04",
                                  "Rightmost declarator is here",
                                  false, DiagnosticsEngine::Note);
            }
        }
    }
    d_data.d_consecutive.clear();
}

void report::add_consecutive(Decl *decl, SourceRange sr)
{
    Range r(d_analyser.manager(), sr);
    if (   !decl
        || !r
        || d_data.d_consecutive.size() == 0
        || d_data.d_consecutive.back().second.to().line() + 1 !=
                                                             r.from().line()) {
        do_consecutive();
    }
    if (decl && r) {
        d_data.d_consecutive.push_back(std::make_pair(decl, r));
    }
}

bool report::VisitDeclStmt(DeclStmt *ds)
{
    if (ds->isSingleDecl() &&
        d_analyser.get_parent<Stmt>(ds) ==
        d_analyser.get_parent<CompoundStmt>(ds)) {
        add_consecutive(ds->getSingleDecl(), ds->getSourceRange());
    }

    return true;
}

bool report::VisitFieldDecl(FieldDecl *fd)
{
    add_consecutive(fd, fd->getSourceRange());

    return true;
}

void report::operator()(const Token &token,
                        const MacroDirective *md,
                        SourceRange range,
                        MacroArgs const *args)
{
    Location l(d_analyser.manager(), token.getLocation());
    if (d_analyser.is_component(l.file())) {
        unsigned n;
        if (args && (n = args->getNumArguments()) > 0) {
            const Token *begin = args->getUnexpArgument(0);
            const Token *end = begin + n;
            Location arg(d_analyser.manager(), begin->getLocation());
            std::vector<size_t> levels(1, 4);
            if (l.line() == arg.line()) {
                levels[0] = arg.column() - l.column();
            }
            indent in(levels[0]);
            in.d_macro = true;
            add_indent(arg.location(), in);
            in.d_offset *= -1;
            add_indent(range.getEnd(), in);
        }
    }
}

llvm::Regex outdent ("^ *// *(v-*)[-^]$");
llvm::Regex reindent("^ *// *[-^](-*v)$");

void report::operator()(SourceRange comment)
{
    if (d_analyser.is_test_driver()) {
        llvm::StringRef line = d_analyser.get_source_line(comment.getBegin());
        Location loc(d_analyser.manager(), comment.getBegin());
        llvm::SmallVector<llvm::StringRef, 7> matches;
        if (line == "//..") {
            if (loc.file() == d_analyser.toplevel()) {
                indent ind((d_data.d_in_dotdot[loc.file()] ^= 1) ? +4 : -4);
                ind.d_dotdot = true;
                add_indent(comment.getEnd(), ind);
            }
        } else if (outdent.match(line, &matches)) {
            add_indent(comment.getEnd(), -matches[1].size());
        } else if (reindent.match(line, &matches)) {
            add_indent(comment.getEnd(), matches[1].size());
        }
    }
}

void report::process(Range r, bool greater)
{
    if (!d_data.d_done[r.from().file()][r.from().line()]++) {
        int offset = 0;
        int exact_offset = 0;
        unsigned end = d_analyser.manager().getFileOffset(r.from().location());
        bool dotdot = false;
        for (const auto &i : d_data.d_indent[r.from().file()]) {
            if (end < i.first) {
                break;
            }
            offset += i.second.d_offset;
            dotdot ^= i.second.d_dotdot;
            exact_offset =
                i.second.d_right_justified ? i.second.d_offset : offset;
        }
        llvm::StringRef line = d_analyser.get_source_line(r.from().location());
        if (!dotdot &&
            line.substr(0, r.from().column() - 1)
                               .find_first_not_of(' ') == line.npos &&
            (!greater || line.size() - line.ltrim().size() < exact_offset)) {
            std::string expect =
                std::string(std::max(0,
                                     std::min(79 - int(line.trim().size()),
                                              exact_offset)),
                            ' ') +
                line.trim().str();
            if (line != expect) {
                d_analyser.report(r.from().location(), check_name, "IND01",
                                  "Possibly mis-indented line");
                d_analyser.report(r.from().location(), check_name, "IND01",
                                  "Correct version may be\n%0",
                                  false, DiagnosticsEngine::Note)
                    << expect;
            }
        }
    }
}

void report::operator()()
{
    TraverseDecl(d_analyser.context()->getTranslationUnitDecl());

    // Process remnant declarators.
    add_consecutive(0, SourceRange());
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    analyser.onTranslationUnitDone += report(analyser);
    observer.onMacroExpands += report(analyser);
    observer.onComment += report(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static RegisterCheck c1(check_name, &subscribe);
