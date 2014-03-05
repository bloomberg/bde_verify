// csabbg_testdriver.cpp                                              -*-C++-*-
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_util.h>
#include <csabase_visitor.h>
#include <llvm/Support/Regex.h>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <vector>
#include <ctype.h>

#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("test-driver");

// ----------------------------------------------------------------------------

using namespace clang;
using namespace clang::ast_matchers;
using namespace cool::csabase;

namespace
{

struct data
    // Data attached to analyser for this check.
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

    const Stmt *d_main;  // The compound statement of 'main()'.
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

    void check_boilerplate();
        // Check test driver boilerplate in the main file.

    void search(SourceLocation *best_loc,
                llvm::StringRef *best_needle,
                size_t *best_distance,
                llvm::StringRef key,
                const std::vector<llvm::StringRef>& needles,
                FileID fid);
        // Search the contents of the file specified by 'fid' for the closest
        // match to one of the specified 'needles' near the specified 'key' and
        // set the specified 'best_loc', 'best_needle', and 'best_distance' to
        // the matched position, string, and closeness respectively.

    void match_print_banner(const BoundNodes &nodes);
    bool found_banner;
    llvm::StringRef banner_text;
    const clang::StringLiteral *banner_literal;

    void match_noisy_print(const BoundNodes &nodes);
    void match_no_print(const BoundNodes &nodes);
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

llvm::Regex test_title(
    "[[:blank:]]*//[[:blank:]]*" "[-=_]([[:blank:]]?[-=_])*"  "[[:blank:]]*\n"
    "[[:blank:]]*//[[:blank:]]*" "(.*[^[:blank:]])" "[[:blank:]]*\n",
    llvm::Regex::Newline);  // Match the title of a test case.

llvm::Regex testing(
    "//[[:blank:]]*Test(ing|ed|s)?[[:blank:]]*:?[[:blank:]]*\n",
    llvm::Regex::IgnoreCase);  // Loosely match 'Testing:' in a case comment.

llvm::Regex test_item(
    "^[^.;]*[[:alpha:]][^.;]*;?[^.;]*$",
    llvm::Regex::Newline);  // Loosely match a test item; at least one letter,
                            // no more than one ';', and no '.'.

const internal::DynTypedMatcher &
print_banner_matcher()
    // Return an AST matcher which looks for the banner printer in a test case
    // statement.  It is satisfied with a 'printf' or 'cout' version, with or
    // without a leading newline/'endl'.  The 'printf' string literal combining
    // text and underlining is bound to "BANNER", the 'cout' banner text is
    // bound to "TEST" and the 'cout' underlining is bound to "====".  Valid
    // cases look like one of
    //: o 'cout' with initial 'endl'
    //..
    //    if (verbose) cout << endl
    //                      << "TESTING FOO" << endl
    //                      << "===========" << endl;
    //..
    //: o 'cout' without initial 'endl'
    //..
    //    if (verbose) cout << "TESTING FOO" << endl
    //                      << "===========" << endl;
    //..
    //: o 'printf' with initial '\n'
    //..
    //    if (verbose) printf("\nTESTING FOO\n===========\n");
    //..
    //: o 'printf' without initial '\n'
    //..
    //    if (verbose) printf("TESTING FOO\n===========\n");
    //..
{
    static const internal::DynTypedMatcher matcher =
        caseStmt(has(compoundStmt(hasDescendant(ifStmt(
            hasCondition(ignoringImpCasts(declRefExpr(to(
                varDecl(hasName("verbose")))))
            ),
            anyOf(
                hasDescendant(
                    callExpr(
                        argumentCountIs(1),
                        callee(functionDecl(hasName("printf"))),
                        hasArgument(0, ignoringImpCasts(
                            stringLiteral().bind("BANNER")))
                    )
                ),
                hasDescendant(
                    operatorCallExpr(
                        hasOverloadedOperatorName("<<"),
                        hasArgument(0, ignoringImpCasts(
                    operatorCallExpr(
                        hasOverloadedOperatorName("<<"),
                        hasArgument(0, ignoringImpCasts(
                    operatorCallExpr(
                        hasOverloadedOperatorName("<<"),
                        hasArgument(0, ignoringImpCasts(
                    operatorCallExpr(
                        hasOverloadedOperatorName("<<"),
                        hasArgument(0, ignoringImpCasts(
                    operatorCallExpr(
                        hasOverloadedOperatorName("<<"),
                        hasArgument(0, ignoringImpCasts(declRefExpr(to(
                            varDecl(hasName("cout")))))),
                        hasArgument(1, ignoringImpCasts(declRefExpr(to(
                            functionDecl(hasName("endl"))))))))),
                        hasArgument(1, ignoringImpCasts(
                            stringLiteral().bind("TEST")))))),
                        hasArgument(1, ignoringImpCasts(declRefExpr(to(
                            functionDecl(hasName("endl"))))))))),
                        hasArgument(1, ignoringImpCasts(
                            stringLiteral().bind("====")))))),
                        hasArgument(1, ignoringImpCasts(declRefExpr(to(
                            functionDecl(hasName("endl")))))))
                ),
                hasDescendant(
                    operatorCallExpr(
                        hasOverloadedOperatorName("<<"),
                        hasArgument(0, ignoringImpCasts(
                    operatorCallExpr(
                        hasOverloadedOperatorName("<<"),
                        hasArgument(0, ignoringImpCasts(
                    operatorCallExpr(
                        hasOverloadedOperatorName("<<"),
                        hasArgument(0, ignoringImpCasts(
                    operatorCallExpr(
                        hasOverloadedOperatorName("<<"),
                        hasArgument(0, ignoringImpCasts(declRefExpr(to(
                            varDecl(hasName("cout")))))),
                        hasArgument(1, ignoringImpCasts(
                            stringLiteral().bind("TEST")))))),
                        hasArgument(1, ignoringImpCasts(declRefExpr(to(
                            functionDecl(hasName("endl"))))))))),
                        hasArgument(1, ignoringImpCasts(
                            stringLiteral().bind("====")))))),
                        hasArgument(1, ignoringImpCasts(declRefExpr(to(
                            functionDecl(hasName("endl")))))))
                ),
                hasDescendant(
                    operatorCallExpr(
                        hasOverloadedOperatorName("<<"),
                        hasArgument(0, ignoringImpCasts(
                    operatorCallExpr(
                        hasOverloadedOperatorName("<<"),
                        hasArgument(0, ignoringImpCasts(
                    operatorCallExpr(
                        hasOverloadedOperatorName("<<"),
                        hasArgument(0, ignoringImpCasts(declRefExpr(to(
                            varDecl(hasName("cout")))))),
                        hasArgument(1, ignoringImpCasts(
                            stringLiteral().bind("TEST")))))),
                        hasArgument(1, ignoringImpCasts(
                            stringLiteral().bind("====")))))),
                        hasArgument(1, ignoringImpCasts(declRefExpr(to(
                            functionDecl(hasName("endl")))))))
                )
            )
        )))));
    return matcher;
}

const internal::DynTypedMatcher &
noisy_print_matcher()
    // Return an AST matcher which looks for (not very) verbose output inside
    // loops in a test case statement.
{
    static const internal::DynTypedMatcher matcher =
        caseStmt(has(compoundStmt(forEachDescendant(
            ifStmt(hasCondition(ignoringImpCasts(
                       declRefExpr(to(varDecl(hasName("verbose")))))),
                   anyOf(hasAncestor(doStmt()),
                         hasAncestor(forStmt()),
                         hasAncestor(whileStmt()))).bind("noisy")))));
    return matcher;
}

const internal::DynTypedMatcher &
no_print_matcher()
    // Return an AST matcher which looks for missing verbose output inside
    // loops in a test statement.
{
    static const internal::DynTypedMatcher matcher =
        caseStmt(has(compoundStmt(
            eachOf(
                forEachDescendant(doStmt().bind("try")),
                forEachDescendant(forStmt().bind("try")),
                forEachDescendant(whileStmt().bind("try"))),
            forEachDescendant(stmt(
                equalsBoundNode("try"),
                unless(hasDescendant(ifStmt(
                    hasCondition(ignoringImpCasts(declRefExpr(to(varDecl(anyOf(
                        hasName("verbose"),
                        hasName("veryVerbose"),
                        hasName("veryVeryVerbose"),
                        hasName("veryVeryVeryVerbose")
                    ))))))
                ))),
                unless(hasAncestor(ifStmt(
                    hasCondition(ignoringImpCasts(declRefExpr(to(varDecl(anyOf(
                        hasName("verbose"),
                        hasName("veryVerbose"),
                        hasName("veryVeryVerbose"),
                        hasName("veryVeryVeryVerbose")
                    ))))))
                )))
            ).bind("loop"))
        )));
    return matcher;
}

void report::operator()()
{
    if (!d_analyser.is_test_driver()) {
        return;                                                       // RETURN
    }

    check_boilerplate();

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

        MatchFinder mf;
        OnMatch<report, &report::match_print_banner> m1(this);
        mf.addDynamicMatcher(print_banner_matcher(), &m1);
        OnMatch<report, &report::match_noisy_print> m2(this);
        mf.addDynamicMatcher(noisy_print_matcher(), &m2);
        OnMatch<report, &report::match_no_print> m3(this);
        mf.addDynamicMatcher(no_print_matcher(), &m3);

        found_banner = false;
        banner_text = llvm::StringRef();
        banner_literal = 0;
        mf.match(*cs, *d_analyser.context());
        if (!found_banner && case_value != 0) {
            d_analyser.report(sc->getLocStart(),
                              check_name, "TP17",
                              "Test case does not contain "
                              "'if (verbose) print test banner'");
        }

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

        if (found_banner) {
            if (test_title.match(comment, &matches)) {
                llvm::StringRef t = matches[2];
                testing_pos = comment.find(t);
                line_pos = testing_pos + t.size();
                std::pair<size_t, size_t> m = mid_mismatch(t, banner_text);
                if (m.first != t.size()) {
                    d_analyser.report(
                        cr.getBegin().getLocWithOffset(testing_pos + m.first),
                        check_name, "TP22",
                        "Mismatch between title in comment and as printed");
                    d_analyser.report(banner_literal,
                                      check_name, "TP22",
                                      "Printed title is",
                                      false, DiagnosticsEngine::Note);
                }
            } else {
                d_analyser.report(
                        cr.getBegin().getLocWithOffset(comment.find('\n') + 1),
                        check_name, "TP22",
                        "Test case title should be\n%0")
                    << banner_text;
            }
        }

        if (testing.match(comment, &matches)) {
            llvm::StringRef t = matches[0];
            testing_pos = comment.find(t);
            line_pos = testing_pos + t.size();
            std::pair<size_t, size_t> m =
                mid_mismatch(t, "// Testing:\n");
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
                if (match_itr->second == case_value.getSExtValue()) {
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
                        DiagnosticsEngine::Note);
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
            !areConsecutive(d_manager, c.back(), range)) {
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

void report::match_print_banner(const BoundNodes& nodes)
{
    const StringLiteral *l1 = nodes.getNodeAs<StringLiteral>("BANNER");
    const StringLiteral *l2 = nodes.getNodeAs<StringLiteral>("TEST");
    const StringLiteral *l3 = nodes.getNodeAs<StringLiteral>("====");

    found_banner = true;

    if (l1) {
        llvm::StringRef s = l1->getString();
        size_t n = s.size();
        // e.g., n == 11
        // \n TEST \n ==== \n
        //  0 1234  5 6789 10
        // or n == 10
        // TEST \n ==== \n
        // 0123  4 5678  9
        if ((s.count('\n') != 3 ||
             s[0] != '\n' ||
             s[n - 1] != '\n' ||
             s[n / 2] != '\n' ||
             s.find_first_not_of('=', n / 2 + 1) != n - 1 ||
             s.find_first_of("abcdefghijklmnopqrstuvwxyz") != s.npos) &&
            (s.count('\n') != 2 ||
             s[n - 1] != '\n' ||
             s[n / 2 - 1] != '\n' ||
             s.find_first_not_of('=', n / 2) != n - 1 ||
             s.find_first_of("abcdefghijklmnopqrstuvwxyz") != s.npos)
           ) {
                d_analyser.report(l1, check_name, "TP18",
                                  "Incorrect test banner format");
                d_analyser.report(l1, check_name, "TP18",
                                  "Correct format is\n%0",
                                  false, DiagnosticsEngine::Note)
                    << "\"\\nALL CAPS DESCRIPTION\\n====================\\n\"";
        }
        banner_text = s.ltrim().split('\n').first;
        banner_literal = l1;
    } else if (l2 && l3) {
        llvm::StringRef text = l2->getString();
        llvm::StringRef ul = l3->getString();
        if (text.size() > 0 && text[0] == '\n' &&
            ul  .size() > 0 && ul  [0] == '\n') {
            text = text.substr(1);
            ul   = ul  .substr(1);
        }
        if (text.size() != ul.size() ||
            ul.find_first_not_of('=') != ul.npos ||
            text.find_first_of("abcdefghijklmnopqrstuvwxyz") != text.npos) {
                d_analyser.report(l2, check_name, "TP18",
                                  "Incorrect test banner format");
                d_analyser.report(l2, check_name, "TP18",
                                  "Correct format is\n%0",
                                  false, DiagnosticsEngine::Note)
                    << "cout << endl\n"
                       "     << \"ALL CAPS DESCRIPTION\" << endl\n"
                       "     << \"====================\" << endl;\n";
        }
        banner_text = text.ltrim().split('\n').first;
        banner_literal = l2;
    }
}

void report::match_noisy_print(const BoundNodes& nodes)
{
    const IfStmt *noisy = nodes.getNodeAs<IfStmt>("noisy");
    d_analyser.report(noisy->getCond(), check_name, "TP20",
                      "Within loops, act on very verbose");
}

void report::match_no_print(const BoundNodes& nodes)
{
    const Stmt *quiet = nodes.getNodeAs<Stmt>("loop");
    d_analyser.report(quiet, check_name, "TP21",
                      "Loops must contain very verbose action");
}

#undef  NL
#define NL "\n"

const char standard_bde_assert_test_macros[] =
""                                                                           NL
"// ==================="
"========================================================="                  NL
"//                      STANDARD BDE ASSERT TEST MACROS"                    NL
"// -------------------"
"---------------------------------------------------------"                  NL
""                                                                           NL
"static int testStatus = 0;"                                                 NL
""                                                                           NL
"static void aSsErT(int c, const char *s, int i)"                            NL
"{"                                                                          NL
"    if (c) {"                                                               NL
"        cout << \"Error \" << __FILE__ << \"(\" << i << \"): \" << s"       NL
"             << \"    (failed)\" << endl;"                                  NL
"        if (testStatus >= 0 && testStatus <= 100) ++testStatus;"            NL
"    }"                                                                      NL
"}"                                                                          NL
"#define ASSERT(X) { aSsErT(!(X), #X, __LINE__); }"                          NL
;

const char standard_bde_assert_test_macros_bsl[] =
"//===================="
"========================================================="                  NL
"//                      STANDARD BDE ASSERT TEST MACRO"                     NL
"//--------------------"
"---------------------------------------------------------"                  NL
"// NOTE: THIS IS A LOW-LEVEL COMPONENT AND MAY NOT USE ANY C++ LIBRARY"     NL
"// FUNCTIONS, INCLUDING IOSTREAMS."                                         NL
"static int testStatus = 0;"                                                 NL
""                                                                           NL
"static void aSsErT(bool b, const char *s, int i)"                           NL
"{"                                                                          NL
"    if (b) {"                                                               NL
"        printf(\"Error \" __FILE__ \"(%d): %s    (failed)\\n\", i, s);"     NL
"        if (testStatus >= 0 && testStatus <= 100) ++testStatus;"            NL
"    }"                                                                      NL
"}"                                                                          NL
""                                                                           NL
;

const char standard_bde_loop_assert_test_macros_old[] =
"//=================="
"==========================================================="                NL
"//                  STANDARD BDE LOOP-ASSERT TEST MACROS"                   NL
"//------------------"
"-----------------------------------------------------------"                NL
"#define LOOP_ASSERT(I,X) { \\"                                              NL
"   if (!(X)) { cout << #I << \": \" << I << \"\\n\"; "
"aSsErT(1, #X, __LINE__); }}"                                                NL
""                                                                           NL
"#define LOOP2_ASSERT(I,J,X) { \\"                                           NL
"   if (!(X)) { cout << #I << \": \" << I << \"\\t\" "
"<< #J << \": \" \\"                                                         NL
"              << J << \"\\n\"; "
"aSsErT(1, #X, __LINE__); } }"                                               NL
""                                                                           NL
"#define LOOP3_ASSERT(I,J,K,X) { \\"                                         NL
"   if (!(X)) { cout << #I << \": \" << I << \"\\t\" "
"<< #J << \": \" << J << \"\\t\" \\"                                         NL
"              << #K << \": \" << K << \"\\n\"; "
"aSsErT(1, #X, __LINE__); } }"                                               NL
""                                                                           NL
"#define LOOP4_ASSERT(I,J,K,L,X) { \\"                                       NL
"   if (!(X)) { cout << #I << \": \" << I << \"\\t\" "
"<< #J << \": \" << J << \"\\t\" << \\"                                      NL
"       #K << \": \" << K << \"\\t\" << #L << \": \" << L << \"\\n\"; \\"    NL
"       aSsErT(1, #X, __LINE__); } }"                                        NL
""                                                                           NL
"#define LOOP5_ASSERT(I,J,K,L,M,X) { \\"                                     NL
"   if (!(X)) { cout << #I << \": \" << I << \"\\t\" "
"<< #J << \": \" << J << \"\\t\" << \\"                                      NL
"       #K << \": \" << K << \"\\t\" << #L << \": \" << L << \"\\t\" << \\"  NL
"       #M << \": \" << M << \"\\n\"; \\"                                    NL
"       aSsErT(1, #X, __LINE__); } }"                                        NL
""                                                                           NL
"#define LOOP6_ASSERT(I,J,K,L,M,N,X) { \\"                                   NL
"   if (!(X)) { cout << #I << \": \" << I << \"\\t\" "
"<< #J << \": \" << J << \"\\t\" << \\"                                      NL
"       #K << \": \" << K << \"\\t\" << #L << \": \" << L << \"\\t\" << \\"  NL
"       #M << \": \" << M << \"\\t\" << #N << \": \" << N << \"\\n\"; \\"    NL
"       aSsErT(1, #X, __LINE__); } }"                                        NL
""                                                                           NL
;

const char standard_bde_loop_assert_test_macros_new[] =
""                                                                           NL
"// ================="
"==========================================================="                NL
"//                    STANDARD BDE LOOP-ASSERT TEST MACROS"                 NL
"// -----------------"
"-----------------------------------------------------------"                NL
""                                                                           NL
"#define C_(X)   << #X << \": \" << X << '\\t'"                              NL
"#define A_(X,S) { if (!(X)) { cout S << endl; aSsErT(1, #X, __LINE__); } }" NL
"#define LOOP_ASSERT(I,X)            A_(X,C_(I))"                            NL
"#define LOOP2_ASSERT(I,J,X)         A_(X,C_(I)C_(J))"                       NL
"#define LOOP3_ASSERT(I,J,K,X)       A_(X,C_(I)C_(J)C_(K))"                  NL
"#define LOOP4_ASSERT(I,J,K,L,X)     A_(X,C_(I)C_(J)C_(K)C_(L))"             NL
"#define LOOP5_ASSERT(I,J,K,L,M,X)   A_(X,C_(I)C_(J)C_(K)C_(L)C_(M))"        NL
"#define LOOP6_ASSERT(I,J,K,L,M,N,X) A_(X,C_(I)C_(J)C_(K)C_(L)C_(M)C_(N))"   NL
;

const char standard_bde_loop_assert_test_macros_bsl[] =
"//=================="
"==========================================================="                NL
"//                      STANDARD BDE TEST DRIVER MACROS"                    NL
"//------------------"
"-----------------------------------------------------------"                NL
""                                                                           NL
"#define ASSERT       BSLS_BSLTESTUTIL_ASSERT"                               NL
"#define LOOP_ASSERT  BSLS_BSLTESTUTIL_LOOP_ASSERT"                          NL
"#define LOOP0_ASSERT BSLS_BSLTESTUTIL_LOOP0_ASSERT"                         NL
"#define LOOP1_ASSERT BSLS_BSLTESTUTIL_LOOP1_ASSERT"                         NL
"#define LOOP2_ASSERT BSLS_BSLTESTUTIL_LOOP2_ASSERT"                         NL
"#define LOOP3_ASSERT BSLS_BSLTESTUTIL_LOOP3_ASSERT"                         NL
"#define LOOP4_ASSERT BSLS_BSLTESTUTIL_LOOP4_ASSERT"                         NL
"#define LOOP5_ASSERT BSLS_BSLTESTUTIL_LOOP5_ASSERT"                         NL
"#define LOOP6_ASSERT BSLS_BSLTESTUTIL_LOOP6_ASSERT"                         NL
"#define ASSERTV      BSLS_BSLTESTUTIL_ASSERTV"                              NL
""                                                                           NL
;

const char semi_standard_test_output_macros[] =
""                                                                           NL
"// ================="
"==========================================================="                NL
"//                  SEMI-STANDARD TEST OUTPUT MACROS"                       NL
"// -----------------"
"-----------------------------------------------------------"                NL
""                                                                           NL
"#define P(X) cout << #X \" = \" << (X) << endl; "
"// Print identifier and value."                                             NL
"#define Q(X) cout << \"<| \" #X \" |>\" << endl;  "
"// Quote identifier literally."                                             NL
"#define P_(X) cout << #X \" = \" << (X) << \", \" << flush; "
"// 'P(X)' without '\\n'"                                                    NL
"#define T_ cout << \"\\t\" << flush;             // Print tab w/o newline." NL
"#define L_ __LINE__                           // current Line number"       NL
;

const char semi_standard_test_output_macros_bsl[] =
""                                                                           NL
"#define Q   BSLS_BSLTESTUTIL_Q   // Quote identifier literally."            NL
"#define P   BSLS_BSLTESTUTIL_P   // Print identifier and value."            NL
"#define P_  BSLS_BSLTESTUTIL_P_  // P(X) without '\\n'."                    NL
"#define T_  BSLS_BSLTESTUTIL_T_  // Print a tab (w/o newline)."             NL
"#define L_  BSLS_BSLTESTUTIL_L_  // current Line number"                    NL
""                                                                           NL
;

void report::search(SourceLocation *best_loc,
                    llvm::StringRef *best_needle,
                    size_t *best_distance,
                    llvm::StringRef key,
                    const std::vector<llvm::StringRef> &needles,
                    FileID fid)
{
    const SourceManager &m = d_analyser.manager();
    SourceLocation top = m.getLocForStartOfFile(fid);
    llvm::StringRef haystack = m.getBufferData(fid);
    size_t num_lines = haystack.count('\n');

    // For each needle, get its number of lines and blank lines, and determine
    // the maximum number of lines over all needles.
    size_t ns = needles.size();
    std::vector<size_t> needle_lines(ns);
    std::vector<size_t> needle_blank_lines(ns);
    size_t max_needle_lines = 0;
    for (size_t n = 0; n < ns; ++n) {
        llvm::StringRef needle = needles[n];
        needle_lines[n] = needle.count('\n');
        needle_blank_lines[n] = needle.count("\n\n");
        if (max_needle_lines < needle_lines[n]) {
            max_needle_lines = needle_lines[n];
        }
    }

    // Compute the set of lines to examine.  Always examine last line, so that
    // some match is returned.
    std::set<size_t> lines;
    lines.insert(Location(m, m.getLocForEndOfFile(fid)).line());

    // For each line in the haystack where we find the key, add the range of
    // lines around it, 'max_needle_lines' in each direction, to the set of
    // lines to be examined.
    for (size_t key_pos = haystack.find(key); key_pos != haystack.npos;) {
        size_t key_line = Location(m, top.getLocWithOffset(key_pos)).line();
        for (size_t i = 0; i <= max_needle_lines; ++i) {
            if (key_line > i && key_line - i <= num_lines) {
                lines.insert(key_line - i);
            }
            if (key_line + i <= num_lines) {
                lines.insert(key_line + i);
            }
        }
        size_t pos = haystack.drop_front(key_pos + key.size()).find(key);
        if (pos == haystack.npos) {
            break;
        }
        key_pos += key.size() + pos;
    }

    *best_distance = ~size_t(0);

    // For each line to be examined...
    std::set<size_t>::const_iterator bl = lines.begin();
    std::set<size_t>::const_iterator el = lines.end();
    for (std::set<size_t>::const_iterator il = bl; il != el; ++il) {
        size_t line = *il;
        SourceLocation begin = m.translateLineCol(fid, line, 1);
        // For each needle...
        for (size_t n = 0; n < ns; ++n) {
            llvm::StringRef needle = needles[n];
            size_t nl = needle_lines[n];
            size_t nbl = needle_blank_lines[n];
            // Examine successively smaller ranges of lines from the starting
            // line, beginning with the number of lines in the needle down to
            // that number less the number of blank lines in the needle.
            for (size_t nn = nl; nn >= nl - nbl; --nn) {
                SourceLocation end = m.translateLineCol(fid, line + nn, 1);
                SourceRange r(begin, end);
                llvm::StringRef s = d_analyser.get_source(r, true);
                size_t distance = s.edit_distance(needle);
                // Record a better match whenever one is found.
                if (distance < *best_distance) {
                    *best_distance = distance;
                    std::pair<size_t, size_t> mm = mid_mismatch(s, needle);
                    *best_loc = r.getBegin().getLocWithOffset(mm.first);
                    *best_needle = needle;
                    // Return on an exact match.
                    if (distance == 0 || mm.first == s.size()) {
                        return;                                       // RETURN
                    }
                }
            }
        }
    }
}

void report::check_boilerplate()
{
    const SourceManager &m = d_analyser.manager();
    FileID fid = m.getMainFileID();

    size_t distance;
    llvm::StringRef needle;
    std::vector<llvm::StringRef> needles;
    SourceLocation loc;

    needles.clear();
    needles.push_back(standard_bde_assert_test_macros);
    needles.push_back(standard_bde_assert_test_macros_bsl);
    search(&loc, &needle, &distance, "(failed)", needles, fid);
    if (distance != 0) {
        d_analyser.report(loc,
                          check_name, "TP19",
                          "Missing or malformed standard test driver section");
        d_analyser.report(loc,
                          check_name, "TP19",
                          "Correct form is\n%0",
                          false, DiagnosticsEngine::Note)
            << needle;
    }

    needles.clear();
    needles.push_back(standard_bde_loop_assert_test_macros_old);
    needles.push_back(standard_bde_loop_assert_test_macros_new);
    needles.push_back(standard_bde_loop_assert_test_macros_bsl);
    search(&loc, &needle, &distance, "define LOOP_ASSERT", needles, fid);
    if (distance != 0) {
        d_analyser.report(loc, check_name, "TP19",
                          "Missing or malformed standard test driver section");
        d_analyser.report(loc, check_name, "TP19", "Correct form is\n%0",
                          false, DiagnosticsEngine::Note)
            << needle;
    }

    needles.clear();
    needles.push_back(semi_standard_test_output_macros);
    needles.push_back(semi_standard_test_output_macros_bsl);
    search(&loc, &needle, &distance, "define P_", needles, fid);
    if (distance != 0) {
        d_analyser.report(loc, check_name, "TP19",
                          "Missing or malformed standard test driver section");
        d_analyser.report(loc, check_name, "TP19", "Correct form is\n%0",
                          false, DiagnosticsEngine::Note)
            << needle;
    }
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

static RegisterCheck c1(check_name, &subscribe);
