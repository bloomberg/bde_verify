// csatr_includeguard.cpp                                             -*-C++-*-

#include <clang/Basic/IdentifierTable.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Lex/Token.h>
#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Regex.h>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <string>

namespace clang { class MacroDirective; }
namespace csabase { class Visitor; }

using namespace clang;
using namespace csabase;

// ----------------------------------------------------------------------------

static std::string const check_name("include-guard");

// -----------------------------------------------------------------------------

namespace
{

struct data
    // Data needed for include guard checking inside headers.
{
    data();
        // Create an object of this type.

    bool        d_test;     // true after guard is tested
    bool        d_define;   // trur after guard is defined
};

data::data()
: d_test(false)
, d_define(false)
{
}

struct report : Report<data>
    // Detect incorrect guard use in headers.
{
    INHERIT_REPORT_CTOR(report, Report, data);

    std::string guard();
        // Return the expected include guard.

    void operator()(SourceLocation, SourceRange range);
        // Process '#if' at the specified 'range'.

    void operator()(SourceLocation where, Token const& token);
        // Process '#ifndef' of the specified 'token' at the specified location
        // 'where'.

    void operator()(Token const& token, MacroDirective const*);
        // Process the '#define' of the specified 'token'.

    void operator()(SourceLocation location,
                    std::string const&,
                    std::string const& filename);
        // Process close of the specified 'file' at the specified 'location'.
};

std::string report::guard()
{
    return "INCLUDED_" + llvm::StringRef(a.component()).upper();
}

void report::operator()(SourceLocation, SourceRange range)
{
    if (a.is_component_header(range.getBegin()) && !d.d_test) {
        llvm::StringRef value = a.get_source(range);
        llvm::Regex re("^ *! *defined *[(]? *" + guard() + " *[)]? *\r*$");
        if (!re.match(value)) {
            a.report(range.getBegin(), check_name, "TR14",
                              "Wrong include guard (expected '!defined(%0)')")
                << guard();
        }
        d.d_test = true;
    }
}

void report::operator()(SourceLocation where, Token const& token)
{
    if (a.is_component_header(token.getLocation()) &&
        !d.d_test) {
        if (IdentifierInfo const* id = token.getIdentifierInfo()) {
            if (id->getNameStart() != guard()) {
                a.report(token.getLocation(), check_name, "TR14",
                                  "Wrong name for include guard "
                                  "(expected '%0')")
                    << guard();
            }
            d.d_test = true;
        }
    }
}

void report::operator()(Token const& token, MacroDirective const*)
{
    if (a.is_component_header(token.getLocation())
        && !d.d_define
        && token.getIdentifierInfo()
        && token.getIdentifierInfo()->getNameStart() == guard()
        ) {
        d.d_define = true;
    }
}

void report::operator()(SourceLocation location,
                        std::string const&,
                        std::string const& filename)
{
    if (a.is_component_header(filename) &&
        !(d.d_test && d.d_define)) {
        a.report(location, check_name, "TR14",
                 d.d_test
                 ? "Missing define for include guard %0"
                 : d.d_define
                 ? "Missing test for include guard %0"
                 : "Missing include guard %0")
            << guard();
    }
}

// -----------------------------------------------------------------------------

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
{
    observer.onIfndef       += report(analyser);
    observer.onIf           += report(analyser);
    observer.onMacroDefined += report(analyser);
    observer.onCloseFile    += report(analyser);
}

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
