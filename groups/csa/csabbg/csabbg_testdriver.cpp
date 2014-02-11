// csabbg_testdriver.cpp                                              -*-C++-*-
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_util.h>
#include <csabase_visitor.h>
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
using clang::Token;
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

    typedef std::map<size_t /* line */, size_t /* index */> CommentsOfLines;
    CommentsOfLines d_comments_of_lines;

    typedef std::map<const FunctionDecl*, SourceRange> FunDecls;
    FunDecls d_fundecls;  // FunDecl, comment

    typedef std::multimap<long long, std::string> TestsOfCases;
    TestsOfCases d_tests_of_cases;  // Map functions to tets numbers.

    typedef std::multimap<std::string, long long> CasesOfTests;
    CasesOfTests d_cases_of_tests;  // Map test numbers to functions.

    typedef std::set<size_t> CCLines;  // Conditional compilation lines.
    CCLines d_cclines;

    const clang::Stmt *d_main;  // The compond statement of 'main()'.
};

struct report
    // Callback object for inspecting test drivers.
{
    Analyser&      d_analyser;
    SourceManager& d_manager;
    data&          d_data;

    report(Analyser& analyser);
        // Initialize an object of this type.

    SourceRange get_test_plan();
        // Return the TEST PLAN comment block.

    void operator()();
        // Callback for the end of the translation unit.

    void operator()(SourceRange comment);
        // Callback for the specified 'comment'.

    void mark_ccline(SourceLocation loc);
        // Mark the line of the specified 'loc' as a preprocessor conditional.

    void operator()(const FunctionDecl *function);
        // Callback for the specified 'function'.

    void operator()(SourceLocation loc, SourceRange);
        // Callback for '#if' and '#elif' at the specified 'loc'.

    void operator()(SourceLocation loc, const Token&);
        // Callback for '#ifdef' and '#ifndef' at the specified 'loc'.

    void operator()(SourceLocation loc, SourceLocation);
        // Callback for '#else' and '#endif' at the specified 'loc'.
};

report::report(Analyser& analyser)
: d_analyser(analyser)
, d_manager(analyser.manager())
, d_data(analyser.attachment<data>())
{
}

// Loosely match the banner of a TEST PLAN.
llvm::Regex test_plan_banner(
    "//[[:blank:]]*" "[-=_]([[:blank:]]?[-=_])*"  "[[:blank:]]*\n"
    "//[[:blank:]]*" "TEST" "[[:blank:]]*" "PLAN" "[[:blank:]]*\n"
    "//[[:blank:]]*" "[-=_]([[:blank:]]?[-=_])*"  "[[:blank:]]*\n",
    llvm::Regex::Newline | llvm::Regex::IgnoreCase);

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
    // Loosely match a long dashed separator.

llvm::Regex test_plan(
    "//"  "([^][[:alnum:]]*)"
    "\\[" "[[:blank:]]*" "(" "-?" "[[:digit:]]*" ")" "\\]"
          "[[:blank:]]*"
    "(.*)$",
    llvm::Regex::Newline);  // Match a test plan item.  [ ] are essential.

llvm::Regex testing(
    "//[[:blank:]]*Test(ing|ed|s)?[[:blank:]]*:?[[:blank:]]*\n",
    llvm::Regex::IgnoreCase);  // Loosely match 'Testing:' in a case comment.

llvm::Regex test_item(
    "^[^.;]*[[:alpha:]][^.;]*;?[^.;]*$",
    llvm::Regex::Newline);  // Loosely match a test item; at least one letter,
                            // no more than one ';', and no '.'.

