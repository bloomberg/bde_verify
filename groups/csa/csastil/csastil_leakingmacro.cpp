// csastil_leakingmacro.cpp                                           -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_binder.h>
#include <csabase_filenames.h>
#include <csabase_registercheck.h>
#include <csabase_ppobserver.h>
#include <csabase_location.h>
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

static void
onCloseFile(csabase::Analyser* analyser,
            clang::SourceLocation    location,
            std::string const&       current,
            std::string const&       closed)
{
    csabase::FileName fn(closed);
    std::string component = llvm::StringRef(fn.component()).upper();

    leaking_macro& context(analyser->attachment<leaking_macro>());
    for (const auto& macro : context.d_macros.top()) {
        csabase::Location where(analyser->get_location(macro.second));
        if (   where.file() != "<built-in>"
            && where.file() != "<command line>"
            && macro.first.find("INCLUDED_") != 0
            && (   analyser->is_component_header(macro.second)
                || macro.first.size() < 4)
            && llvm::StringRef(macro.first).upper().find(component) != 0
            )
        {
            analyser->report(macro.second, check_name, "SLM01",
                             "Macro definition '%0' leaks from header",
                             true)
                << macro.first;
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
