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

namespace CB = cool::csabase;

// ----------------------------------------------------------------------------

static std::string const check_name("external-guards");

// ----------------------------------------------------------------------------

namespace
{
    struct ExternalGuards
    {
        typedef std::pair<std::string, clang::SourceLocation> condition_type;
        std::stack<condition_type> d_conditions;
    };
}

static char
toUpper(unsigned char c)
{
    return std::toupper(c);
}

static std::string
toUpper(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
                   static_cast<char(*)(unsigned char)>(&toUpper));
    return value;
}

static std::string
getComponent(std::string const& file)
{
    cool::csabase::FileName fn(file);
    return toUpper(fn.component());
}

// ----------------------------------------------------------------------------

static void
onIfdef(CB::Analyser*         analyser,
        clang::SourceLocation where,
        clang::Token const&   token)
{
    ExternalGuards& context(analyser->attachment<ExternalGuards>());
    // This condition is never part of an include guard.
    context.d_conditions.push(std::make_pair(std::string(), where));
}

// ----------------------------------------------------------------------------

static std::string const prefix0("INCLUDED_");

static void
onIfndef(CB::Analyser*         analyser,
         clang::SourceLocation where,
         clang::Token const&   token)
{
    ExternalGuards& context(analyser->attachment<ExternalGuards>());
    std::string macro(token.getIdentifierInfo()->getNameStart());
    context.d_conditions.push(std::make_pair(macro.find(prefix0) == 0
                                             ? macro.substr(prefix0.size())
                                             : std::string(), where));
}

// ----------------------------------------------------------------------------

static std::string const prefix1("!definedINCLUDED_");
static std::string const prefix2("!defined(INCLUDED_");

static bool
isSpace(unsigned char c)
{
    return std::isspace(c);
}

static void
onIf(CB::Analyser*         analyser,
     clang::SourceLocation where,
     clang::SourceRange    source)
{
    ExternalGuards& context(analyser->attachment<ExternalGuards>());
    std::string condition(analyser->get_source(source));
    condition.erase(std::remove_if(condition.begin(), condition.end(), &isSpace),
                    condition.end());
    if (condition.find(prefix1) == 0) {
        context.d_conditions.push(std::make_pair(condition.substr(prefix1.size()), where));
    }
    else if (condition.find(prefix2) == 0 && condition[condition.size() - 1] == ')') {
        context.d_conditions.push(std::make_pair(condition.substr(prefix2.size(), condition.size() - prefix2.size() - 1), where));
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
            : getComponent(value.substr(begin, end - begin));
    }
    pos = value.find(include1);
    if (pos != value.npos) {
        std::string::size_type begin(pos + include1.size());
        std::string::size_type end(value.find('"', begin));
        return end == value.npos
            ? std::string()
            : getComponent(value.substr(begin, end - begin));
    }
    return std::string();
}

static void
onEndif(CB::Analyser* analyser,
        clang::SourceLocation end,
        clang::SourceLocation)
{
    ExternalGuards& context(analyser->attachment<ExternalGuards>());
    if (!context.d_conditions.empty()) {
        if (analyser->is_component_header(end)
            && !context.d_conditions.top().first.empty()
            && (context.d_conditions.top().first
                != toUpper(analyser->component()))
            ) {
            std::string source(analyser->get_source(clang::SourceRange(context.d_conditions.top().second, end)));
            source.erase(std::remove_if(source.begin(), source.end(), &isSpace), source.end());
            std::string include(getInclude(source));
            if (include.empty()) {
                analyser->report(context.d_conditions.top().second, check_name,
                                 "SEG: include guard without include file");
            }
            else if (   include != context.d_conditions.top().first
                     && (   include.find('_') != include.npos
                         || include + "_H" !=
                                context.d_conditions.top().first)) {
                analyser->report(context.d_conditions.top().second, check_name,
                                 "SEG: include guard mismatching include file");
            }
        }
        context.d_conditions.pop();
    }
    else {
        llvm::errs() << "mismatched conditionals?\n"; //-dk:TODO remove
    }
}

// ----------------------------------------------------------------------------

static void
onInclude(CB::Analyser*         analyser,
          clang::SourceLocation where,
          bool                  ,
          std::string const&    file)
{
    ExternalGuards& context(analyser->attachment<ExternalGuards>());
    if (analyser->is_component_header(where)
        && (context.d_conditions.empty()
            || context.d_conditions.top().first.empty()
            || (context.d_conditions.top().first
                == toUpper(analyser->component()))
            )
        )
    {
        analyser->report(where, check_name,
                         "SEG: include without external include guard");
    }
}

// ----------------------------------------------------------------------------

static void
subscribe(CB::Analyser& analyser, CB::Visitor&, CB::PPObserver& observer)
{
    observer.onInclude  += CB::bind(&analyser, &onInclude);
    // observer.onSkipFile += binder(&analyser);
    observer.onIfdef    += CB::bind(&analyser, &onIfdef);
    observer.onIfndef   += CB::bind(&analyser, &onIfndef);
    observer.onIf       += CB::bind(&analyser, &onIf);
    observer.onEndif    += CB::bind(&analyser, &onEndif);
}

// ----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_observer(check_name, &subscribe);
