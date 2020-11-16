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
#include <csabase_util.h>
#include <csaglb_comments.h>
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

struct PList
{
    virtual ~PList() { }
    virtual unsigned         size(                  ) const = 0;
    virtual const NamedDecl *getParam(unsigned index) const = 0;
};

struct PListFunctionTypeLoc : PList
{
    const FunctionTypeLoc *o;
    PListFunctionTypeLoc(const FunctionTypeLoc *o) : o(o) {                 }
    unsigned         size(              ) const { return o->getNumParams(); }
    const NamedDecl *getParam(unsigned i) const { return o->getParam(i);    }
};

struct PListTemplateParameterList : PList
{
    const TemplateParameterList *o;
    PListTemplateParameterList(const TemplateParameterList *o) : o(o) {  }
    unsigned         size(              ) const { return o->size();      }
    const NamedDecl *getParam(unsigned i) const { return o->getParam(i); }
};

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

    typedef std::map<size_t, std::pair<bool, int> > DoneMap;
                                                        // line -> (done, diff)
    std::map<std::string, DoneMap> d_done;

    std::map<std::string, bool> d_in_dotdot;

    typedef std::pair<NamedDecl *, Range> Consecutive;
    std::vector<Consecutive> d_consecutive;

    typedef std::map<Location, bool> ToProcessMap;
    ToProcessMap d_to_process;
};

struct report : public RecursiveASTVisitor<report>, public Report<data>
    // Callback object for indentation checking.
{
    INHERIT_REPORT_CTOR(report, Report, data);

    typedef RecursiveASTVisitor<report> Base;

    void add_indent(SourceLocation sloc, indent ind, bool sing = false);

    void operator()();
        // Check indentation.

    void operator()(const Token &token,
                    const MacroDirective *md,
                    SourceRange,
                    const MacroArgs *);
        // Macro expansion.

    bool VisitStmt(Stmt *stmt);
    bool VisitDecl(Decl *decl);
    void get_indent(Location l, int& offset, int& exact, bool& dotdot);
    int process(Location l, bool greater, int lastdiff);
    void process_parameter_list(PList          *pl,
                                SourceLocation  sl,
                                const char     *type,
                                const char     *tagline,
                                const char     *tagalign);

    bool WalkUpFromCompoundStmt(CompoundStmt *stmt);
    bool WalkUpFromEnumConstantDecl(EnumConstantDecl *decl);
    bool WalkUpFromDeclaratorDecl(DeclaratorDecl *tag);
    bool WalkUpFromTagDecl(TagDecl *tag);
    bool WalkUpFromFunctionTypeLoc(FunctionTypeLoc func);
    bool WalkUpFromTemplateDecl(TemplateDecl *tplt);
    bool WalkUpFromClassTemplatePartialSpecializationDecl(
                                 ClassTemplatePartialSpecializationDecl *tplt);
    bool WalkUpFromAccessSpecDecl(AccessSpecDecl *as);
    bool WalkUpFromCallExpr(CallExpr *call);
    bool WalkUpFromParenListExpr(ParenListExpr *list);
    bool WalkUpFromSwitchCase(SwitchCase *sc);

    bool VisitDeclStmt(DeclStmt *ds);
    bool VisitFieldDecl(FieldDecl *fd);
    bool VisitVarDecl(VarDecl *vd);
    bool VisitIfStmt(IfStmt *is);

    bool WalkUpFromDoStmt(DoStmt *stmt);
    bool WalkUpFromForStmt(ForStmt *stmt);
    bool WalkUpFromIfStmt(IfStmt *stmt);
    bool WalkUpFromSwitchStmt(SwitchStmt *stmt);
    bool WalkUpFromWhileStmt(WhileStmt *stmt);

    void do_consecutive();
        // Issue warnings for misaligned declarators and clear the existing
        // declarator set.

    bool diff_func_parms(NamedDecl *d1, NamedDecl *d2);
        // Return 'true' if the specified 'd1' is a function parameter in a
        // different function declaration than the specified parameter 'd2'.
        // Note that we compare the parameters by looking at their nesting
        // depth and relative positions, not their declaration contexts,
        // because clang hasn't set those yet at the time this is called.

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
    if (sing && a.is_component(sloc)) {
        ERRS() << loc.file() << "\n"
               << loc.line() << ":" << loc.column() << ":"
               << m.getFileOffset(loc.location()) << " "
               << ind.d_offset << " " << ind.d_right_justified;
        ERNL();
    }
}

