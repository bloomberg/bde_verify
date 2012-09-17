// csastil_leakingmacro.cpp                                           -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
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

namespace CB = cool::csabase;

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
            this->d_macros.push(map_type());
        }
    };
}

// ----------------------------------------------------------------------------

static void
onOpenFile(cool::csabase::Analyser* analyser,
           clang::SourceLocation    location,
           std::string const&       current,
           std::string const&       opened)
{
    ::leaking_macro& context(analyser->attachment< ::leaking_macro>());
    context.d_macros.push(::leaking_macro::map_type());
}

// ----------------------------------------------------------------------------

static char
to_upper(unsigned char c)
{
    return std::toupper(c);
}

static void
onCloseFile(cool::csabase::Analyser* analyser,
            clang::SourceLocation    location,
            std::string const&       current,
            std::string const&       closed)
{
    std::string::size_type slash(closed.rfind('/'));
    slash = slash == closed.npos? 0u: slash + 1;
    std::string::size_type point(closed.find('.', slash));
    point = point == closed.npos? closed.size(): point;
    std::string component(closed.substr(slash, point - slash));
    std::transform(component.begin(), component.end(),
                   component.begin(),
                   &::to_upper);

    ::leaking_macro& context(analyser->attachment< ::leaking_macro>());
    typedef ::leaking_macro::map_type map_type;
    map_type const& macros(context.d_macros.top());
    for (map_type::const_iterator it(macros.begin()), end(macros.end());
         it != end; ++it) {
        cool::csabase::Location where(analyser->get_location(it->second));
        if (where.file() != "<built-in>"
            && where.file() != "<command line>"
            && it->first != "INCLUDED_" + component
            && it->first.find(component) != 0
            && (analyser->is_component_header(it->second)
                ? true
                : it->first.size() < 4)
            )
        {
            analyser->report(it->second, ::check_name,
                             "SLM: macro definition '%0' leaks from header",
                             true)
                << it->first;
        }
    }
    context.d_macros.pop();
}

// ----------------------------------------------------------------------------

static void
onMacroDefined(cool::csabase::Analyser* analyser,
               clang::Token const&      token,
               clang::MacroInfo const*  info)
{
    ::leaking_macro& context(analyser->attachment< ::leaking_macro>());
    std::string source(token.getIdentifierInfo()->getNameStart());
    context.d_macros.top().insert(std::make_pair(source, token.getLocation()));
}

static void
onMacroUndefined(cool::csabase::Analyser* analyser,
                 clang::Token const&      token,
                 clang::MacroInfo const*  info)
{
    ::leaking_macro& context(analyser->attachment< ::leaking_macro>());
    std::string source(token.getIdentifierInfo()->getNameStart());
    context.d_macros.top().erase(source);
}

// ----------------------------------------------------------------------------

static void
subscribe(cool::csabase::Analyser&   analyser,
          cool::csabase::Visitor&    ,
          cool::csabase::PPObserver& observer)
{
    observer.onOpenFile       += cool::csabase::bind(&analyser, &onOpenFile);
    observer.onCloseFile      += cool::csabase::bind(&analyser, &onCloseFile);
    observer.onMacroDefined   += cool::csabase::bind(&analyser, &onMacroDefined);
    observer.onMacroUndefined += cool::csabase::bind(&analyser, &onMacroUndefined);
}

// ----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_observer(check_name, &::subscribe);
