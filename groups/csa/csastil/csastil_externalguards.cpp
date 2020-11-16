// csastil_externalguards.cpp                                         -*-C++-*-

#include <clang/Basic/IdentifierTable.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Lex/Token.h>
#include <csabase_analyser.h>
#include <csabase_binder.h>
#include <csabase_debug.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_filenames.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Regex.h>
#include <llvm/Support/raw_ostream.h>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <stack>
#include <string>
#include <utility>

namespace csabase { class Visitor; }

using namespace clang;
using namespace csabase;

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
onIfdef(Analyser* analyser, SourceLocation where, Token const& token)
{
    ExternalGuards& context(analyser->attachment<ExternalGuards>());
    // This condition is never part of an include guard.
    context.d_conditions.push(std::make_pair(std::string(), where));
}

// ----------------------------------------------------------------------------

static void
onIfndef(Analyser* analyser, SourceLocation where, Token const& token)
{
    llvm::StringRef guard = token.getIdentifierInfo()->getName();
    if (!guard.startswith("INCLUDE")) {
        guard = llvm::StringRef();
    }
    analyser->attachment<ExternalGuards>().d_conditions.push(
        std::make_pair(guard.str(), where));
}

// ----------------------------------------------------------------------------

static llvm::Regex ndef(
    "^ *! *defined *[(]? *(INCLUDE[_[:alnum:]]*) *[)]?[[:space:]]*$");

static void onIf(Analyser* analyser, SourceLocation where, SourceRange source)
{
    llvm::SmallVector<llvm::StringRef, 7> matches;
    llvm::StringRef guard;
    if (ndef.match(analyser->get_source(source), &matches)) {
        guard = matches[1];
    }
    analyser->attachment<ExternalGuards>().d_conditions.push(
        std::make_pair(guard.str(), where));
}

// ----------------------------------------------------------------------------

static llvm::Regex next_include_before_if(
    "(^ *# *if)|"                     // 1
    "(^ *# *include *\"([^\"]*)\")|"  // 2, 3
    "(^ *# *include *<([^>]*)>)",     // 4, 5
    llvm::Regex::Newline
);

static std::string getInclude(llvm::StringRef source)
{
    llvm::SmallVector<llvm::StringRef, 7> matches;
    if (next_include_before_if.match(source, &matches)) {
        if (matches[3].size()) {
            return matches[3].str();
        }
        if (matches[5].size()) {
            return matches[5].str();
        }
    }
    return std::string();
}

static void onEndif(Analyser* analyser, SourceLocation end, SourceLocation)
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
            std::string include = getInclude(
                analyser->get_source(SourceRange(where, end), true));
            std::string include_guard =
                "INCLUDED_" + FileName(include).component().upper();
            if (include.empty()) {
                auto report = analyser->report(where, check_name, "SEG01",
                                 "Include guard '%0' without include file");
                report << guard;
            }
            else if (   include_guard != guard
                     && (   include.find('_') != include.npos
                         || include_guard + "_H" != guard)
                     ) {
                auto report = analyser->report(where, check_name, "SEG02",
                                 "Include guard '%0' mismatch for included "
                                 "file '%1' - use '%2'");
                report  << guard
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

static void onInclude(Analyser* analyser,
                      SourceLocation where,
                      bool,
                      std::string const& file)
{
    ExternalGuards& context(analyser->attachment<ExternalGuards>());

    bool unguarded =
        context.d_conditions.empty() ||
        context.d_conditions.top().first.empty() ||
        context.d_conditions.top().first ==
            "INCLUDED_" + llvm::StringRef(analyser->component()).upper();

    if (analyser->is_component_header(where)) {
        if (unguarded) {
            auto report = analyser->report(where, check_name, "SEG03",
                             "Include of '%0' without external include guard "
                             "'%1'");
            report << file
                   << "INCLUDED_" + FileName(file).component().upper();
        }
        else {
            std::string guard = context.d_conditions.top().first;
            auto report = analyser->report(where, check_name, "SEG04",
                             "Include of '%0' with external include guard "
                             "'%1'");
            report << file
                   << guard;
        }
    }
}

// ----------------------------------------------------------------------------

static void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
{
    observer.onInclude  += bind(&analyser, &onInclude);
    observer.onIfdef    += bind(&analyser, &onIfdef);
    observer.onIfndef   += bind(&analyser, &onIfndef);
    observer.onIf       += bind(&analyser, &onIf);
    observer.onEndif    += bind(&analyser, &onEndif);
}

// ----------------------------------------------------------------------------

static RegisterCheck register_observer(check_name, &subscribe);

// ----------------------------------------------------------------------------
// Copyright (C) 2014 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
