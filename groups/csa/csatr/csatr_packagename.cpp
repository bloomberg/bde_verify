// -*-c++-*- groups/csa/csatr/csatr_packagename.cpp 
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_filenames.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <llvm/Support/Path.h>
#include <ctype.h>
#include <sys/stat.h>
#ident "$Id$"

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
        on_open(cool::csabase::Analyser& analyser): d_analyser(&analyser) {}
        void operator()(clang::SourceLocation where, std::string const&, std::string const& name) const
        {
            packagename& attachment(d_analyser->attachment<packagename>());
            if (!attachment.d_done && name == this->d_analyser->toplevel()) {
                attachment.d_done = true;
                cool::csabase::Analyser& analyser(*this->d_analyser);
                cool::csabase::FileName fn(name);

                if (fn.component().count('_') != fn.package().count('_') + 1) {
                    analyser.report(where, check_name, "TR02: "
                                    "component file name '%0' must consist of "
                                    "package %1 followed by underscore and "
                                    "name with no underscores", true)
                        << fn.component()
                        << fn.package();
                        return;
                }

                llvm::StringRef srpackage = fn.package();
                llvm::StringRef srgroup = fn.group();
                int pkgsize = srpackage.size() - srgroup.size();
                if (pkgsize < 1 || pkgsize > 4) {
                    analyser.report(where, check_name, "TR02: "
                            "package name %0 must consist of 1-4 characters "
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
                    analyser.report(where, check_name, "TR02: "
                            "package and group names must consist of lower "
                            "case alphanumeric characters, start with a lower "
                            "case letter, and be separated by underscores: "
                            "'%0'", true)
                        << srpackage;
                }

                struct stat direct;
                if (stat(fn.directory().str().c_str(), &direct) == 0) {
                    llvm::SmallVector<char, 1024> svpath(fn.directory().begin(),
                                                         fn.directory().end());
                    llvm::sys::path::append(svpath, "..", srpackage);
                    std::string expect(svpath.begin(), svpath.end());
                    struct stat indirect;
                    if (stat(expect.c_str(), &indirect) ||
                        direct.st_ino != indirect.st_ino) {
                        analyser.report(where, check_name, "TR02: "
                                "component '%0' doesn't seem to be in package "
                                "'%1'", true)
                            << fn.component()
                            << srpackage
                            << expect;
                    }
                }
            }
        }

        cool::csabase::Analyser* d_analyser;
    };
}

static void
subscribe(cool::csabase::Analyser& analyser, cool::csabase::Visitor&, cool::csabase::PPObserver& observer)
{
    observer.onOpenFile  += on_open(analyser);
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_observer(check_name, &subscribe);
