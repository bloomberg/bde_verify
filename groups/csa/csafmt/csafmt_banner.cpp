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
    typedef std::map<std::string, std::vector<SourceRange> > Comments;
    Comments d_comments;

    void append(Analyser& analyser, SourceRange range);
};

void comments::append(Analyser& analyser, SourceRange range)
{
    SourceManager& m = analyser.manager();
    std::vector<SourceRange>& c = d_comments[m.getFilename(range.getBegin())];
    if (c.size() != 0 &&
        m.getFilename(c.back().getEnd()) == m.getFilename(range.getBegin()) &&
        m.getPresumedLineNumber(range.getBegin()) ==
        m.getPresumedLineNumber(c.back().getEnd()) + 1 &&
        analyser.get_source(SourceRange(c.back().getEnd(),
                                        range.getBegin().getLocWithOffset(-1)),
                            true).find_first_not_of(" \t\n\r\v\f") ==
            llvm::StringRef::npos) {
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

static const std::string sep = "// " + std::string(76, '=');
static const std::string inline_banner_expect =  // correct banner
    sep + "\n//" + std::string(25, ' ') + "INLINE FUNCTION DEFINITIONS\n" + sep;

#undef NL
#define NL "\r*[\r\n]"
#undef SP
#define SP "[ \t]*"

static llvm::Regex loose_inline_banner(    // match sloppy banner
    SP "//" SP "[-=]*"  SP                NL
    SP "//" SP "INLINE"
            SP "(" "FUNCTIONS?"   SP ")?"
            SP "(" "DEFINITIONS?" SP ")?" NL
    SP "//" SP "[-=]*"  SP,
    llvm::Regex::IgnoreCase);

static llvm::Regex tight_inline_banner(    // match correct banner
    "// ={76}"                            NL
    "// {25}INLINE FUNCTION DEFINITIONS"  NL
    "// ={76}",
    llvm::Regex::NoFlags);

#undef SP
#undef NL

void files::operator()()
{
    comments::Comments& c = d_analyser.attachment<comments>().d_comments;
    for (comments::Comments::iterator itr = c.begin(); itr != c.end(); ++itr) {
        if (d_analyser.is_component(itr->first)) {
            std::vector<SourceRange>& b = itr->second;
            size_t n = b.size();
            for (size_t i = 0; i < n; ++i) {
                llvm::StringRef s = d_analyser.get_source(b[i], true);
                if (loose_inline_banner.match(s) &&
                    !tight_inline_banner.match(s)) {
                    std::pair<unsigned, unsigned> m =
                        cool::csabase::mid_mismatch(s, inline_banner_expect);
                    SourceRange r(b[i].getBegin().getLocWithOffset(m.first),
                                  b[i].getEnd().getLocWithOffset(m.second));
                    d_analyser.report(r.getBegin(), check_name, "BAN01",
                                      "Inline banner with incorrect form "
                                      "is\n%0\nand should be\n%1")
                        << s
                        << inline_banner_expect
                        << FixItHint::CreateReplacement(
                               r,
                               inline_banner_expect.substr(
                                   m.first,
                                   inline_banner_expect.size() - m.first -
                                       m.second));
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
