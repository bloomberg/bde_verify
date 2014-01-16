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

    void check(SourceRange range);
        // Inspect a single comment.
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
            check(*comments_itr);
        }
    }
}

static llvm::Regex description_tag(
    "@" "[[:space:]]*" "DESCRIPTION" "[[:space:]]*" ":",
    llvm::Regex::IgnoreCase
);

static llvm::Regex fully_value_semantic(
    "fully" "[^_[:alnum:]]*" "value" "[^_[:alnum:]]*" "semantic",
    llvm::Regex::IgnoreCase);

void files::check(SourceRange range)
{
    llvm::SmallVector<llvm::StringRef, 7> matches;
    llvm::StringRef comment = d_analyser.get_source(range, true);

    if (   description_tag.match(comment)
        && fully_value_semantic.match(comment, &matches)) {
        std::pair<unsigned, unsigned> m =
            cool::csabase::mid_match(comment, matches[0]);
        d_analyser.report(range.getBegin().getLocWithOffset(m.first),
                          check_name, "FVS01",
                          "The term \"%0\" is deprecated; use a description "
                          "appropriate to the component type")
            << matches[0]
            << SourceRange(range.getBegin().getLocWithOffset(m.first),
                           range.getEnd().getLocWithOffset(-m.second));
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
