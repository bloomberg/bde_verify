// csafmt_longlines.cpp                                                -*-C++-*-
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_location.h>
#include <string>

#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("longlines");

// ----------------------------------------------------------------------------

using clang::SourceLocation;
using clang::SourceManager;
using clang::SourceRange;
using csabase::Analyser;
using csabase::Location;
using csabase::PPObserver;
using csabase::Range;
using csabase::Visitor;

namespace
{

struct files
    // Callback object for inspecting files.
{
    Analyser& d_analyser;                   // Analyser object.

    files(Analyser& analyser);
        // Create a 'files' object, accessing the specified 'analyser'.

    void operator()(SourceLocation      loc,
                    std::string const &,
                    std::string const &);
        // The file specified by 'loc' is examined for long lines.

    void operator()();
        // End of TU callback - examine main file.
};

files::files(Analyser& analyser)
: d_analyser(analyser)
{
}

void files::operator()(SourceLocation     loc,
                       std::string const &,
                       std::string const &)
{
    const SourceManager &m = d_analyser.manager();
    clang::FileID fid = m.getFileID(loc);
    loc = m.getLocForStartOfFile(fid);
    llvm::StringRef b = m.getBufferData(fid);

    size_t prev = ~size_t(0);
    size_t next;
    do {
        if ((next = b.find('\n', prev + 1)) == b.npos) {
            next = b.size();
        }
        if (next - prev > 80) {
            d_analyser.report(loc.getLocWithOffset(prev + 80), check_name,
                              "LL01",
                              "Line exceeds 79 characters in length");
        }
    } while ((prev = next) < b.size());
}

void files::operator()()
{
    (*this)(d_analyser.manager().getLocForEndOfFile(
                d_analyser.manager().getMainFileID()),
            std::string(),
            std::string());
}

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
    // Hook up the callback functions.
{
    observer.onCloseFile += files(analyser);
    analyser.onTranslationUnitDone += files(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static csabase::RegisterCheck c1(check_name, &subscribe);
