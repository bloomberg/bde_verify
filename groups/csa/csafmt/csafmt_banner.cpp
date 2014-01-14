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

struct files
    // Callback object for inspecting files.
{
    Analyser& d_analyser;                   // Analyser object.

    files(Analyser& analyser);
        // Create a 'files' object, accessing the specified 'analyser'.

    void operator()(SourceLocation     loc,
                    std::string const &from,
                    std::string const &file);
        // The file specified by 'loc' is examined for improper banners.
};

files::files(Analyser& analyser)
: d_analyser(analyser)
{
}

void files::operator()(SourceLocation     loc,
                       std::string const &from,
                       std::string const &file)
{
    if (!d_analyser.is_component(file)) {
        return;                                                       // RETURN
    }

    const SourceManager &m = d_analyser.manager();
    llvm::StringRef buf = m.getBufferData(m.getFileID(loc));

    static const std::string sep = "\n// " + std::string(76, '=') + "\n";
    static const std::string inline_banner_expect = // correct banner
        sep +
        "//" + std::string(25, ' ') + "INLINE FUNCTION DEFINITIONS" +
        sep;

#undef NL
#define NL "\r*[\r\n]"
#undef SP
#define SP "[ \t]*"

    static llvm::Regex loose_inline_banner(    // match sloppy banner
                                                                            NL
            SP "//" SP "[-=]*"  SP                                          NL
            SP "//" SP "INLINE"
                    SP "(" "FUNCTIONS?"   SP ")?"
                    SP "(" "DEFINITIONS?" SP ")?"                           NL
            SP "//" SP "[-=]*"  SP                                          NL,
        llvm::Regex::IgnoreCase);

    static llvm::Regex tight_inline_banner(    // match correct banner
                                                                            NL
            "// ={76}"                                                      NL
            "// {25}INLINE FUNCTION DEFINITIONS"                            NL
            "// ={76}"                                                      NL,
        llvm::Regex::NoFlags);

#undef SP
#undef NL

    llvm::SmallVector<llvm::StringRef, 3> matches;

    while (loose_inline_banner.match(buf, &matches)) {
        llvm::StringRef match = matches[0];
        size_t offset         = buf.find(match);
        size_t end            = offset + match.size();

        if (!tight_inline_banner.match(buf)) {
            std::pair<unsigned, unsigned> m =
                cool::csabase::mid_mismatch(match, inline_banner_expect);
            SourceRange r(loc.getLocWithOffset(offset + m.first),
                          loc.getLocWithOffset(end    - m.second));
            d_analyser.report(r.getBegin(),
                              check_name, "BAN01",
                              "Inline banner with incorrect form "
                              "is %0 and should be %1")
                << match
                << inline_banner_expect
                << FixItHint::CreateReplacement(r, inline_banner_expect.substr(
                            m.first,
                            inline_banner_expect.size() - m.first - m.second));
        }
        buf = buf.drop_front(end);
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
