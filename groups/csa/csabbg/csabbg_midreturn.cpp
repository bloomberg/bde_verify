// csabbg_midreturn.cpp                                               -*-C++-*-

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/StmtIterator.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchersInternal.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>
#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_util.h>
#include <csaglb_comments.h>
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/VariadicFunction.h>
#include <llvm/Support/Casting.h>
#include <stddef.h>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <set>
#include <vector>
#include <sstream>
#include <string>

namespace csabase { class Visitor; }

using namespace clang;
using namespace clang::ast_matchers;
using namespace csabase;

// ----------------------------------------------------------------------------

static std::string const check_name("mid-return");

// ----------------------------------------------------------------------------

namespace
{

// Data attached to analyzer for this check.
struct data
{
    std::set<const ReturnStmt*> d_last_returns;  // Last top-level 'return'
    std::set<const ReturnStmt*> d_all_returns;   // All 'return'
    std::set<SourceLocation>    d_rcs;           // Suppression comments
    std::vector<SourceRange>    d_all_macros;    // Expanded macros
};

internal::DynTypedMatcher return_matcher()
    // Return an AST matcher which looks for return statements.
{
    return decl(forEachDescendant(returnStmt().bind("return")));
}

// Callback object invoked upon completion.
struct report : Report<data>
{
    INHERIT_REPORT_CTOR(report, Report, data);

    // Function for searching for final return statements.
    const ReturnStmt* last_return(Stmt::const_child_range r)
    {
        const ReturnStmt *ret = 0;
        for (const Stmt *s : r) {
            if (llvm::dyn_cast<CompoundStmt>(s)) {
                // Recurse into simple compound statements.
                ret = last_return(s->children());
            } else {
                // Try to cast each statement to a ReturnStmt. Therefore 'ret'
                // will only be non-zero if the final statement is a 'return'.
                ret = llvm::dyn_cast<ReturnStmt>(s);
            }
        }
        return ret;
    }

    void match_return(const BoundNodes& nodes)
    {
        const ReturnStmt *ret = nodes.getNodeAs<ReturnStmt>("return");

        if (!d_analyser.is_component(ret->getBeginLoc())) {
            return;                                                   // RETURN
        }

        // If the statement is contained in a function template specialization
        // (even nested within local classes) ignore it - the original in the
        // template will be processed.
        auto func = d_analyser.get_parent<FunctionDecl>(ret);
        auto lambda = d_analyser.get_parent<LambdaExpr>(ret);
        d_data.d_last_returns.insert(
            last_return((lambda ? lambda : func->getBody())->children()));
        do {
            if (func->isTemplateInstantiation()) {
                return;                                               // RETURN
            }
        } while (0 != (func = d_analyser.get_parent<FunctionDecl>(func)));
        d_data.d_all_returns.insert(ret);
    }

    void operator()(const Token&,
                    const MacroDefinition&,
                    SourceRange r,
                    const MacroArgs *)
        // macro expands callback
    {
        d_data.d_all_macros.push_back(r);
    }

    void operator()()
    {
        for (auto& range : a.attachment<CommentData>().d_allComments) {
            if (a.is_component(range.getBegin())) {
                llvm::StringRef comment = a.get_source(range);
                size_t          rpos    = comment.rfind("// RETURN");
                if (rpos != comment.npos) {
                    d.d_rcs.insert(range.getBegin().getLocWithOffset(rpos));
                }
            }
        }

        MatchFinder mf;
        OnMatch<report, &report::match_return> m1(this);
        mf.addDynamicMatcher(return_matcher(), &m1);
        mf.match(*d_analyser.context()->getTranslationUnitDecl(),
                 *d_analyser.context());

        process_all_returns(
            d_data.d_all_returns.begin(), d_data.d_all_returns.end());
    }

    bool isAllCasesReturn(const ReturnStmt *ret)
    {
        const SwitchStmt *ss = d_analyser.get_parent<SwitchStmt>(ret);
        if (ss) {
            const SwitchCase *me = d_analyser.get_parent<SwitchCase>(ret);
            if (!me || me->getSubStmt() != ret) {
                return false;
            }
            for (const SwitchCase* sc = ss->getSwitchCaseList();
                 sc;
                 sc = sc->getNextSwitchCase()) {
                if (llvm::dyn_cast<CaseStmt>(sc) &&
                    !llvm::dyn_cast<ReturnStmt>(sc->getSubStmt())) {
                    return false;                                     // RETURN
                }
            }
            return true;                                              // RETURN
        }
        return false;
    }

