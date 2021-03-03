// csastil_leakingmacro.cpp                                           -*-C++-*-

#include <clang/Basic/IdentifierTable.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Lex/Token.h>
#include <csabase_analyser.h>
#include <csabase_binder.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_filenames.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <llvm/ADT/StringRef.h>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <map>
#include <stack>
#include <string>
#include <utility>

namespace clang { class MacroDirective; }
namespace csabase { class Visitor; }

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("leaking-macro");

// ----------------------------------------------------------------------------

namespace
{
    struct leaking_macro
    {
        typedef std::map<std::string, SourceLocation> map_type;
        std::stack<map_type> d_macros;
        leaking_macro()
        {
            d_macros.push(map_type());
        }
    };
}

// ----------------------------------------------------------------------------

static void onOpenFile(Analyser* analyser,
                       SourceLocation location,
                       std::string const& current,
                       std::string const& opened)
{
    leaking_macro& context(analyser->attachment<leaking_macro>());
    context.d_macros.push(leaking_macro::map_type());
}

// ----------------------------------------------------------------------------

static void onCloseFile(Analyser* analyser,
                        SourceLocation location,
                        std::string const& current,
                        std::string const& closed)
{
    SourceManager& m = analyser->manager();
    if (location.isValid() && m.getMainFileID() == m.getFileID(location)) {
        return;                                                       // RETURN
    }

    FileName fn(closed);
    std::string component = llvm::StringRef(fn.component()).upper();

    leaking_macro& context(analyser->attachment<leaking_macro>());
    for (const auto& macro : context.d_macros.top()) {
        Location where(analyser->get_location(macro.second));
        if (   analyser->is_header(where.file())
            && macro.first.find("INCLUDED_") != 0
            && (   analyser->is_component_header(macro.second)
                || macro.first.size() < 4)
            && llvm::StringRef(macro.first).upper().find(component) != 0
            )
        {
            analyser->report(macro.second, check_name, "SLM01",
                             "Macro definition '%0' leaks from header")
                << macro.first;
        }
    }
    context.d_macros.pop();
}

// ----------------------------------------------------------------------------

static void onMacroDefined(Analyser* analyser,
                           Token const& token,
                           MacroDirective const* info)
{
    leaking_macro& context(analyser->attachment<leaking_macro>());
    std::string source(token.getIdentifierInfo()->getNameStart());
    context.d_macros.top().insert(std::make_pair(source, token.getLocation()));
}

static void onMacroUndefined(Analyser* analyser,
                             Token const& token,
                             MacroDirective const* info)
{
    leaking_macro& context(analyser->attachment<leaking_macro>());
    std::string source(token.getIdentifierInfo()->getNameStart());
    context.d_macros.top().erase(source);
}

static void onTranslationUnitDone(Analyser* analyser)
{
    SourceManager& m = analyser->manager();
    onCloseFile(analyser,
                SourceLocation(),
                "",
                m.getFilename(m.getLocForStartOfFile(m.getMainFileID())).str());
}

// ----------------------------------------------------------------------------

static void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
{
    observer.onOpenFile            += bind(&analyser, &onOpenFile);
    observer.onCloseFile           += bind(&analyser, &onCloseFile);
    observer.onMacroDefined        += bind(&analyser, &onMacroDefined);
    observer.onMacroUndefined      += bind(&analyser, &onMacroUndefined);
    analyser.onTranslationUnitDone += bind(&analyser, &onTranslationUnitDone);
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
