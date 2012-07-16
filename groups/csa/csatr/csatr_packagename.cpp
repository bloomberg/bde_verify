// -*-c++-*- groups/csa/csatr/csatr_packagename.cpp 
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
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

static bool not_packagechar(unsigned char c)
{
    return !(isalnum(c) && (isdigit(c) || islower(c)));
}

// -----------------------------------------------------------------------------

namespace
{
    struct on_open
    {
        on_open(cool::csabase::Analyser& analyser): d_analyser(&analyser) {}
        void operator()(clang::SourceLocation where, std::string const&, std::string const& name) const
        {
            ::packagename& attachment(this->d_analyser->attachment< ::packagename>());
            if (!attachment.d_done && name == this->d_analyser->toplevel()) {
                cool::csabase::Analyser& analyser(*this->d_analyser);

                std::string::size_type slash(name.rfind('/'));
                slash = slash == name.npos? 0: slash + 1;
                std::string directory(name.substr(0, slash == name.npos? 0: slash));
                std::string::size_type period(name.find('.', slash));
                std::string component(slash == name.npos? 0: name.substr(slash, period == name.npos? name.npos: (period - slash)));
                std::string::size_type underscore(component.rfind('_'));
                if (underscore == component.npos) {
                    analyser.report(where, ::check_name, "TR02: component file name '%0' doesn't contain an underscore", true)
                        << component;
                }
                std::string package(component.substr(0, underscore));
                if (!(3 < package.size() && package.size() < 8)) {
                    analyser.report(where, ::check_name,
                                    "TR02: package names must consist of 1 to 4 characters preceded by the group name: '%0'", true)
                        << package;
                }
                if (std::find_if(package.begin(), package.end(), &::not_packagechar) != package.end()
                    || isdigit(static_cast<unsigned char>(package[3]))) {
                    analyser.report(where, ::check_name,
                                    "TR02: page names most consist of lower case alphanumeric characters only: '%0'", true)
                        << package;
                }
                std::string path(directory + ".");
                
                struct stat direct;
                if (!::stat(path.c_str(), &direct)) {
                    std::string expect(name.substr(0, slash) + "../" + package);
                    struct stat indirect;
                    if (::stat(expect.c_str(), &indirect) || direct.st_ino != indirect.st_ino) {
                        analyser.report(where, ::check_name,
                                        "TR02: component '%0' doesn't seems to be in package '%1'",
                                        true)
                            << component
                            << package
                            << expect;
                    }
                    attachment.d_done = true;
                }
            }
        }
        cool::csabase::Analyser* d_analyser;
    };
}

static void
subscribe(cool::csabase::Analyser& analyser, cool::csabase::Visitor&, cool::csabase::PPObserver& observer)
{
    observer.onOpenFile  += ::on_open(analyser);
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_observer(check_name, &::subscribe);
