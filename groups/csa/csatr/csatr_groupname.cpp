// csatr_groupname.cpp                                                -*-C++-*-

#include <clang/Basic/SourceLocation.h>
#include <csabase_analyser.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_filenames.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <ctype.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Path.h>
#include <sys/stat.h>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <string>

namespace csabase { class Visitor; }

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("groupname");

// ----------------------------------------------------------------------------

namespace
{
    struct groupname
    {
        groupname() : d_done(false)
        {
        }

        bool        d_done;
    };
}

// ----------------------------------------------------------------------------

static bool groupchar(unsigned char c)
{
    return isalpha(c) && islower(c);
}

// ----------------------------------------------------------------------------

namespace
{
}

struct on_group_open
{
    on_group_open(Analyser& analyser) : d_analyser(&analyser)
    {
    }

    void operator()(SourceLocation where,
                    std::string const&,
                    std::string const& name) const
    {
        Analyser& analyser(*d_analyser);
        groupname& attachment(analyser.attachment<groupname>());
        FileName fn(name);
        if (!attachment.d_done && analyser.is_toplevel(name)) {
            attachment.d_done = true;

            std::string const& group(analyser.group());

            bool traditional = group.size() == 3 &&
                               groupchar(group[0]) &&
                               groupchar(group[1]) &&
                               groupchar(group[2]);
            bool standalone  = fn.tag().size() != 0;

            if (traditional) {
                llvm::SmallVector<char, 1024> vpath(fn.pkgdir().begin(),
                                                    fn.pkgdir().end());
                llvm::sys::path::append(vpath, "..");
                std::string packagedir(vpath.begin(), vpath.end());
                struct stat direct;
                if (stat(packagedir.c_str(), &direct) == 0) {
                    llvm::sys::path::append(vpath, "..", group);
                    std::string groupdir(vpath.begin(), vpath.end());
                    struct stat indirect;
                    if (stat(groupdir.c_str(), &indirect) ||
                        direct.st_ino != indirect.st_ino) {
                        analyser.report(where, check_name, "GN02",
                                "Component '%0' doesn't seem to be in "
                                "package group '%1'", true)
                            << analyser.component()
                            << group;
                    }
                }
            }
            else if (!standalone) {
                analyser.report(where, check_name, "GN01",
                        "Group names must consist of three lower-case "
                        "letters: '%0'", true)
                    << group;
            }
        }
    }

    Analyser* d_analyser;
};

static void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
{
    observer.onOpenFile  += on_group_open(analyser);
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
