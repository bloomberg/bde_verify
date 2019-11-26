// csatr_files.cpp                                                    -*-C++-*-

#include <clang/Basic/SourceLocation.h>
#include <csabase_analyser.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_filenames.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Path.h>
#include <sys/stat.h>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <string>

namespace csabase { class Visitor; }

using namespace csabase;
using namespace clang;

// -----------------------------------------------------------------------------

static std::string const check_name("files");

// -----------------------------------------------------------------------------

namespace
{
    struct on_files_open
    {
        on_files_open(Analyser& analyser) : d_analyser(analyser)
        {
        }

        void operator()(SourceLocation where,
                        std::string const&,
                        std::string const& name) const
        {
            FileName fn(name);
            if (!fn.name().startswith("m_") && fn.extra() != ".m" &&
                d_analyser.is_toplevel(name)) {
                struct stat buffer;
                std::string component = fn.component().str();
                std::string prefix = fn.directory().str() + component;
                std::string pkg_prefix = fn.pkgdir().str() + component;
                std::string test_prefix =
                    fn.directory().str() + "test" +
                    llvm::sys::path::get_separator().str() + component;
                if (stat((    prefix + ".h").c_str(), &buffer) &&
                    stat((pkg_prefix + ".h").c_str(), &buffer)) {
                    d_analyser.report(where, check_name, "FI01",
                            "Header file '%0.h' not accessible", true)
                        << component;
                }
                if (stat((     prefix + ".t.cpp").c_str(), &buffer) &&
                    stat(( pkg_prefix + ".t.cpp").c_str(), &buffer) &&
                    stat((test_prefix + ".t.cpp").c_str(), &buffer) &&
                    stat((     prefix + ".g.cpp").c_str(), &buffer) &&
                    stat(( pkg_prefix + ".g.cpp").c_str(), &buffer) &&
                    stat((test_prefix + ".g.cpp").c_str(), &buffer)) {
                    d_analyser.report(where, check_name, "FI02",
                            "Test file '%0.t.cpp' not accessible", true)
                        << component;
                }
            }
        }

        Analyser& d_analyser;
    };
}

static void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
{
    observer.onOpenFile  += on_files_open(analyser);
}

// -----------------------------------------------------------------------------

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
