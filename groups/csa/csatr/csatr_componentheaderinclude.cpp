// csatr_componentheaderinclude.cpp                                   -*-C++-*-

#include <clang/AST/Decl.h>
#include <clang/Basic/SourceLocation.h>
#include <csabase_analyser.h>
#include <csabase_binder.h>
#include <csabase_filenames.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <llvm/Support/Casting.h>
#include <stddef.h>
#include <utils/array.hpp>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <algorithm>
#include <limits>
#include <string>

namespace clang { class Decl; }
namespace csabase { class Visitor; }

using namespace clang;
using namespace csabase;

// ----------------------------------------------------------------------------

static std::string const check_name("component-header");

// ----------------------------------------------------------------------------

namespace
{
    struct data
    {
        data()
        : check_(true)
        , header_seen_(false)
        , line_(std::numeric_limits<size_t>::max())
        {
        }
        bool   check_;
        bool   header_seen_;
        size_t line_;
    };
}

// ----------------------------------------------------------------------------

static std::string const builtin("<built-in>");
static std::string const command_line("<command line>");
static std::string const id_names[] = { "RCSId" };

// ----------------------------------------------------------------------------

static void close_file(Analyser& analyser,
                       SourceLocation where,
                       std::string const&,
                       std::string const& name)
{
    if (analyser.is_component_header(name))
    {
        analyser.attachment<data>().line_ =
            analyser.get_location(where).line();
    }
}

// ----------------------------------------------------------------------------

static void include_file(Analyser& analyser,
                         SourceLocation where,
                         bool,
                         std::string const& name)
{
    data& status(analyser.attachment<data>());
    if (status.check_)
    {
        if (analyser.is_component_header(name) ||
            analyser.is_component_header(analyser.toplevel()))
        {
            status.header_seen_ = true;
        }
        else if (!status.header_seen_
                 && !analyser.is_main()
                 && !analyser.is_toplevel(name)
                 && builtin != name
                 && command_line != name
                 && "bdes_ident.h" != FileName(name).name()
                 && "bsls_ident.h" != FileName(name).name())
        {
            analyser.report(where, check_name, "TR09",
                            "Include files precede component header",
                            true);
            status.check_ = false;
        }
    }
}

// ----------------------------------------------------------------------------

static void declaration(Analyser& analyser, Decl const* decl)
{
    data& status(analyser.attachment<data>());
    if (status.check_)
    {
        if (!status.header_seen_ ||
            analyser.is_component_header(analyser.toplevel()))
        {
            status.header_seen_ = true;
            status.line_ = 0;
        }

        Location loc(analyser.get_location(decl));
        bool top = analyser.is_toplevel(loc.file());
        if ((!top && status.header_seen_) ||
            (top && status.line_ < loc.line())) {
            status.check_ = false;
        }
        else if (((!top && !status.header_seen_) || loc.line() < status.line_)
                 && builtin != loc.file() && command_line != loc.file()
                 && (llvm::dyn_cast<NamedDecl>(decl) == 0
                     || utils::end(id_names)
                          == std::find(utils::begin(id_names),
                                       utils::end(id_names),
                                       llvm::dyn_cast<NamedDecl>(decl)
                                                          ->getNameAsString()))
                 && !analyser.is_main())
        {
            analyser.report(decl, check_name, "TR09",
                            "Declarations precede component header",
                            true);
            status.check_ = false;
        }
    }
}

// -----------------------------------------------------------------------------

static void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
{
    observer.onInclude   += bind<Analyser&>(analyser, include_file);
    observer.onCloseFile += bind<Analyser&>(analyser, close_file);
}

// -----------------------------------------------------------------------------

static RegisterCheck register_observer(check_name, &subscribe);
static RegisterCheck register_check(check_name, &declaration);

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