bool report::VisitStmt(Stmt *stmt)
{
    if (a.is_component(stmt)) {
        Location l(m, stmt->getBeginLoc());
        if (l) {
            d.d_to_process[l] = llvm::dyn_cast<Expr>(stmt) != 0;
        }
    }
    return true;
}

bool report::VisitIfStmt(IfStmt *stmt)
{
    if (a.is_component(stmt)) {
        Location l(m, stmt->getElseLoc());
        if (l) {
            d.d_to_process[l] = false;
        }
    }
    return true;
}

bool report::VisitDecl(Decl *decl)
{
    if (a.is_component(decl)) {
        Location l(m, decl->getBeginLoc());
        if (l) {
            d.d_to_process[l] = false;
        }
    }
    return true;
}

bool report::WalkUpFromDoStmt(DoStmt *stmt)
{
    if (a.is_component(stmt) && !stmt->getDoLoc().isMacroID()) {
        if (!llvm::dyn_cast<CompoundStmt>(stmt->getBody())) {
            add_indent(stmt->getBody()->getBeginLoc(), +4);
            add_indent(stmt->getWhileLoc(), -4);
        }
    }
    return Base::WalkUpFromDoStmt(stmt);
}

bool report::WalkUpFromForStmt(ForStmt *stmt)
{
    if (a.is_component(stmt) && !stmt->getForLoc().isMacroID()) {
        if (!llvm::dyn_cast<CompoundStmt>(stmt->getBody())) {
            add_indent(stmt->getBody()->getBeginLoc(), +4);
            add_indent(stmt->getEndLoc(), -4);
        }
    }
    return Base::WalkUpFromForStmt(stmt);
}

bool report::WalkUpFromIfStmt(IfStmt *stmt)
{
    if (a.is_component(stmt) && !stmt->getIfLoc().isMacroID()) {
        // 'break' and 'continue' have locEnd equal to locStart
        auto end = Lexer::getLocForEndOfToken(
            stmt->getEndLoc(), 0, m, a.context()->getLangOpts());
        if (!llvm::dyn_cast<CompoundStmt>(stmt->getThen())) {
            add_indent(stmt->getIfLoc().getLocWithOffset(2), +4);
            add_indent(stmt->getElse() ? stmt->getElseLoc() : end, -4);
        }
        if (stmt->getElse() &&
            (!llvm::dyn_cast<CompoundStmt>(stmt->getElse()) &&
             m.getPresumedLineNumber(stmt->getElse()->getBeginLoc()) !=
             m.getPresumedLineNumber(stmt->getElseLoc()))) {
            add_indent(stmt->getElse()->getBeginLoc(), +4);
            add_indent(end, -4);
        }
    }
    return Base::WalkUpFromIfStmt(stmt);
}

bool report::WalkUpFromSwitchStmt(SwitchStmt *stmt)
{
    if (a.is_component(stmt) && !stmt->getSwitchLoc().isMacroID()) {
        if (!llvm::dyn_cast<CompoundStmt>(stmt->getBody())) {
            add_indent(stmt->getBody()->getBeginLoc(), +4);
            add_indent(stmt->getEndLoc(), -4);
        }
    }
    return Base::WalkUpFromSwitchStmt(stmt);
}

