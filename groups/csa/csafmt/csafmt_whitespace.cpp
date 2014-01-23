// csafmt_whitespace.cpp                                              -*-C++-*-
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

static std::string const check_name("whitespace");

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

struct files
    // Callback object for inspecting files.
{
    Analyser& d_analyser;                   // Analyser object.

    files(Analyser& analyser);
        // Create a 'files' object, accessing the specified 'analyser'.

    void operator()(SourceLocation loc,
                    std::string const &,
                    std::string const &);
        // The file specified by 'loc' is examined for tab characters and
        // trailing spaces.
};

files::files(Analyser& analyser)
: d_analyser(analyser)
{
}

llvm::Regex bad_ws("\t+| +\n", llvm::Regex::NoFlags);

void files::operator()(SourceLocation loc,
                       std::string const &,
                       std::string const &)
{
    const SourceManager &m = d_analyser.manager();
    llvm::StringRef buf = m.getBufferData(m.getFileID(loc));
    loc = m.getLocForStartOfFile(m.getFileID(loc));
    size_t offset = 0;
    llvm::StringRef s;
    llvm::SmallVector<llvm::StringRef, 7> matches;
    while (bad_ws.match(s = buf.drop_front(offset), &matches)) {
        llvm::StringRef text = matches[0];
        std::pair<size_t, size_t> m = cool::csabase::mid_match(s, text);
        size_t matchpos = offset + m.first;
        offset = matchpos + text.size();
        if (text[0] == '\t') {
            d_analyser.report(loc.getLocWithOffset(matchpos),
                    check_name, "TAB01",
                    "Tab character%s0 in source")
                << static_cast<long>(text.size());
        }
        else {
            d_analyser.report(loc.getLocWithOffset(matchpos),
                    check_name, "ESP01",
                    "Space%s0 at end of line")
                << static_cast<long>(text.size() - 1);
        }
    }
}

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
    // Hook up the callback functions.
{
    observer.onOpenFile += files(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static cool::csabase::RegisterCheck c1(check_name, &subscribe);
