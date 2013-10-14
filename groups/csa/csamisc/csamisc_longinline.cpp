// csastil_longinline.cpp                                             -*-C++-*-
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_config.h>
#include <csabase_location.h>
#include <csabase_registercheck.h>
#include <sstream>

#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("long-inline");

// ----------------------------------------------------------------------------

using clang::FunctionDecl;
using clang::PresumedLoc;
using clang::Stmt;
using clang::SourceManager;

using cool::csabase::Analyser;
using cool::csabase::Location;
using cool::csabase::PPObserver;
using cool::csabase::RegisterCheck;
using cool::csabase::Visitor;

namespace
{

// Data attached to analyzer for this check.
struct data
{
    enum E {
        e_ENDS_IN_OTHER_FILE,
        e_TOO_LONG,
        e_CONFUSED,
    };

    unsigned d_max_lines;

    // Function and its problem.
    typedef std::vector<std::pair<const FunctionDecl*, E> > VecFE;
    VecFE d_long_inlines;
};

// Callback function for inspecting function declarations.
void long_inlines(Analyser& analyser, const FunctionDecl* func)
{
    // Process only function definitions, not declarations.
    if (func->hasBody() && func->isInlineSpecified()) {
        data&           d    = analyser.attachment<data>();
        data::VecFE&    ad   = d.d_long_inlines;
        Stmt           *body = func->getBody();
        SourceManager&  sm   = analyser.manager();
        PresumedLoc     b    = sm.getPresumedLoc(body->getLocStart());
        PresumedLoc     e    = sm.getPresumedLoc(body->getLocEnd());

        if (b.isInvalid() || e.isInvalid()) {
            ad.push_back(std::make_pair(func, data::e_CONFUSED));
        } else if (strcmp(b.getFilename(), e.getFilename()) != 0) {
            ad.push_back(std::make_pair(func, data::e_ENDS_IN_OTHER_FILE));
        } else if (e.getLine() < b.getLine()) {
            ad.push_back(std::make_pair(func, data::e_CONFUSED));
        } else if (e.getLine() - b.getLine() >= d.d_max_lines) {
            ad.push_back(std::make_pair(func, data::e_TOO_LONG));
        }
    }
}

// Callback object invoked upon completion.
struct report
{
    Analyser* d_analyser;

    report(Analyser& analyser) : d_analyser(&analyser) {}

    void operator()()
    {
        const data& d = d_analyser->attachment<data>();
        process(d.d_long_inlines.begin(), d.d_long_inlines.end());
    }

    template <class IT>
    void process(IT begin, IT end)
    {
        const data& d = d_analyser->attachment<data>();
        for (IT it = begin; it != end; ++it) {
            std::ostringstream ss;
            ss << "HR002: ";
            switch (it->second) {
              case data::e_ENDS_IN_OTHER_FILE: {
                  ss << "Inline function spans source files";
              } break;
              case data::e_TOO_LONG: {
                  ss << "Inline function is more than "
                     << d.d_max_lines
                     << " lines long";
              } break;
              case data::e_CONFUSED: {
                  ss << "Compiler state for inline function is confused";
              } break;
              default: {
                  ss << "Unknown inline function problem " << it->second;
              } break;
            }
            d_analyser->report(it->first->getNameInfo().getLoc(), check_name, ss.str());
        }
    }
};

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
{
    std::istringstream is(analyser.config()->value("max_inline_lines"));
    data&              d = analyser.attachment<data>();
    if (!(is >> d.d_max_lines)) {
        d.d_max_lines = 10;
    }

    analyser.onTranslationUnitDone += report(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static RegisterCheck c1(check_name, &long_inlines);
static RegisterCheck c3(check_name, &subscribe);