bool report::WalkUpFromWhileStmt(WhileStmt *stmt)
{
    if (a.is_component(stmt) && !stmt->getWhileLoc().isMacroID()) {
        if (!llvm::dyn_cast<CompoundStmt>(stmt->getBody())) {
            add_indent(stmt->getBody()->getBeginLoc(), +4);
            add_indent(stmt->getEndLoc(), -4);
        }
    }
    return Base::WalkUpFromWhileStmt(stmt);
}

bool report::WalkUpFromEnumConstantDecl(EnumConstantDecl *decl)
{
    auto tag = llvm::dyn_cast<EnumDecl>(decl->getDeclContext());
    if (*tag->enumerator_begin() == decl) {
        SourceLocation tagloc = tag->getDefinition()->getLocation();
        SourceLocation litloc = decl->getLocation();
        if (tag->getBraceRange().isValid() &&
            !tag->getBraceRange().getEnd().isMacroID() &&
            m.getPresumedLineNumber(tagloc) ==
            m.getPresumedLineNumber(litloc)) {
            int indent = m.getPresumedColumnNumber(litloc) -
                         m.getPresumedColumnNumber(tagloc);
            add_indent(tag->getLocation().getLocWithOffset(1), indent - 4);
            add_indent(tag->getBraceRange().getEnd(), 4 - indent);
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

bool report::WalkUpFromDeclaratorDecl(DeclaratorDecl *decl)
{
    for (unsigned i = 0; i < decl->getNumTemplateParameterLists(); ++i) {
        TemplateParameterList *tpl = decl->getTemplateParameterList(i);
        if (!tpl->getLAngleLoc().isMacroID()) {
            PListTemplateParameterList pl(tpl);
            process_parameter_list(
                &pl, tpl->getLAngleLoc(), "Template", "IND05", "IND06");
        }
    }
    return Base::WalkUpFromDeclaratorDecl(decl);
}

bool report::WalkUpFromTagDecl(TagDecl *tag)
{
    for (unsigned i = 0; i < tag->getNumTemplateParameterLists(); ++i) {
        TemplateParameterList *tpl = tag->getTemplateParameterList(i);
        if (!tpl->getLAngleLoc().isMacroID()) {
            PListTemplateParameterList pl(tpl);
            process_parameter_list(
                &pl, tpl->getLAngleLoc(), "Template", "IND05", "IND06");
        }
    }
    if (tag->getBraceRange().isValid() &&
        !tag->getBraceRange().getEnd().isMacroID()) {
        add_indent(tag->getLocation().getLocWithOffset(1), +4);
        add_indent(tag->getBraceRange().getEnd(), -4);
    }
    return Base::WalkUpFromTagDecl(tag);
}

void report::process_parameter_list(PList          *pl,
                                    SourceLocation  sl,
                                    const char     *type,
                                    const char     *tagline,
                                    const char     *tagalign)
{
    unsigned n = pl->size();

    if (n == 0) {
        return;                                                       // RETURN
    }

    Location f(m, sl);
    Location arg1(m, pl->getParam(0)->getBeginLoc());
    Location argn(m, pl->getParam(n - 1)->getBeginLoc());

    if (n > 1) {
        std::vector<llvm::StringRef> parms(n);
        for (unsigned i = 0; i < n; ++i) {
            auto p = pl->getParam(i);
            if (p->getIdentifier()) {
                parms[i] = p->getName();
            }
        }
        bool one_per_line = true;
        bool all_on_one_line = true;
        size_t line = arg1.line();
        SourceLocation bad = arg1.location();
        for (size_t i = 1; i < n; ++i) {
            auto parm = pl->getParam(i);
            Location arg(m, parm->getBeginLoc());
            if (!are_numeric_cognates(parms[i - 1], parms[i])) {
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
        }

        if (!one_per_line && !all_on_one_line) {
            auto report = a.report(bad, check_name, tagline,
                     "%0 parameters should be all on a "
                     "single line or each on a separate line");
            report << type;
        }
        else if (one_per_line) {
            Location anr;
            Location anp;
            for (size_t i = 0; i < n; ++i) {
                auto parm = pl->getParam(i);
                Location an(m, parm->getLocation());
                if (parm->getIdentifier() &&
                    (i == 0 || anp.line() != an.line()) &&
                    (!anr || anr.column() < an.column())) {
                        anr = an;
                }
                anp = an;
            }
            for (size_t i = 0; i < n; ++i) {
                auto parm = pl->getParam(i);
                Location an(m, parm->getLocation());
                if (parm->getIdentifier() &&
                    (i == 0 || anp.line() != an.line()) &&
                    an.column() != anr.column()) {
                    auto report = a.report(an.location(), check_name, tagalign,
                             "%0 parameter names should align vertically");
                    
                    report << type;
                    a.report(anr.location(), check_name, tagalign,
                             "Rightmost name is here", false,
                             DiagnosticIDs::Note);
                }
                anp = an;
            }
        }
    }

    size_t level;
    SourceLocation lpe =
        a.get_trim_line_range(pl->getParam(n - 1)->getEndLoc()).getEnd();
    if (f.line() == arg1.line() || 0 == strcmp(type, "Template")) {
        Range tr(m, a.get_trim_line_range(f.location()));
        level = arg1.column() - tr.from().column();
        add_indent(arg1.location(), level);
        add_indent(lpe, -level);
    } else {
        size_t length = 0;
        int offset = 0;
        for (size_t i = 0; i < n; ++i) {
            size_t line_length =
                llvm::StringRef(
                    a.get_source_line(pl->getParam(i)->getBeginLoc()))
                    .trim()
                    .size();
            if (length < line_length) {
                length = line_length;
            }
            int line_offset;
            int e;
            bool d;
            get_indent(Location(m, pl->getParam(i)->getBeginLoc()),
                       line_offset, e, d);
            if (offset < line_offset) {
                offset = line_offset;
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

bool report::WalkUpFromFunctionTypeLoc(FunctionTypeLoc func)
{
    SourceLocation sl = func.getLocalRangeBegin();
    if (sl.isValid() && !sl.isMacroID()) {
        PListFunctionTypeLoc pl(&func);
        process_parameter_list(&pl, sl, "Function", "IND02", "IND03");
    }

    return Base::WalkUpFromFunctionTypeLoc(func);
}

bool report::WalkUpFromTemplateDecl(TemplateDecl *tplt)
{
    if (!tplt->getLocation().isMacroID()) {
        TemplateParameterList *tpl = tplt->getTemplateParameters();
        PListTemplateParameterList pl(tpl);

        process_parameter_list(
            &pl, tpl->getLAngleLoc(), "Template", "IND05", "IND06");
    }

    return Base::WalkUpFromTemplateDecl(tplt);
}

bool report::WalkUpFromClassTemplatePartialSpecializationDecl(
                                  ClassTemplatePartialSpecializationDecl *tplt)
{
    if (!tplt->getLocation().isMacroID()) {
        TemplateParameterList *tpl = tplt->getTemplateParameters();
        PListTemplateParameterList pl(tpl);

        process_parameter_list(
            &pl, tpl->getLAngleLoc(), "Template", "IND05", "IND06");
    }

    return Base::WalkUpFromClassTemplatePartialSpecializationDecl(tplt);
}

bool report::WalkUpFromAccessSpecDecl(AccessSpecDecl *as)
{
    if (!as->getLocation().isMacroID()) {
        Location l(m, as->getAccessSpecifierLoc());
        add_indent(l.location(), -2);
        add_indent(as->getEndLoc(), +2);
    }
    return Base::WalkUpFromAccessSpecDecl(as);
}

bool report::WalkUpFromCallExpr(CallExpr *call)
{
    unsigned n = call->getNumArgs();
    while (n > 0 &&
           llvm::dyn_cast<CXXDefaultArgExpr>(call->getArg(n - 1))) {
        --n;
    }
    if (n > 0 &&
        !call->getBeginLoc().isMacroID() &&
        !llvm::dyn_cast<CXXOperatorCallExpr>(call)) {
        Location l(m, call->getBeginLoc());
        Location r(m, call->getRParenLoc());
        Location a1(m, call->getArg(0)->getBeginLoc());
        Location an(m, call->getArg(n - 1)->getBeginLoc());
        int offset, exact_offset;
        bool dotdot;
        get_indent(l, offset, exact_offset, dotdot);
        if (l.line() == a1.line()) {
            size_t level = a1.column() - offset - 1;
            add_indent(a1.location(), level);
            add_indent(r.location(), -level);
        } else {
            size_t length = 0;
            for (size_t i = 0; i < n; ++i) {
                size_t line_length =
                    llvm::StringRef(
                        a.get_source_line(call->getArg(i)->getBeginLoc()))
                        .trim()
                        .size();
                if (length < line_length) {
                    length = line_length;
                }
            }
            if (length + exact_offset + 4 > 79) {
                indent in(std::max(79 - int(length) - exact_offset, 0));
                in.d_right_justified = true;
                add_indent(a1.location(), in);
                add_indent(r.location(), -in.d_offset);
            } else {
                add_indent(a1.location(), 4);
                add_indent(r.location(), -4);
            }
        }
    }
    return Base::WalkUpFromCallExpr(call);
}

bool report::WalkUpFromParenListExpr(ParenListExpr *list)
{
    unsigned n = list->getNumExprs();
    if (n > 0 && !list->getBeginLoc().isMacroID()) {
        Location l(m, list->getLParenLoc());
        Location r(m, list->getRParenLoc());
        Location a1(m, list->getExpr(0)->getBeginLoc());
        Location an(m, list->getExpr(n - 1)->getBeginLoc());
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
                            a.get_source_line(list->getExpr(i)->getBeginLoc()))
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
    if (!stmt->getBeginLoc().isMacroID()) {
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
        llvm::SmallVector<NamedDecl *, 8> decl;
        llvm::SmallVector<SourceLocation, 8> sl;
        llvm::SmallVector<Location, 8> loc;
        size_t line = 0;
        size_t index = 0;
        for (size_t i = 0; i < d.d_consecutive.size(); ++i) {
            NamedDecl *decli = d.d_consecutive[i].first;
            SourceLocation sli = decli->getLocation();
            Location loci(m, sli);
            index = loci.line() == line ? index + 1 : 0;
            line = loci.line();
            if (index >= loc.size()) {
                decl.push_back(decli);
                sl.push_back(sli);
                loc.push_back(loci);
            } else if (loci.column() > loc[index].column()) {
                decl[index] = decli;
                sl[index] = sli;
                loc[index] = loci;
            }
        }
        line = 0;
        for (size_t i = 0; i < d.d_consecutive.size(); ++i) {
            NamedDecl *decli = d.d_consecutive[i].first;
            SourceLocation sli = decli->getLocation();
            Location loci(m, sli);
            index = loci.line() == line ? index + 1 : 0;
            line = loci.line();
            if (loc[index].column() != loci.column() &&
                loc[index].column() +
                                decl[index]->getNameAsString().length() !=
                loci.column() + decli->getNameAsString().length()) {
                a.report(sli, check_name, "IND04",
                         "Declarators on consecutive lines must be aligned");
                a.report(sl[index], check_name, "IND04",
                         "Rightmost declarator is here",
                         false, DiagnosticIDs::Note);
            }
        }
    }
    d.d_consecutive.clear();
}

bool report::diff_func_parms(NamedDecl *d1, NamedDecl *d2)
{
    auto p1 = llvm::dyn_cast<ParmVarDecl>(d1);
    auto p2 = llvm::dyn_cast<ParmVarDecl>(d2);
    if (p1 && p2) {
        return p1->getFunctionScopeDepth() != p2->getFunctionScopeDepth() ||
               p1->getFunctionScopeIndex() <= p2->getFunctionScopeIndex();
    }
    return false;
}

void report::add_consecutive(NamedDecl *decl, SourceRange sr)
{
    Range r(m, sr);
    if (   !decl
        || !r
        || d.d_consecutive.size() == 0
        || d.d_consecutive.back().second.to().file() != r.to().file()
        || (d.d_consecutive.back().second.to().line() + 1 != r.from().line() &&
            d.d_consecutive.back().second.to().line()     != r.from().line())
        || diff_func_parms(decl, d.d_consecutive.back().first)) {
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
        if (args && (n = args->getNumMacroArguments()) > 0) {
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

void report::get_indent(Location l, int& offset, int& exact, bool& dotdot)
{
    offset = 0;
    exact = 0;
    dotdot = false;
    unsigned end = m.getFileOffset(l.location());
    for (const auto &i : d.d_indent[l.file()]) {
        if (end < i.first) {
            break;
        }
        offset += i.second.d_offset;
        dotdot ^= i.second.d_dotdot;
        exact = i.second.d_right_justified ? i.second.d_offset : offset;
    }
}

int report::process(Location l, bool greater, int lastdiff)
{
    int diff = 0;
    auto& done = d.d_done[l.file()][l.line()];
    if (done.first) {
        diff = done.second;
    }
    else {
        done.first = true;
        int offset;
        int exact_offset;
        bool dotdot;
        get_indent(l, offset, exact_offset, dotdot);
        llvm::StringRef line = a.get_source_line(l.location());
        if (!dotdot &&
            line.substr(0, l.column() - 1).find_first_not_of(' ') ==
                line.npos &&
            (!greater || line.size() < exact_offset + line.ltrim().size())) {
            int long_offset =
                std::min(79 - int(line.trim().size()), exact_offset);
            std::string expect = std::string(std::max(0, exact_offset), ' ') +
                                 line.trim().str();
            diff = expect.size() - line.size();
            if (line != expect && diff != lastdiff &&
                long_offset == exact_offset) {
                a.report(l.location(), check_name, "IND01",
                         "Possibly mis-indented line");
                auto report = a.report(l.location(), check_name, "IND01",
                         "Correct version may be\n%0",
                         false, DiagnosticIDs::Note);
                
                report << expect;
            }
        }
        done.second = diff;
    }
    return diff;
}

void report::operator()()
{
    if (a.is_test_driver()) {
        llvm::Regex outdent("^ *// *(v-*)[-^]\r*$");
        llvm::Regex reindent("^ *// *[-^](-*v)\r*$");

        for (auto& comment : a.attachment<CommentData>().d_allComments) {
            llvm::StringRef line = a.get_source_line(comment.getBegin());
            Location        loc(m, comment.getBegin());
            llvm::SmallVector<llvm::StringRef, 7> matches;
            if (line == "//..") {
                if (a.is_toplevel(loc.file())) {
                    indent ind((d.d_in_dotdot[loc.file()] ^= 1) ? +4 : -4);
                    ind.d_dotdot = true;
                    add_indent(comment.getEnd(), ind);
                }
            }
            else if (outdent.match(line, &matches)) {
                add_indent(comment.getEnd(), -matches[1].size());
            }
            else if (reindent.match(line, &matches)) {
                add_indent(comment.getEnd(), matches[1].size());
            }
        }
    }

    TraverseDecl(a.context()->getTranslationUnitDecl());

    // Process remnant declarators.
    add_consecutive(0, SourceRange());

    int lastdiff = 0;
    for (const auto p : d.d_to_process) {
        lastdiff = process(p.first, p.second, lastdiff);
    }
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    analyser.onTranslationUnitDone += report(analyser);
    observer.onMacroExpands += report(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static RegisterCheck c1(check_name, &subscribe);

// ----------------------------------------------------------------------------
// Copyright (C) 2014 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