void report::operator()()
{
    if (!d_analyser.is_test_driver()) {
        return;                                                       // RETURN
    }

    SourceRange plan_range = get_test_plan();

    if (!plan_range.isValid()) {
        d_analyser.report(
            d_manager.getLocForStartOfFile(d_manager.getMainFileID()),
            check_name, "TP14",
            "TEST PLAN section is absent");
    }

    llvm::StringRef plan = d_analyser.get_source(plan_range);

    llvm::SmallVector<llvm::StringRef, 7> matches;
    size_t offset = 0;

    // Hack off the banner.
    if (test_plan_banner.match(plan.drop_front(offset), &matches)) {
        offset += plan.drop_front(offset).find(matches[0]) + matches[0].size();
    }

    // Hack off past the separator if there is one.
    if (separator.match(plan.drop_front(offset), &matches)) {
        offset += plan.drop_front(offset).find(matches[0]) + matches[0].size();
    } else {
        d_analyser.report(plan_range.getBegin(),
                          check_name, "TP02",
                          "TEST PLAN section is missing '//---...---' "
                          "separator line");
    }

    // Hack off everything before the first item with brackets.
    if (test_plan.match(plan.drop_front(offset), &matches)) {
        offset += plan.drop_front(offset).find(matches[0]);
    }

    size_t plan_pos = offset;

    llvm::StringRef s;
    size_t count = 0;
    while (test_plan.match(s = plan.drop_front(offset), &matches)) {
        ++count;
        llvm::StringRef line = matches[0];
        llvm::StringRef cruft = matches[1];
        llvm::StringRef number = matches[2];
        llvm::StringRef item = matches[3];
        size_t matchpos = offset + s.find(line);
        offset = matchpos + line.size();
        long long test_num = 0;
        if (number.getAsInteger(10, test_num)) {
            test_num = std::numeric_limits<long long>::min();
        }
        size_t lb = matchpos + line.find('[');
        size_t rb = matchpos + line.find(']');
        SourceRange bracket_range(plan_range.getBegin().getLocWithOffset(lb),
                                  plan_range.getBegin().getLocWithOffset(rb));

        if (number.empty()) {
            d_analyser.report(bracket_range.getBegin(),
                              check_name, "TP03",
                              "Missing test number")
                << bracket_range;
        }

        if (test_num == 0) {
            d_analyser.report(bracket_range.getBegin(),
                              check_name, "TP04",
                              "Test number may not be 0")
                << bracket_range;
        }

        if (item.empty()) {
            d_analyser.report(bracket_range.getEnd().getLocWithOffset(1),
                              check_name, "TP07",
                              "Missing test item");
        } else {
            d_data.d_tests_of_cases.insert(std::make_pair(test_num, item));
            d_data.d_cases_of_tests.insert(std::make_pair(item, test_num));
        }

        if (cruft.find_first_not_of(" ") != cruft.npos) {
            d_analyser.report(bracket_range.getBegin().getLocWithOffset(-1),
                              check_name, "TP16",
                              "Extra characters before test number brackets");
        }
    }
    if (count == 0) {
        d_analyser.report(plan_range.getBegin().getLocWithOffset(plan_pos),
                          check_name, "TP13",
                          "No test items found in test plan");
    }

    // Find the main switch statement.
    const SwitchStmt *ss = 0;
    if (const Stmt *stmt = d_data.d_main) {
        Stmt::const_child_iterator b = stmt->child_begin();
        Stmt::const_child_iterator e = stmt->child_end();
        for (Stmt::const_child_iterator i = b; !ss && i != e; ++i) {
            ss = llvm::dyn_cast<SwitchStmt>(*i);
        }
        if (!ss) {
            d_analyser.report(stmt, check_name, "TP11",
                              "No switch statement found in test driver main");
            return;                                                   // RETURN
        }
    } else {
        return;                                                       // RETURN
    }

    const SwitchCase* sc;
    for (sc = ss->getSwitchCaseList(); sc; sc = sc->getNextSwitchCase()) {
        size_t line = Location(d_manager, sc->getColonLoc()).line() + 1;

        // Skip over preprocessor conditionals.
        while (d_data.d_cclines.find(line) != d_data.d_cclines.end()) {
            ++line;
        }

        SourceRange cr;
        if (d_data.d_comments_of_lines.find(line) !=
            d_data.d_comments_of_lines.end()) {
            cr = d_data.d_comments[d_data.d_comments_of_lines[line]];
        }

        const CaseStmt* cs = llvm::dyn_cast<CaseStmt>(sc);
        if (!cs) {
            // Default case.
            continue;
        }

        llvm::APSInt case_value;
        cs->getLHS()->EvaluateAsInt(case_value, *d_analyser.context());
        if (!cr.isValid()) {
            if (case_value != 0) {
                d_analyser.report(sc->getLocStart(),
                                  check_name, "TP05",
                                  "Test case has no comment");
            }
            continue;
        } else {
            if (case_value == 0) {
                d_analyser.report(sc->getLocStart(),
                        check_name, "TP10",
                        "Case 0 should not have a test comment");
            }
        }

        llvm::StringRef comment = d_analyser.get_source(cr);
        llvm::SmallVector<llvm::StringRef, 7> matches;
        size_t testing_pos = 0;
        size_t line_pos = 0;
        if (testing.match(comment, &matches)) {
            llvm::StringRef t = matches[0];
            testing_pos = comment.find(t);
            line_pos = testing_pos + t.size();
            std::pair<size_t, size_t> m =
                cool::csabase::mid_mismatch(t, "// Testing:\n");
            if (m.first != t.size()) {
                d_analyser.report(
                    cr.getBegin().getLocWithOffset(testing_pos + m.first),
                    check_name, "TP15",
                    "Correct format is '// Testing:'");
            }
        } else {
            d_analyser.report(cr.getBegin(),
                              check_name, "TP12",
                              "Comment should contain a 'Testing:' section");
        }

        for (size_t end_pos = 0;
             (line_pos = comment.find("//", line_pos)) != comment.npos;
             line_pos = end_pos) {
            end_pos = comment.find('\n', line_pos);
            llvm::StringRef line = comment.slice(line_pos + 2, end_pos).trim();
            if (testing_pos == 0 && line.empty()) {
                break;
            }
            typedef data::CasesOfTests::const_iterator Ci;
            std::pair<Ci, Ci> be = d_data.d_cases_of_tests.equal_range(line);
            Ci match_itr;
            for (match_itr = be.first; match_itr != be.second; ++match_itr) {
                if (match_itr->second == case_value) {
                    break;
                }
            }
            if (match_itr != be.second) {
                continue;
            }
            if (be.first != be.second) {
                size_t off = plan_pos;
                off += plan.drop_front(off).find(line);
                off = plan.rfind(']', off) - 1;
                d_analyser.report(cr.getBegin().getLocWithOffset(line_pos),
                        check_name, "TP08",
                        "Test plan does not have case number %0 for this item")
                    << case_value.getSExtValue();
                d_analyser.report(
                        plan_range.getBegin().getLocWithOffset(off),
                        check_name, "TP08",
                        "Test plan item is", false,
                        clang::DiagnosticsEngine::Note);
            }
            else if (test_item.match(line)) {
                d_analyser.report(cr.getBegin().getLocWithOffset(line_pos),
                                  check_name, "TP09",
                                  "Test plan should contain this item from "
                                  "'Testing' section of case %0")
                    << case_value.getSExtValue();
            }
        }

        typedef data::TestsOfCases::const_iterator Ci;
        std::pair<Ci, Ci> be = d_data.d_tests_of_cases.equal_range(
            case_value.getSExtValue());
        for (Ci i = be.first; i != be.second; ++i) {
            if (comment.drop_front(testing_pos).find(i->second) ==
                comment.npos) {
                d_analyser.report(
                    plan_range.getBegin().getLocWithOffset(
                        plan_pos + plan.drop_front(plan_pos).find(i->second)),
                    check_name, "TP06",
                    "'Testing' section of case %0 should contain this item "
                    "from test plan")
                    << case_value.getSExtValue();
            }
        }
    }
}

