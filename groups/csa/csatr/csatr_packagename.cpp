// csatr_packagename.cpp                                              -*-C++-*-

#include <clang/Basic/SourceLocation.h>
#include <csabase_analyser.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_filenames.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <ctype.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/Path.h>
#include <stddef.h>
#include <sys/stat.h>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <string>

namespace csabase { class Visitor; }

using namespace csabase;
using namespace clang;

// -----------------------------------------------------------------------------

static std::string const check_name("packagename");

// -----------------------------------------------------------------------------

namespace
{
    struct packagename
    {
        packagename():
            d_done(false)
        {
        }
        bool        d_done;
    };
}

// -----------------------------------------------------------------------------

namespace
{
    struct on_open
    {
        on_open(Analyser& analyser) : d_analyser(&analyser)
        {
        }

        void operator()(SourceLocation where,
                        std::string const&,
                        std::string const& name) const
        {
            packagename& attachment(d_analyser->attachment<packagename>());
            FileName fn(name);

            if (!attachment.d_done && d_analyser->is_toplevel(name)) {
                attachment.d_done = true;
                Analyser& analyser(*d_analyser);

                bool standalone = fn.tag().size() != 0;

                if (standalone) {
                    if (fn.name().find('_', 2) == fn.name().npos) {
                        analyser.report(where, check_name, "PN01",
                                    "Component file name '%0' in standalone "
                                    "component contains only one underscore",
                                    true)
                            << fn.name();
                    }
                    return;                                           // RETURN
                }

                if (fn.component().count('_') != (fn.package() == "bslfwd" ?
                            2 : fn.package().count('_') + 1)) {
                    std::string super =
                        (fn.pkgdir() +
                         fn.component().substr(0, fn.component().rfind('_')) +
                         ".h"
                        ).str();
                    struct stat buffer;
                    if (stat(super.c_str(), &buffer)) {
                        analyser.report(where, check_name, "PN02",
                                    "Cannot find component header file '%0' "
                                    "to which this component '%1' is "
                                    "subordinate")
                            << super
                            << fn.component();
                        return;                                       // RETURN
                    }
                }

                llvm::StringRef srpackage = fn.package();
                llvm::StringRef srgroup = fn.group();
                int pkgsize = srpackage.size() - srgroup.size();
                if (srpackage != srgroup && (pkgsize < 1 || pkgsize > 4)) {
                    analyser.report(where, check_name, "PN03",
                            "Package name %0 must consist of 1-4 characters "
                            "preceded by the group name: '%0'", true)
                        << srgroup.str();
                }

                bool digit_ok = false;
                bool under_ok = false;
                bool bad = false;
                for (size_t i = 0; !bad && i < srpackage.size(); ++i) {
                    unsigned char c = srpackage[i];
                    if (islower(c)) {
                        digit_ok = true;
                        under_ok = true;
                    }
                    else if ('_' == c) {
                        bad = !under_ok;
                        digit_ok = false;
                        under_ok = false;
                    }
                    else if (isdigit(c)) {
                        bad = !digit_ok;
                    }
                    else {
                        bad = true;
                    }
                }
                if (bad) {
                    analyser.report(where, check_name, "PN04",
                            "Package and group names must consist of lower "
                            "case alphanumeric characters, start with a lower "
                            "case letter, and be separated by underscores: "
                            "'%0'", true)
                        << srpackage;
                }

                llvm::SmallVector<char, 1024> svpath(fn.pkgdir().begin(),
                                                     fn.pkgdir().end());
                llvm::sys::path::append(svpath, ".");
                std::string s(svpath.begin(), svpath.end());
                struct stat direct;
                if (stat(s.c_str(), &direct) == 0) {
                    llvm::sys::path::append(svpath, "..", srpackage);
                    std::string expect(svpath.begin(), svpath.end());
                    struct stat indirect;
                    if (stat(expect.c_str(), &indirect) != 0 ||
                        direct.st_ino != indirect.st_ino) {
                        analyser.report(where, check_name, "PN05",
                                "Component '%0' doesn't seem to be in package "
                                "'%1'", true)
                            << fn.component()
                            << srpackage;
                    }
                }
            }
        }

        Analyser* d_analyser;
    };
}

static void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
{
    observer.onOpenFile += on_open(analyser);
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
