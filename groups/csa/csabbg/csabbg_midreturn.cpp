// csabbg_midreturn.cpp                                               -*-C++-*-
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <set>
#include <string>
#include <sstream>

#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("mid-return");

// ----------------------------------------------------------------------------

using clang::CompoundStmt;
using clang::ConstStmtRange;
using clang::FunctionDecl;
using clang::ReturnStmt;
using clang::SourceLocation;
using clang::SourceManager;
using clang::SourceRange;
using clang::Stmt;
using cool::csabase::Analyser;
using cool::csabase::Location;
using cool::csabase::PPObserver;
using cool::csabase::Visitor;

namespace
{

// Data attached to analyzer for this check.
struct data
{
    std::set<const Stmt*>    d_last_returns; // Last top-level 'return'
    std::set<const Stmt*>    d_all_returns;  // All 'return' statements
    std::set<SourceLocation> d_rcs;          // Suppression comments
};

// Callback object for inspecting comments.
struct comments
{
    Analyser* d_analyser;

    comments(Analyser& analyser) : d_analyser(&analyser) {}

    void operator()(SourceRange range)
    {
        Location location(d_analyser->get_location(range.getBegin()));
        if (d_analyser->is_component(location.file())) {
            std::string comment(d_analyser->get_source(range));
            size_t rpos = comment.rfind("// RETURN");
            if (rpos != comment.npos) {
                d_analyser->attachment<data>().d_rcs.insert(
                    range.getBegin().getLocWithOffset(rpos));
            }
        }
    }
};

// Callback function for inspecting return statements.
void all_returns(Analyser& analyser, const ReturnStmt* ret)
{
    analyser.attachment<data>().d_all_returns.insert(ret);
}

// Function for searching for return statements.
const ReturnStmt* last_return(ConstStmtRange s)
{
    const ReturnStmt* ret = 0;
    for (; s; ++s) {
        if (llvm::dyn_cast<CompoundStmt>(*s)) {
            // Recurse into simple compound statements.
            ret = last_return((*s)->children());
        } else {
            // Try to cast each statement to a ReturnStmt. Therefore 'ret'
            // will only be non-zero if the final statement is a 'return'.
            ret = llvm::dyn_cast<ReturnStmt>(*s);
        }
    }
    return ret;
}

// Callback function for inspecting function declarations.
void last_returns(Analyser& analyser, const FunctionDecl* func)
{
    // Process only function definitions, not declarations.
    if (func->hasBody() &&
        func->getBody() &&
        !func->isTemplateInstantiation()) {
        const ReturnStmt* ret = last_return(func->getBody()->children());
        if (ret) {
            analyser.attachment<data>().d_last_returns.insert(ret);
        }
    }
}

// Callback object invoked upon completion.
struct report
{
    Analyser* d_analyser;

    report(Analyser& analyser) : d_analyser(&analyser) {}

    void operator()()
    {
        const data& d = d_analyser->attachment<data>();
        process_all_returns(d.d_all_returns.begin(), d.d_all_returns.end());
    }

    void process_all_returns(std::set<const Stmt*>::iterator begin,
                             std::set<const Stmt*>::iterator end)
    {
        const data& d = d_analyser->attachment<data>();
        for (std::set<const Stmt*>::iterator it = begin; it != end; ++it) {
            // Ignore final top-level return statements.
            if (!d.d_last_returns.count(*it)) {
                if (!is_commented(*it, d.d_rcs.begin(), d.d_rcs.end())) {
                    d_analyser->report(*it, check_name, "MR01",
                        "Mid-function 'return' requires '// RETURN' comment");
                }
            }
        }
    }

    // Determine if a statement has a proper '// RETURN' comment.
    bool is_commented(const Stmt* stmt,
                      std::set<SourceLocation>::iterator comments_begin,
                      std::set<SourceLocation>::iterator comments_end)
    {
        SourceManager& m = d_analyser->manager();
        unsigned       sline = m.getPresumedLineNumber(stmt->getLocEnd());
        unsigned       scolm = m.getPresumedColumnNumber(stmt->getLocEnd());
        clang::FileID  sfile = m.getFileID(stmt->getLocEnd());

        for (std::set<SourceLocation>::iterator it = comments_begin;
             it != comments_end;
             ++it) {
            unsigned      cline = m.getPresumedLineNumber(*it);
            unsigned      ccolm = m.getPresumedColumnNumber(*it);
            clang::FileID cfile = m.getFileID(*it);

            if (   (cline == sline || (scolm >= 69 && cline == sline + 1))
                && cfile == sfile) {
                if (ccolm != 71) {
                    std::ostringstream ss;
                    ss << "'// RETURN' comment must end in column 79, "
                       << "not " << (ccolm + 8);
                    if (scolm >= 69 && ccolm > 71) {
                        ss << " (place it alone on the next line)";
                    }
                    d_analyser->report(*it, check_name, "MR01", ss.str());
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
    observer.onComment += comments(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static cool::csabase::RegisterCheck c1(check_name, &last_returns);
static cool::csabase::RegisterCheck c2(check_name, &all_returns);
static cool::csabase::RegisterCheck c3(check_name, &subscribe);