    void process_all_returns(std::set<const ReturnStmt*>::iterator begin,
                             std::set<const ReturnStmt*>::iterator end)
    {
        const data& d = d_analyser.attachment<data>();
        for (auto it = begin; it != end; ++it) {
            // Ignore final top-level return statements.
            if (!d.d_last_returns.count(*it) &&
                d_analyser.is_component(*it) &&
                !is_commented(*it, d.d_rcs.begin(), d.d_rcs.end()) &&
                !isAllCasesReturn(*it)) {
                d_analyser.report(*it, check_name, "MR01",
                        "Mid-function 'return' requires '// RETURN' comment",
                        true);
                SourceRange line_range =
                    d_analyser.get_line_range((*it)->getEndLoc());
                if (line_range.isValid()) {
                    llvm::StringRef line = d_analyser.get_source(line_range);
                    std::string tag = (line.size() < 70
                                       ? std::string(70 - line.size(), ' ')
                                       : "\n" + std::string(70, ' ')
                                      ) + "// RETURN";
                    d_analyser.report(*it, check_name, "MR01",
                                      "Correct text is\n%0",
                                      true, DiagnosticIDs::Note)
                        << line.str() + tag;
                    d_analyser.InsertTextBefore(line_range.getEnd(), tag);
                }
            }
        }
    }

    // Determine if a statement has a proper '// RETURN' comment.
    bool is_commented(const ReturnStmt* stmt,
                      std::set<SourceLocation>::iterator comments_begin,
                      std::set<SourceLocation>::iterator comments_end)
    {
        if (!d_analyser.is_component(stmt)) {
            return true;                                              // RETURN
        }

        SourceManager& m = d_analyser.manager();
        SourceLocation loc = stmt->getEndLoc();
        if (loc.isMacroID()) {
            for (auto r : d_data.d_all_macros) {
                if (!m.isBeforeInTranslationUnit(loc, r.getBegin()) &&
                    !m.isBeforeInTranslationUnit(r.getEnd(), loc)) {
                    loc = r.getEnd();
                    break;
                }
            }
        }
        // This "getLocForEndOfToken" weirdness is necessary because for some
        // expressions (like "a.def"), "stmt->getEndLoc()" returns a position
        // within the expression instead of the end (e.g., 'd', not 'f').
        loc = m.getFileLoc(Lexer::getLocForEndOfToken(
            loc, 0, m, d_analyser.context()->getLangOpts()));

        unsigned sline = m.getPresumedLineNumber(loc);
        unsigned scolm = m.getPresumedColumnNumber(loc);
        FileID   sfile = m.getFileID(loc);

        for (std::set<SourceLocation>::iterator it = comments_begin;
             it != comments_end;
             ++it) {
            unsigned      cline = m.getPresumedLineNumber(*it);
            unsigned      ccolm = m.getPresumedColumnNumber(*it);
            FileID cfile = m.getFileID(*it);

            if (   (cline == sline || (scolm >= 69 && cline == sline + 1))
                && cfile == sfile) {
                if (ccolm != 71) {
                    std::ostringstream ss;
                    ss << "'// RETURN' comment must end in column 79, "
                       << "not " << (ccolm + 8);
                    if (scolm >= 69 && ccolm > 71) {
                        ss << " (place it alone on the next line)";
                    }
                    d_analyser.report(*it, check_name, "MR02", ss.str());
                    SourceRange line_range = d_analyser.get_line_range(*it);
                    llvm::StringRef line = d_analyser.get_source(line_range)
                                               .slice(0, ccolm - 1)
                                               .rtrim();
                    std::string tag = (line.size() < 70
                                       ? std::string(70 - line.size(), ' ')
                                       : "\n" + std::string(70, ' ')
                                      ) + "// RETURN";
                    d_analyser.report(*it, check_name, "MR02",
                            "Correct text is\n%0",
                            true, DiagnosticIDs::Note)
                        << line.str() + tag;
                    line_range.setBegin(
                        line_range.getBegin().getLocWithOffset(line.size()));
                    d_analyser.ReplaceText(line_range, tag);
                }
                return true;
            }
        }
        return false;
    }
};

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
{
    analyser.onTranslationUnitDone += report(analyser);
    observer.onPPMacroExpands += report(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static RegisterCheck c3(check_name, &subscribe);

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
