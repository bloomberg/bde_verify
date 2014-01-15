// csafmt_banner.cpp                                                  -*-C++-*-
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

static std::string const check_name("banner");

// ----------------------------------------------------------------------------

using clang::FileID;
using clang::FixItHint;
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
        // Report improper banners.
};

files::files(Analyser& analyser)
: d_analyser(analyser)
{
}

void files::operator()(SourceRange range)
{
    d_analyser.attachment<comments>().append(d_analyser, range);
}

#undef NL
#define NL "\r*[\r\n]"
#undef SP
#define SP "[ \t]*"

static llvm::Regex generic_banner(   // things that look like banners
       "//" SP "(" "[-=_]" "(" SP "[-=_]" ")*" ")"                SP  NL
    SP "//" SP "(" "[_[:alnum:]]" "(" SP "[_[:alnum:]]" ")*" ")"  SP  NL
    SP "//" SP "(" "[-=_]" "(" SP "[-=_]" ")*" ")"                SP,
    llvm::Regex::IgnoreCase);

#undef SP
#undef NL

void files::operator()()
{
    SourceManager& manager = d_analyser.manager();
    comments::Comments& file_comments =
        d_analyser.attachment<comments>().d_comments;
    llvm::SmallVector<llvm::StringRef, 7> matches;

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
            SourceRange comment_range = *comments_itr;
            llvm::StringRef comment =
                d_analyser.get_source(comment_range, true);
            unsigned comment_offset = 0;
            for (llvm::StringRef comment_suffix = comment; 
                 generic_banner.match(comment_suffix, &matches);
                 comment_suffix = comment.drop_front(comment_offset)) {
                llvm::StringRef banner = matches[0];
                unsigned banner_pos =
                    comment_offset + comment_suffix.find(banner);
                comment_offset = banner_pos + banner.size();
                SourceLocation banner_start =
                    comment_range.getBegin().getLocWithOffset(banner_pos);
                if (manager.getPresumedColumnNumber(banner_start) != 1) {
                    continue;
                }
                unsigned end_of_first_line = banner.find('\n');
                if (end_of_first_line != 80) {
                    d_analyser.report(
                        banner_start.getLocWithOffset(end_of_first_line - 1),
                        check_name, "BAN02",
                        "Banner ends at column %0 instead of 79")
                        << end_of_first_line;
                }

                llvm::StringRef text = matches[3];
                unsigned text_pos = banner.find(text);
                unsigned actual_last_space_pos =
                    manager.getPresumedColumnNumber(
                        banner_start.getLocWithOffset(text_pos)) -
                    1;
                unsigned expected_last_space_pos =
                    ((79 - 2 - text.size()) / 2 + 2) & ~3;
                if (actual_last_space_pos != expected_last_space_pos) {
                    std::string expected_text =
                        "//" + std::string(expected_last_space_pos - 2, ' ') +
                        text.str();
                    const char* error = (actual_last_space_pos & 3) ?
                        "Improperly centered banner text"
                        " (not reachable using tab key)" :
                        "Improperly centered banner text";
                    d_analyser.report(
                        banner_start.getLocWithOffset(text_pos),
                        check_name, "BAN03", error);
                    d_analyser.report(
                        banner_start.getLocWithOffset(text_pos),
                        check_name, "BAN03", "Correct text is\n%0", false,
                        clang::DiagnosticsEngine::Note)
                        << expected_text;
                }

                unsigned end_of_second_line = banner.rfind('\n');
                if (banner.size() - end_of_second_line != 80) {
                    d_analyser.report(banner_start.getLocWithOffset(
                                          banner.size() - 1),
                                      check_name,
                                      "BAN02",
                                      "Banner ends at column %0 instead of 79")
                        << static_cast<int>(banner.size() -
                                            (end_of_second_line + 1));
                }
            }
        }
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
