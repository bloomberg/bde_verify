// csafmt_nonascii.cpp                                                -*-C++-*-
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_location.h>
#include <string>

#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("nonascii");

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

    void operator()(SourceLocation     loc,
                    std::string const &from,
                    std::string const &file);
        // The file specified by 'loc' is examined for non-ascii characters.
};

files::files(Analyser& analyser)
: d_analyser(analyser)
{
}

void files::operator()(SourceLocation     loc,
                       std::string const &from,
                       std::string const &file)
{
    const SourceManager &m = d_analyser.manager();
    const llvm::MemoryBuffer *buf = m.getBuffer(m.getFileID(loc));
    const char *b = buf->getBufferStart();
    const char *e = buf->getBufferEnd();

    const char *begin = 0;
    for (const char *s = b; s <= e; ++s) {
        if (!(*s & 0x80)) {
            if (begin != 0) {
                SourceRange bad(loc.getLocWithOffset(begin - b),
                                loc.getLocWithOffset(s - b - 1));
                d_analyser.report(bad.getBegin(), check_name,
                                  "NA01: Non-ASCII characters")
                    << bad;
                begin = 0;
            }
        } else {
            if (begin == 0) {
                begin = s;
            }
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