void report::operator()(SourceRange range)
{
    Location location(d_analyser.get_location(range.getBegin()));
    if (location.file() == d_analyser.toplevel()) {
        data::Comments& c = d_data.d_comments;
        if (c.size() == 0 ||
            !cool::csabase::areConsecutive(d_manager, c.back(), range)) {
            d_data.d_comments_of_lines[location.line()] = c.size();
            c.push_back(range);
        }
        else {
            c.back().setEnd(range.getEnd());
        }
    }
}

void report::operator()(const FunctionDecl *function)
{
    if (function->isMain() && function->hasBody()) {
        d_data.d_main = function->getBody();
    }
}

void report::mark_ccline(SourceLocation loc)
{
    Location location(d_analyser.get_location(loc));
    if (location.file() == d_analyser.toplevel()) {
        d_data.d_cclines.insert(location.line());
    }
}

void report::operator()(SourceLocation loc, SourceRange)
{
    mark_ccline(loc);
}

void report::operator()(SourceLocation loc, const Token&)
{
    mark_ccline(loc);
}

void report::operator()(SourceLocation loc, SourceLocation)
{
    mark_ccline(loc);
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    analyser.onTranslationUnitDone += report(analyser);
    visitor.onFunctionDecl += report(analyser);
    observer.onComment += report(analyser);
    observer.onIf += report(analyser);
    observer.onElif += report(analyser);
    observer.onIfdef += report(analyser);
    observer.onIfndef += report(analyser);
    observer.onElse += report(analyser);
    observer.onEndif += report(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static cool::csabase::RegisterCheck c1(check_name, &subscribe);
