// csastil_leakingmacro.cpp                                           -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_filenames.h>
#include <csabase_registercheck.h>
#include <csabase_ppobserver.h>
#include <csabase_location.h>
#include <csabase_binder.h>
#include <cctype>
#include <algorithm>
#include <functional>
#include <map>
#include <stack>
#include <stack>
#include <utility>

namespace CB = csabase;

// ----------------------------------------------------------------------------

static std::string const check_name("leaking-macro");

// ----------------------------------------------------------------------------

namespace
{
    struct leaking_macro
    {
        typedef std::map<std::string, clang::SourceLocation> map_type;
        std::stack<map_type> d_macros;
        leaking_macro()
        {
            d_macros.push(map_type());
        }
    };
}

// ----------------------------------------------------------------------------

static void
onOpenFile(csabase::Analyser* analyser,
           clang::SourceLocation    location,
           std::string const&       current,
           std::string const&       opened)
{
    leaking_macro& context(analyser->attachment<leaking_macro>());
    context.d_macros.push(leaking_macro::map_type());
}

// ----------------------------------------------------------------------------

static char
to_upper(unsigned char c)
{
    return std::toupper(c);
}

static std::string
toUpper(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), &to_upper);
    return value;
}

static void
onCloseFile(csabase::Analyser* analyser,
            clang::SourceLocation    location,
            std::string const&       current,
            std::string const&       closed)
{
    csabase::FileName fn(closed);
    std::string component = fn.component();
    std::transform(component.begin(), component.end(),
                   component.begin(),
                   &to_upper);

    leaking_macro& context(analyser->attachment<leaking_macro>());
    typedef leaking_macro::map_type map_type;
    map_type const& macros(context.d_macros.top());
    for (map_type::const_iterator it(macros.begin()), end(macros.end());
         it != end; ++it) {
        csabase::Location where(analyser->get_location(it->second));
        if (where.file() != "<built-in>"
            && where.file() != "<command line>"
            && it->first.find("INCLUDED_") != 0
            && toUpper(it->first).find(component) != 0
            && (analyser->is_component_header(it->second)
                ? true
                : it->first.size() < 4)
            )
        {
            analyser->report(it->second, check_name, "SLM01",
                             "Macro definition '%0' leaks from header",
                             true)
                << it->first;
        }
    }
    context.d_macros.pop();
}

// ----------------------------------------------------------------------------

static void
onMacroDefined(csabase::Analyser* analyser,
               clang::Token const&      token,
               clang::MacroDirective const*  info)
{
    leaking_macro& context(analyser->attachment<leaking_macro>());
    std::string source(token.getIdentifierInfo()->getNameStart());
    context.d_macros.top().insert(std::make_pair(source, token.getLocation()));
}

static void
onMacroUndefined(csabase::Analyser* analyser,
                 clang::Token const&      token,
                 clang::MacroDirective const*  info)
{
    leaking_macro& context(analyser->attachment<leaking_macro>());
    std::string source(token.getIdentifierInfo()->getNameStart());
    context.d_macros.top().erase(source);
}

// ----------------------------------------------------------------------------

static void
subscribe(csabase::Analyser&   analyser,
          csabase::Visitor&    ,
          csabase::PPObserver& observer)
{
    observer.onOpenFile       += csabase::bind(&analyser, &onOpenFile);
    observer.onCloseFile      += csabase::bind(&analyser, &onCloseFile);
    observer.onMacroDefined   += csabase::bind(&analyser, &onMacroDefined);
    observer.onMacroUndefined += csabase::bind(&analyser, &onMacroUndefined);
}

// ----------------------------------------------------------------------------

static csabase::RegisterCheck register_observer(check_name, &subscribe);
