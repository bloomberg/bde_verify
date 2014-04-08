// csastil_externalguards.cpp                                         -*-C++-*-
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
#include <stack>
#include <string>
#include <utility>

using namespace clang;
using namespace bde_verify::csabase;

// ----------------------------------------------------------------------------

static std::string const check_name("external-guards");

// ----------------------------------------------------------------------------

namespace
{
    struct ExternalGuards
    {
        typedef std::pair<std::string, SourceLocation> condition_type;
        std::stack<condition_type> d_conditions;
    };
}

// ----------------------------------------------------------------------------

static void
onIfdef(Analyser*         analyser,
        SourceLocation where,
        Token const&   token)
{
    ExternalGuards& context(analyser->attachment<ExternalGuards>());
    // This condition is never part of an include guard.
    context.d_conditions.push(std::make_pair(std::string(), where));
}

// ----------------------------------------------------------------------------

static std::string const prefix0("INCLUDED_");

static void
onIfndef(Analyser*         analyser,
         SourceLocation where,
         Token const&   token)
{
    ExternalGuards& context(analyser->attachment<ExternalGuards>());
#if 0
    std::string macro(token.getIdentifierInfo()->getNameStart());
    context.d_conditions.push(std::make_pair(macro.find(prefix0) == 0
                                             ? macro.substr(prefix0.size())
                                             : std::string(), where));
#else
    context.d_conditions.push(
        std::make_pair(token.getIdentifierInfo()->getNameStart(), where));
#endif
}

// ----------------------------------------------------------------------------

//static std::string const prefix1("!definedINCLUDED_");
//static std::string const prefix2("!defined(INCLUDED_");
static std::string const prefix1("!defined");
static std::string const prefix2("!defined(");

static bool
isSpace(unsigned char c)
{
    return std::isspace(c);
}

static void
onIf(Analyser*         analyser,
     SourceLocation where,
     SourceRange    source)
{
    ExternalGuards& context(analyser->attachment<ExternalGuards>());
    std::string condition(analyser->get_source(source));
    condition.erase(std::remove_if(condition.begin(), condition.end(), &isSpace),
                    condition.end());
    if (condition.find(prefix2) == 0 && condition[condition.size() - 1] == ')') {
        context.d_conditions.push(std::make_pair(condition.substr(prefix2.size(), condition.size() - prefix2.size() - 1), where));
    }
    else if (condition.find(prefix1) == 0) {
        context.d_conditions.push(std::make_pair(condition.substr(prefix1.size()), where));
    }
    else {
        context.d_conditions.push(std::make_pair(std::string(), where));
    }
}

// ----------------------------------------------------------------------------

static std::string const hashif("#if");
static std::string const include0("#include<");
static std::string const include1("#include\"");

static std::string
getInclude(std::string const& value)
{
    std::string::size_type pos(value.find(hashif));
    if (pos != value.npos) {
        return std::string();
    }
    pos = value.find(include0);
    if (pos != value.npos) {
        std::string::size_type begin(pos + include0.size());
        std::string::size_type end(value.find('>', begin));
        return end == value.npos
            ? std::string()
            : value.substr(begin, end - begin);
    }
    pos = value.find(include1);
    if (pos != value.npos) {
        std::string::size_type begin(pos + include1.size());
        std::string::size_type end(value.find('"', begin));
        return end == value.npos
            ? std::string()
            : value.substr(begin, end - begin);
    }
    return std::string();
}

static void
onEndif(Analyser* analyser,
        SourceLocation end,
        SourceLocation)
{
    ExternalGuards& context(analyser->attachment<ExternalGuards>());
    if (!context.d_conditions.empty()) {
        std::string guard = context.d_conditions.top().first;
        SourceLocation where = context.d_conditions.top().second;
        std::string component_guard =
            "INCLUDED_" + llvm::StringRef(analyser->component()).upper();
        if (analyser->is_component_header(end)
            && !guard.empty()
            && guard != component_guard
            ) {
            std::string source(analyser->get_source(SourceRange(where, end)));
            source.erase(
                std::remove_if(source.begin(), source.end(), &isSpace),
                source.end());
            std::string include(getInclude(source));
            std::string include_guard =
                "INCLUDED_" + FileName(include).component().upper();
            if (include.empty()) {
                analyser->report(where, check_name, "SEG01",
                                 "Include guard '%0' without include file")
                    << guard;
            }
            else if (   include_guard != guard
                     && (   include.find('_') != include.npos
                         || include_guard + "_H" != guard)
                     ) {
                analyser->report(where, check_name, "SEG02",
                                 "Include guard '%0' mismatch for included "
                                 "file '%1' - use '%2'")
                    << guard
                    << include
                    << (include.find('_') == include.npos ?
                            include_guard + "_H" :
                            include_guard);
            }
        }
        context.d_conditions.pop();
    }
    else {
        // This "can't happen" unless Clang's PPObserver interface changes so
        // that our functions are no longer virtual overrides.  This happens
        // distressingly often.
        llvm::errs() << "Mismatched conditionals?\n";
    }
}

// ----------------------------------------------------------------------------

static void
onInclude(Analyser*         analyser,
          SourceLocation where,
          bool                  ,
          std::string const&    file)
{
    ExternalGuards& context(analyser->attachment<ExternalGuards>());
    if (analyser->is_component_header(where)
        && (context.d_conditions.empty()
            || context.d_conditions.top().first.empty()
            || context.d_conditions.top().first
                == "INCLUDED_" + llvm::StringRef(analyser->component()).upper()
            )
        )
    {
        analyser->report(where, check_name, "SEG03",
                         "Include of '%0' without external include guard "
                         "'%1'")
            << file
            << "INCLUDED_" + FileName(file).component().upper();
    }
}

// ----------------------------------------------------------------------------

static void
subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
{
    observer.onInclude  += bind(&analyser, &onInclude);
    // observer.onSkipFile += binder(&analyser);
    observer.onIfdef    += bind(&analyser, &onIfdef);
    observer.onIfndef   += bind(&analyser, &onIfndef);
    observer.onIf       += bind(&analyser, &onIf);
    observer.onEndif    += bind(&analyser, &onEndif);
}

// ----------------------------------------------------------------------------

static RegisterCheck register_observer(check_name, &subscribe);
