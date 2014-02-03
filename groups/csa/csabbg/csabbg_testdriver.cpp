// csabbg_testdriver.cpp                                              -*-C++-*-
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_util.h>
#include <csabase_visitor.h>
#include <llvm/ADT/APSInt.h>
#include <llvm/Support/Regex.h>
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <ctype.h>

#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("test-driver");

// ----------------------------------------------------------------------------

using clang::CaseStmt;
using clang::FunctionDecl;
using clang::SourceLocation;
using clang::SourceManager;
using clang::SourceRange;
using clang::Stmt;
using clang::SwitchCase;
using clang::SwitchStmt;
using cool::csabase::Analyser;
using cool::csabase::Location;
using cool::csabase::PPObserver;
using cool::csabase::Range;
using cool::csabase::Visitor;

namespace
{

struct data
    // Data attached to analyzer for this check.
{
    typedef std::vector<SourceRange> Comments;
    Comments d_comments;  // Comment blocks per file.

    typedef std::map<const FunctionDecl*, SourceRange> FunDecls;
    FunDecls d_fundecls;  // FunDecl, comment

    typedef std::multimap<llvm::APSInt, std::string> TestsOfCases;
    TestsOfCases d_tests_of_cases;

    typedef std::multimap<std::string, llvm::APSInt> CasesOfTests;
    CasesOfTests d_cases_of_tests;
};

struct report
    // Callback object for inspecting test drivers.
{
    Analyser&      d_analyser;
    SourceManager& d_manager;
    data&          d_data;

    report(Analyser& analyser);

    SourceRange get_test_plan();
        // Return the TEST PLAN comment block.

    void operator()();
        // Callback method for the end of the translation unit.

    void operator()(SourceRange comment);
        // Callback method for the specified 'comment'.

    void operator()(const clang::FunctionDecl *function);
        // Callback method for the specified 'function'.
};

report::report(Analyser& analyser)
: d_analyser(analyser)
, d_manager(analyser.manager())
, d_data(analyser.attachment<data>())
{
}

llvm::Regex test_plan_banner(
    "//[[:blank:]]*" "[-=_]([[:blank:]]?[-=_])*"  "[[:blank:]]*\n"
    "//[[:blank:]]*" "TEST" "[[:blank:]]*" "PLAN" "[[:blank:]]*\n"
    "//[[:blank:]]*" "[-=_]([[:blank:]]?[-=_])*"  "[[:blank:]]*\n",
    llvm::Regex::Newline);

SourceRange report::get_test_plan()
{
    data::Comments::iterator b = d_data.d_comments.begin();
    data::Comments::iterator e = d_data.d_comments.end();
    for (data::Comments::iterator i = b; i != e; ++i) {
        llvm::StringRef comment = d_analyser.get_source(*i);
        if (test_plan_banner.match(comment)) {
            return *i;                                                // RETURN
        }
    }
    return SourceRange();
}

llvm::Regex separator("//[[:blank:]]*-{60,}$\n", llvm::Regex::Newline);

llvm::Regex test_plan(
    "//"  "[[:blank:]]*"
    "\\[" "[[:blank:]]*" "(" "-?" "[[:digit:]]*" ")" "\\]"
          "[[:blank:]]*"
    "(.*)\n",
    llvm::Regex::Newline);

void report::operator()()
{
    SourceRange plan_range = get_test_plan();

    if (!plan_range.isValid()) {
        d_analyser.report(d_analyser.context()->getTranslationUnitDecl(),
                          check_name, "TP01", "TEST PLAN section is absent");
        return;                                                       // RETURN
    }

    llvm::StringRef plan = d_analyser.get_source(plan_range);

    size_t offset = 0;
    llvm::SmallVector<llvm::StringRef, 7> matches;
    if (separator.match(plan, &matches)) {
        offset = plan.find(matches[0]) + matches[0].size();
    } else {
        d_analyser.report(plan_range.getBegin(), check_name, "TP02",
                          "TEST PLAN section is missing separator line");
    }

    llvm::StringRef s;
    while (test_plan.match(s = plan.drop_front(offset), &matches)) {
        llvm::StringRef line = matches[0];
        llvm::StringRef number = matches[1];
        llvm::StringRef item = matches[2];
        size_t matchpos = plan.find(line);
        offset = matchpos + line.size();
        llvm::APSInt test_num(64, false);
        if (number.getAsInteger(10, test_num)) {
            test_num.setAllBits();
        }
        size_t lb = matchpos + line.find('[');
        size_t rb = matchpos + line.find(']');
        SourceRange bracket_range(plan_range.getBegin().getLocWithOffset(lb),
                                  plan_range.getBegin().getLocWithOffset(rb));

        if (number.empty()) {
            d_analyser.report(bracket_range.getBegin(), check_name, "TP03",
                              "Missing test number")
                << bracket_range;
        }

        if (test_num == 0) {
            d_analyser.report(bracket_range.getBegin(), check_name, "TP04",
                              "Test number may not be 0")
                << bracket_range;
        }

        d_data.d_tests_of_cases.insert(std::make_pair(test_num, item));
        d_data.d_cases_of_tests.insert(std::make_pair(item, test_num));
    }
}

void report::operator()(SourceRange range)
{
    Location location(d_analyser.get_location(range.getBegin()));
    if (location.file() == d_analyser.toplevel()) {
        data::Comments& c = d_data.d_comments;
        if (c.size() == 0 ||
            !cool::csabase::areConsecutive(d_manager, c.back(), range)) {
            c.push_back(range);
        } else {
            c.back().setEnd(range.getEnd());
        }
    }
}

llvm::Regex testing(
    "//[[:blank:]]*Testing[[:blank:]]*:?[[:blank:]]*$",
    llvm::Regex::IgnoreCase);

void report::operator()(const clang::FunctionDecl *function)
{
    const SwitchStmt *ss = 0;
    if (function->isMain() && function->hasBody()) {
        if (const Stmt *stmt = function->getBody()) {
            Stmt::const_child_iterator b = stmt->child_begin();
            Stmt::const_child_iterator e = stmt->child_end();
            for (Stmt::const_child_iterator i = b; !ss && i != e; ++i) {
                ss = llvm::dyn_cast<SwitchStmt>(*i);
            }
        }
    }
    if (ss) {
        const SwitchCase* sc;
        for (sc = ss->getSwitchCaseList(); sc; sc = sc->getNextSwitchCase()) {
            size_t line = Location(d_manager, sc->getColonLoc()).line() + 1;
            data::Comments& c = d_data.d_comments;
            SourceRange cr;
            for (size_t i = 0; i < c.size(); ++i) {
                if (line == Location(d_manager, c[i].getBegin()).line()) {
                    cr = c[i];
                    break;
                }
            }
            const CaseStmt *cs = llvm::dyn_cast<CaseStmt>(sc);
            if (cs) {
                llvm::APSInt case_value;
                cs->getLHS()->EvaluateAsInt(case_value, *d_analyser.context());
                if (!cr.isValid()) {
                    if (case_value != 0) {
                        d_analyser.report(sc->getLocStart(), check_name, "TP05",
                                          "Test case has no comment header");
                    }
                } else {
                    llvm::StringRef comment = d_analyser.get_source(cr);
                    llvm::SmallVector<llvm::StringRef, 7> matches;
                    if (testing.match(comment, &matches)) {
                        llvm::StringRef t = matches[0];
                        comment =
                            comment.drop_front(comment.find(t) + t.size());
                    }
                    typedef data::TestsOfCases::const_iterator Ci;
                    std::pair<Ci, Ci> be =
                        d_data.d_tests_of_cases.equal_range(case_value);
                    for (Ci i = be.first; i != be.second; ++i) {
                        if (comment.find(i->second) == comment.npos) {
                            d_analyser.report(sc->getLocStart(),
                                              check_name, "TP06",
                                              "Comment header missing "
                                              "function from test plan\n'%0'")
                                << i->second;
                        }
                    }
                }
            }
        }
    }
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    analyser.onTranslationUnitDone += report(analyser);
    visitor.onFunctionDecl += report(analyser);
    observer.onComment += report(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static cool::csabase::RegisterCheck c1(check_name, &subscribe);
