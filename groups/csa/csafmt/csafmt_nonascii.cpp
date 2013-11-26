// csabbg_nonascii.cpp                                                -*-C++-*-
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
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

struct comments
    // Callback object for inspecting comments.
{
    Analyser& d_analyser;                   // Analyser object.

    comments(Analyser& analyser);
        // Create a 'comments' object, accessing the specified 'analyser'.

    void operator()(SourceRange range);
        // The specified 'range', representing a comment, is examined for
        // non-ascii characters.
};

comments::comments(Analyser& analyser)
: d_analyser(analyser)
{
}

void comments::operator()(SourceRange range)
{
    const std::string s(d_analyser.get_source(range));

    int begin = -1;
    for (size_t i = 0; i <= s.length(); ++i) {
        if (!(s[i] & 0x80)) {
            if (begin >= 0) {
                SourceRange bad(range.getBegin().getLocWithOffset(begin),
                                range.getBegin().getLocWithOffset(i - 1));
                d_analyser.report(bad.getBegin(),
                                  check_name, "NA01: "
                                  "Non-ASCII comment characters")
                    << bad;
                begin = -1;
            }
        } else {
            if (begin == -1) {
                begin = i;
            }
        }
    }
}

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
    // Hook up the callback functions.
{
    observer.onComment += comments(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static cool::csabase::RegisterCheck c1(check_name, &subscribe);
