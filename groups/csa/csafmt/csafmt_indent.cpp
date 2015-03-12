// csafmt_indent.cpp                                                  -*-C++-*-

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <clang/AST/RecursiveASTVisitor.h>
// IWYU pragma: no_include <clang/AST/DeclNodes.inc>
// IWYU pragma: no_include <clang/AST/StmtNodes.inc>
// IWYU pragma: no_include <clang/AST/TypeNodes.def>
#include <clang/AST/Stmt.h>
#include <clang/AST/TypeLoc.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/MacroArgs.h>
#include <clang/Lex/Token.h>
#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <ext/alloc_traits.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Regex.h>
#include <llvm/Support/raw_ostream.h>
#include <stddef.h>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <algorithm>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace clang { class MacroDirective; }
namespace csabase { class Visitor; }

using namespace clang;
using namespace csabase;

// ----------------------------------------------------------------------------

static std::string const check_name("indentation");

// ----------------------------------------------------------------------------

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

    typedef std::pair<NamedDecl *, Range> Consecutive;
    std::vector<Consecutive> d_consecutive;
};

struct report : public RecursiveASTVisitor<report>, public Report<data>
    // Callback object for indentation checking.
{
    using Report<data>::Report;

    typedef RecursiveASTVisitor<report> Base;

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
    bool WalkUpFromEnumConstantDecl(EnumConstantDecl *decl);
    bool WalkUpFromTagDecl(TagDecl *tag);
    bool WalkUpFromFunctionTypeLoc(FunctionTypeLoc func);
    bool WalkUpFromTemplateDecl(TemplateDecl *tplt);
    bool WalkUpFromAccessSpecDecl(AccessSpecDecl *as);
    bool WalkUpFromCallExpr(CallExpr *call);
    bool WalkUpFromParenListExpr(ParenListExpr *list);
    bool WalkUpFromSwitchCase(SwitchCase *sc);

    bool VisitDeclStmt(DeclStmt *ds);
    bool VisitFieldDecl(FieldDecl *fd);
    bool VisitVarDecl(VarDecl *vd);

    void do_consecutive();
        // Issue warnings for misaligned declarators and clear the existing
        // declarator set.

    void add_consecutive(NamedDecl *decl, SourceRange sr);
        // Add the specified 'decl' and 'sr' to the accumulating set of
        // consecutive declarators if 'decl' is not null.  If 'decl' is null or
        // not consecutive with the existing set, call 'do_consecutive' to
        // process that set first.
};

void report::add_indent(SourceLocation sloc, indent ind, bool sing)
{
    Location loc(m, sloc);
    sing = sing || !loc.location().isValid();
    d.d_indent[loc.file()].insert(
        std::make_pair(m.getFileOffset(loc.location()), ind));
    if (sing) {
        ERRS() << loc.file() << " "
               << m.getFileOffset(loc.location()) << " "
               << ind.d_offset << " " << ind.d_right_justified;
        ERNL();
    }
}

bool report::VisitStmt(Stmt *stmt)
{
    if (a.is_component(stmt)) {
        Range r(m, stmt->getSourceRange());
        if (r) {
            process(r, llvm::dyn_cast<Expr>(stmt) != 0);
        }
    }
    return true;
}

bool report::VisitDecl(Decl *decl)
{
    if (a.is_component(decl)) {
        Range r(m, decl->getSourceRange());
        if (r) {
            process(r, false);
        }
    }
    return true;
}

bool report::WalkUpFromEnumConstantDecl(EnumConstantDecl *decl)
{
    auto tag = llvm::dyn_cast<EnumDecl>(decl->getDeclContext());
    if (*tag->enumerator_begin() == decl) {
        SourceLocation tagloc = tag->getDefinition()->getLocation();
        SourceLocation litloc = decl->getLocation();
        if (tag->getRBraceLoc().isValid() &&
            !tag->getRBraceLoc().isMacroID() &&
            m.getPresumedLineNumber(tagloc) ==
            m.getPresumedLineNumber(litloc)) {
            int indent = m.getPresumedColumnNumber(litloc) -
                         m.getPresumedColumnNumber(tagloc);
            add_indent(tag->getLocation().getLocWithOffset(1), indent - 4);
            add_indent(tag->getRBraceLoc(), 4 - indent);
        }
    }
    return Base::WalkUpFromEnumConstantDecl(decl);
}

bool report::WalkUpFromCompoundStmt(CompoundStmt *stmt)
{
    if (!stmt->getLBracLoc().isMacroID()) {
        add_indent(stmt->getLBracLoc().getLocWithOffset(1), +4);
        add_indent(stmt->getRBracLoc(), -4);
    }
    return Base::WalkUpFromCompoundStmt(stmt);
}

bool report::WalkUpFromTagDecl(TagDecl *tag)
{
    if (tag->getRBraceLoc().isValid() && !tag->getRBraceLoc().isMacroID()) {
        add_indent(tag->getLocation().getLocWithOffset(1), +4);
        add_indent(tag->getRBraceLoc(), -4);
    }
    return Base::WalkUpFromTagDecl(tag);
}

