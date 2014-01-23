// csafmt_comments.cpp                                                -*-C++-*-
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_util.h>
#include <llvm/Support/Regex.h>
#include <string>

#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("comments");

// ----------------------------------------------------------------------------

using clang::SourceLocation;
using clang::SourceManager;
using clang::SourceRange;
using cool::csabase::Analyser;
using cool::csabase::Location;
using cool::csabase::PPObserver;
using cool::csabase::Range;
using cool::csabase::Visitor;

namespace
{

struct comments
    // Data holding seen comments.
{
    typedef std::vector<SourceRange> Ranges;
    typedef std::map<std::string, Ranges> Comments;
    Comments d_comments;

    typedef std::set<Range> Reported;
    Reported d_reported;

    void append(Analyser& analyser, SourceRange range);
};

void comments::append(Analyser& analyser, SourceRange range)
{
    SourceManager& m = analyser.manager();
    comments::Ranges& c = d_comments[m.getFilename(range.getBegin())];
    if (c.size() != 0 && cool::csabase::areConsecutive(m, c.back(), range)) {
        c.back().setEnd(range.getEnd());
    } else {
        c.push_back(range);
    }
}

struct files
    // Callback object for inspecting files.
{
    Analyser& d_analyser;                   // Analyser object.

    files(Analyser& analyser);
        // Create a 'files' object, accessing the specified 'analyser'.

    void operator()(SourceRange range);
        // The specified comment 'range' is added to the stored data.

    void operator()();
        // Inspect all comments.

    void check_fvs(SourceRange range);
        // Warn about comment containing "fully value semantic".

    void check_bubble(SourceRange range);
        // Warn about comment containing badly formed inheritance diagram.

    void report_bubble(const Range &r, llvm::StringRef text);
        // Warn about a single bad inheritance bubble at the specified 'r'
        // containing the specified 'text'.
};

files::files(Analyser& analyser)
: d_analyser(analyser)
{
}

void files::operator()(SourceRange range)
{
    d_analyser.attachment<comments>().append(d_analyser, range);
}

void files::operator()()
{
    comments::Comments& file_comments =
        d_analyser.attachment<comments>().d_comments;

    for (comments::Comments::iterator file_begin = file_comments.begin(),
                                      file_end   = file_comments.end(),
                                      file_itr   = file_begin;
         file_itr != file_end;
         ++file_itr) {
        const std::string &file_name = file_itr->first;
        if (!d_analyser.is_component(file_name)) {
            continue;
        }
        comments::Ranges& comments = file_itr->second;
        for (comments::Ranges::iterator comments_begin = comments.begin(),
                                        comments_end   = comments.end(),
                                        comments_itr   = comments_begin;
             comments_itr != comments_end;
             ++comments_itr) {
            check_fvs(*comments_itr);
            check_bubble(*comments_itr);
        }
    }
}

llvm::Regex fvs(
    "fully" "[^_[:alnum:]]*" "value" "[^_[:alnum:]]*" "semantic",
    llvm::Regex::IgnoreCase);

void files::check_fvs(SourceRange range)
{
    llvm::SmallVector<llvm::StringRef, 7> matches;
    llvm::StringRef comment = d_analyser.get_source(range, true);

    unsigned offset = 0;
    llvm::StringRef s;
    while (fvs.match(s = comment.drop_front(offset), &matches)) {
        llvm::StringRef text = matches[0];
        std::pair<unsigned, unsigned> m = cool::csabase::mid_match(s, text);
        unsigned matchpos = offset + m.first;
        offset = matchpos + text.size();
        d_analyser.report(range.getBegin().getLocWithOffset(matchpos),
                          check_name, "FVS01",
                          "The term \"%0\" is deprecated; use a description "
                          "appropriate to the component type")
            << text
            << SourceRange(range.getBegin().getLocWithOffset(matchpos),
                           range.getBegin().getLocWithOffset(offset - 1));
    }
}

llvm::Regex bad_bubble(
                             "([(]" "[[:space:]]*"              // 1
                                    "([[:alnum:]_:]+)"          // 2
                                    "[[:space:]]*"
                             "[)])" ".*\n"
        "//" "([[:space:]]+)" "[|]" "[[:space:]]*.*\n"          // 3
    "(" "//" "\\3"            "[|]" "[[:space:]]*.*\n" ")*"     // 4
        "//" "\\3"            "[V]" "[[:space:]]*.*\n"     
        "//" "[[:space:]]*"  "([(]" "[[:space:]]*"              // 5
                                    "([[:alnum:]_:]+)"          // 6
                                    "[[:space:]]*"
                              "[)])",
    llvm::Regex::Newline);

std::string bubble(llvm::StringRef s)
{
    return "\n ," + std::string(s.size() + 1, '-') + ".\n" +
           "(  "  + s.str()                        + " )\n" +
           " `"   + std::string(s.size() + 1, '-') + "'\n";
}

void files::report_bubble(const Range &r, llvm::StringRef text)
{
    comments::Reported& reported_ranges =
        d_analyser.attachment<comments>().d_reported;
    if (!reported_ranges.count(r)) {
        reported_ranges.insert(r);
        d_analyser.report(r.from().location(), check_name, "BADB01",
                          "Incorrectly formed inheritance bubble")
            << SourceRange(r.from().location(), r.to().location());
        d_analyser.report(r.from().location(), check_name, "BADB01",
                          "Correct format is%0",
                          false, clang::DiagnosticsEngine::Note)
            << bubble(text);
    }
}

void files::check_bubble(SourceRange range)
{
    llvm::SmallVector<llvm::StringRef, 7> matches;
    llvm::StringRef comment = d_analyser.get_source(range, true);

    unsigned offset = 0;
    llvm::StringRef s;
    while (bad_bubble.match(s = comment.drop_front(offset), &matches)) {
        llvm::StringRef text = matches[0];
        std::pair<unsigned, unsigned> m = cool::csabase::mid_match(s, text);
        unsigned matchpos = offset + m.first;
        offset = matchpos + text.size();

        Range b1(d_analyser.manager(),
                 SourceRange(range.getBegin().getLocWithOffset(matchpos),
                             range.getBegin().getLocWithOffset(
                                 matchpos + matches[1].size() - 1)));
        report_bubble(b1, matches[2]);

        Range b2(
            d_analyser.manager(),
            SourceRange(range.getBegin().getLocWithOffset(
                            matchpos + matches[0].size() - matches[5].size()),
                        range.getBegin().getLocWithOffset(
                            matchpos + matches[0].size() - 1)));
        report_bubble(b2, matches[6]);
    }
}

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
    // Hook up the callback functions.
{
    analyser.onTranslationUnitDone += files(analyser);
    observer.onComment             += files(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static cool::csabase::RegisterCheck c1(check_name, &subscribe);
