// -*-c++-*- groups/csa/csatr/csatr_files.cpp 
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyser.h>
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
        on_files_open(cool::csabase::Analyser& analyser): d_analyser(&analyser) {}
        void operator()(clang::SourceLocation where, std::string const&, std::string const& name) const
        {
            cool::csabase::Analyser& analyser(*this->d_analyser);
            if (name == this->d_analyser->toplevel()) {
                std::string::size_type slash(name.rfind('/'));
                std::string            directory(slash == name.npos? "./": name.substr(0, slash + 1));
                std::string            file(slash == name.npos? name: name.substr(slash + 1));
                std::string::size_type period(file.find('.'));
                std::string            component(file.substr(0, period));
                std::string            suffix(file.substr(period == file.npos? file.size(): period));
                if (suffix == ".cpp") {
                    struct stat buffer;
                    std::string header(directory + component + ".h");
                    std::string test(directory + component + ".t.cpp");
                    if (stat(header.c_str(), &buffer)) {
                        analyser.report(where, check_name, "TR03: header file '%0' not accessible", true)
                            << header;
                    }
                    if (stat(test.c_str(), &buffer)) {
                        analyser.report(where, check_name, "TR03: test file '%0' not accessible", true)
                            << test;
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
    observer.onOpenFile  += on_files_open(analyser);
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_observer(check_name, &subscribe);