bool report::WalkUpFromFunctionTypeLoc(FunctionTypeLoc func)
{
    unsigned n = func.getNumParams();
    if (n > 0 &&
        func.getLocalRangeBegin().isValid() &&
        !func.getLocalRangeBegin().isMacroID()) {
        Location f(m, func.getLocalRangeBegin());
        Location arg1(m, func.getParam(0)->getLocStart());
        Location argn(m, func.getParam(n - 1)->getLocStart());

        if (n > 1) {
            bool one_per_line = true;
            bool all_on_one_line = true;
            size_t line = arg1.line();
            SourceLocation bad = arg1.location();
            for (size_t i = 1; i < n; ++i) {
                ParmVarDecl *parm = func.getParam(i);
                Location arg(m, parm->getLocStart());
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
                a.report(bad, check_name, "IND02",
                         "Function parameters should be all on a "
                         "single line or each on a separate line");
            }
            else if (one_per_line) {
                Location an1;
                for (size_t i = 0; i < n; ++i) {
                    ParmVarDecl *parm = func.getParam(i);
                    if (parm->getIdentifier()) {
                        Location an(m, parm->getLocation());
                        if (!an1) {
                            an1 = an;
                        }
                        else if (an.column() != an1.column()) {
                            a.report(an.location(), check_name, "IND03",
                                     "Function parameter names "
                                     "should align vertically");
                            a.report(an1.location(), check_name, "IND03",
                                     "Starting alignment was here",
                                     false, DiagnosticIDs::Note);
                            break;
                        }
                    }
                }
            }
        }

        size_t level;
        SourceLocation lpe =
            a.get_trim_line_range(func.getParam(n - 1)->getLocEnd()).getEnd();
        if (f.line() == arg1.line()) {
            Range tr(m, a.get_trim_line_range(f.location()));
            level = arg1.column() - tr.from().column();
            add_indent(arg1.location(), level);
            add_indent(lpe, -level);
        } else {
            size_t length = 0;
            for (size_t i = 0; i < n; ++i) {
                size_t line_length = llvm::StringRef(
                            a.get_source_line(func.getParam(i)->getLocStart()))
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
            add_indent(lpe, in);
        }
    }
    return Base::WalkUpFromFunctionTypeLoc(func);
}

bool report::WalkUpFromTemplateDecl(TemplateDecl *tplt)
{
    TemplateParameterList *tpl = tplt->getTemplateParameters();
    unsigned n = tpl->size();
    if (n > 0 && !tplt->getLocation().isMacroID()) {
        Location t(m, tplt->getLocStart());
        Location l(m, tpl->getParam(0)->getLocStart());
        Location r(m, tpl->getParam(n - 1)->getLocStart());
        Range tr(m, a.get_trim_line_range(t.location()));
        size_t level =
            t.line() == l.line() ? l.column() - tr.from().column() : 4;
        add_indent(l.location(), level);
        add_indent(tpl->getParam(n - 1)->getLocEnd(), -level);
    }
    return Base::WalkUpFromTemplateDecl(tplt);
}

bool report::WalkUpFromAccessSpecDecl(AccessSpecDecl *as)
{
    if (!as->getLocation().isMacroID()) {
        Location l(m, as->getAccessSpecifierLoc());
        add_indent(l.location(), -2);
        add_indent(as->getLocEnd(), +2);
    }
    return Base::WalkUpFromAccessSpecDecl(as);
}

bool report::WalkUpFromCallExpr(CallExpr *call)
{
    unsigned n = call->getNumArgs();
    if (n > 0 &&
        !call->getLocStart().isMacroID() &&
        !llvm::dyn_cast<CXXOperatorCallExpr>(call)) {
        Location c(m, call->getLocStart());
        Location l(m, call->getArg(0)->getSourceRange().getBegin());
        Range tr(m, a.get_trim_line_range(call->getLocStart()));
        size_t level =
            c.line() == l.line() ? l.column() - tr.from().column() : 4;
        add_indent(c.location().getLocWithOffset(1), level);
        add_indent(call->getRParenLoc(), -level);
    }
    return Base::WalkUpFromCallExpr(call);
}

bool report::WalkUpFromParenListExpr(ParenListExpr *list)
{
    unsigned n = list->getNumExprs();
    if (n > 0 && !list->getLocStart().isMacroID()) {
        Location l(m, list->getLParenLoc());
        Location r(m, list->getRParenLoc());
        Location a1(m, list->getExpr(0)->getLocStart());
        Location an(m, list->getExpr(n - 1)->getLocStart());
        if (l.line() == a1.line()) {
            Range tr(m, a.get_trim_line_range(l.location()));
            size_t level = a1.column() - tr.from().column();
            add_indent(a1.location(), level);
            add_indent(r.location(), -level);
        } else {
            size_t length = 0;
            for (size_t i = 0; i < n; ++i) {
                size_t line_length =
                    llvm::StringRef(
                            a.get_source_line(list->getExpr(i)->getLocStart()))
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
    return Base::WalkUpFromParenListExpr(list);
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
    return Base::WalkUpFromSwitchCase(stmt);
}

void report::do_consecutive()
{
    if (d.d_consecutive.size() > 1) {
        NamedDecl *decl = d.d_consecutive.front().first;
        SourceLocation sl = decl->getLocation();
        Location loc(m, sl);
        for (size_t i = 1; i < d.d_consecutive.size(); ++i) {
            NamedDecl *decli = d.d_consecutive[i].first;
            SourceLocation sli = decli->getLocation();
            Location loci(m, sli);
            if (loci.column() > loc.column()) {
                decl = decli;
                sl = sli;
                loc = loci;
            }
        }
        for (size_t i = 0; i < d.d_consecutive.size(); ++i) {
            NamedDecl *decli = d.d_consecutive[i].first;
            SourceLocation sli = decli->getLocation();
            Location loci(m, sli);
            if (loc.column() != loci.column() &&
                loc.column() + decl->getNameAsString().length() !=
                loci.column() + decli->getNameAsString().length()) {
                a.report(sli, check_name, "IND04",
                         "Declarators on consecutive lines must be aligned");
                a.report(sl, check_name, "IND04",
                         "Rightmost declarator is here",
                         false, DiagnosticIDs::Note);
            }
        }
    }
    d.d_consecutive.clear();
}

void report::add_consecutive(NamedDecl *decl, SourceRange sr)
{
    Range r(m, sr);
    if (   !decl
        || !r
        || d.d_consecutive.size() == 0
        || d.d_consecutive.back().second.to().line() + 1 != r.from().line()) {
        do_consecutive();
    }
    if (decl && r && !decl->getLocation().isMacroID()) {
        d.d_consecutive.push_back(std::make_pair(decl, r));
    }
}

bool report::VisitDeclStmt(DeclStmt *ds)
{
    if (a.get_parent<Stmt>(ds) == a.get_parent<CompoundStmt>(ds)) {
        DeclStmt::decl_iterator b = ds->decl_begin();
        DeclStmt::decl_iterator e = ds->decl_end();
        while (b != e) {
            if (VarDecl *var = llvm::dyn_cast<VarDecl>(*b++)) {
                add_consecutive(var, ds->getSourceRange());
                break;
            }
        }
    }

    return true;
}

bool report::VisitFieldDecl(FieldDecl *fd)
{
    add_consecutive(fd, fd->getSourceRange());

    return true;
}

bool report::VisitVarDecl(VarDecl *vd)
{
    if (llvm::dyn_cast<NamespaceDecl>(vd->getDeclContext()) ||
        llvm::dyn_cast<LinkageSpecDecl>(vd->getDeclContext()) ||
        llvm::dyn_cast<TranslationUnitDecl>(vd->getDeclContext())) {
        add_consecutive(vd, vd->getSourceRange());
    }

    return true;
}

void report::operator()(const Token &token,
                        const MacroDirective *md,
                        SourceRange range,
                        MacroArgs const *args)
{
    Location l(m, token.getLocation());
    if (a.is_component(l.file())) {
        unsigned n;
        if (args && (n = args->getNumArguments()) > 0) {
            const Token *begin = args->getUnexpArgument(0);
            Location arg(m, begin->getLocation());
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
    if (a.is_test_driver()) {
        llvm::StringRef line = a.get_source_line(comment.getBegin());
        Location loc(m, comment.getBegin());
        llvm::SmallVector<llvm::StringRef, 7> matches;
        if (line == "//..") {
            if (loc.file() == a.toplevel()) {
                indent ind((d.d_in_dotdot[loc.file()] ^= 1) ? +4 : -4);
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
    bool& done = d.d_done[r.from().file()][r.from().line()];
    if (!done) {
        done = true;
        int offset = 0;
        int exact_offset = 0;
        unsigned end = m.getFileOffset(r.from().location());
        bool dotdot = false;
        for (const auto &i : d.d_indent[r.from().file()]) {
            if (end < i.first) {
                break;
            }
            offset += i.second.d_offset;
            dotdot ^= i.second.d_dotdot;
            exact_offset =
                i.second.d_right_justified ? i.second.d_offset : offset;
        }
        llvm::StringRef line = a.get_source_line(r.from().location());
        if (!dotdot &&
            line.substr(0, r.from().column() - 1)
                               .find_first_not_of(' ') == line.npos &&
            (!greater || line.size() < exact_offset + line.ltrim().size())) {
            std::string expect =
                std::string(std::max(0,
                                     std::min(79 - int(line.trim().size()),
                                              exact_offset)),
                            ' ') +
                line.trim().str();
            if (line != expect) {
                a.report(r.from().location(), check_name, "IND01",
                         "Possibly mis-indented line");
                a.report(r.from().location(), check_name, "IND01",
                         "Correct version may be\n%0",
                         false, DiagnosticIDs::Note)
                    << expect;
            }
        }
    }
}

void report::operator()()
{
    TraverseDecl(a.context()->getTranslationUnitDecl());

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

// ----------------------------------------------------------------------------
// Copyright (C) 2014 Bloomberg Finance L.P.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------- END-OF-FILE ----------------------------------
