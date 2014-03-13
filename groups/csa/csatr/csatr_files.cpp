// -*-c++-*- groups/csa/csatr/csatr_files.cpp 
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_filenames.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <sys/stat.h>
#ident "$Id$"

// -----------------------------------------------------------------------------

static std::string const check_name("files");

// -----------------------------------------------------------------------------

namespace
{
    struct on_files_open
    {
        on_files_open(bde_verify::csabase::Analyser& analyser)
            : d_analyser(analyser) {}

        void operator()(clang::SourceLocation where,
                        std::string const&,
                        std::string const& name) const
        {
            bde_verify::csabase::FileName fn(name);
            if (fn.name().find("m_") != 0 && name == d_analyser.toplevel()) {
                bde_verify::csabase::FileName fn(name);
                struct stat buffer;
                std::string prefix =
                    fn.directory().str() + fn.component().str();
                std::string pkg_prefix =
                    fn.pkgdir().str()   + fn.component().str();
                if (stat((    prefix + ".h").c_str(), &buffer) &&
                    stat((pkg_prefix + ".h").c_str(), &buffer)) {
                    d_analyser.report(where, check_name, "FI01",
                            "Header file '%0' not accessible", true)
                        << (pkg_prefix + ".h");
                }
                if (stat((    prefix + ".t.cpp").c_str(), &buffer) &&
                    stat((pkg_prefix + ".t.cpp").c_str(), &buffer)) {
                    d_analyser.report(where, check_name, "FI02",
                            "Test file '%0' not accessible", true)
                        << (pkg_prefix + ".t.cpp");
                }
            }
        }

        bde_verify::csabase::Analyser& d_analyser;
    };
}

static void
subscribe(bde_verify::csabase::Analyser& analyser, bde_verify::csabase::Visitor&, bde_verify::csabase::PPObserver& observer)
{
    observer.onOpenFile  += on_files_open(analyser);
}

// -----------------------------------------------------------------------------

static bde_verify::csabase::RegisterCheck register_observer(check_name, &subscribe);
