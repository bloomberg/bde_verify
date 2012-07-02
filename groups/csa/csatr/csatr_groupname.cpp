// -*-c++-*- groups/csa/csatr/csatr_groupname.cpp 
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

static std::string const check_name("groupname");

// -----------------------------------------------------------------------------

namespace
{
    struct groupname
    {
        groupname():
            d_done(false)
        {
        }
        bool        d_done;
    };
}

// -----------------------------------------------------------------------------

static bool not_groupchar(unsigned char c)
{
    return !(isalpha(c) && islower(c));
}

// -----------------------------------------------------------------------------

namespace
{
    struct on_group_open
    {
        on_group_open(cool::csabase::Analyser& analyser): d_analyser(&analyser) {}
        void operator()(clang::SourceLocation where, std::string const&, std::string const& name) const
        {
            ::groupname& attachment(this->d_analyser->attachment< ::groupname>());
            //-dk:TODO Restrict to check to components in a package group, i.e. don't
            //         check this for components in stand-alone package.
            if (!attachment.d_done && name == this->d_analyser->toplevel()) {
                cool::csabase::Analyser& analyser(*this->d_analyser);

                std::string::size_type slash(name.rfind('/'));
                slash = slash == name.npos? 0: slash + 1;
                std::string::size_type period(name.find('.', slash));
                std::string component(slash == name.npos? 0: name.substr(slash, period == name.npos? name.npos: (period - slash)));
                std::string directory(name.substr(0, slash == name.npos? 0: slash));
                std::string group(component.size() < 3? component: component.substr(0, 3));
                if (group.size() != 3) {
                    analyser.report(where, ::check_name,
                                    "TR01: group names most consist of 3 characters: '%0'", true)
                        << group;
                }
                else if (group[1] != '_') {
                    if (std::find_if(group.begin(), group.end(), &::not_groupchar)
                         != group.end()) {
                        analyser.report(where, ::check_name,
                                        "TR01: group names most consist of lower case alphabetic characters only: '%0'", true)
                            << group;
                    }

                    std::string path(directory + "..");
                    struct stat direct;
                    if (!::stat(path.c_str(), &direct)) {
                        std::string expect(name.substr(0, slash) + "../../" + group);
                        struct stat indirect;
                        if (::stat(expect.c_str(), &indirect) || direct.st_ino != indirect.st_ino) {
                            analyser.report(where, ::check_name,
                                            "TR01: component '%0' doesn't seems to be in package group '%1'",
                                            true)
                                << component
                                << group
                                << expect;
                        }
                    }
                }
                attachment.d_done = true;
            }
        }
        cool::csabase::Analyser* d_analyser;
    };
}

static void
subscribe(cool::csabase::Analyser& analyser, cool::csabase::Visitor&, cool::csabase::PPObserver& observer)
{
    observer.onOpenFile  += ::on_group_open(analyser);
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_observer(check_name, &::subscribe);
