// -*-c++-*- groups/csa/csatr/csatr_groupname.cpp 
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <ctype.h>
#include <sys/stat.h>
#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("groupname");

// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------

static bool not_groupchar(unsigned char c)
{
    return !(isalpha(c) && islower(c));
}

// ----------------------------------------------------------------------------

namespace
{
    struct on_group_open;
}

struct on_group_open
{
    on_group_open(cool::csabase::Analyser& analyser)
        : d_analyser(&analyser)
    {
    }
    void operator()(clang::SourceLocation where,
                    std::string const&    ,
                    std::string const&    name) const
    {
        ::groupname& attachment(this->d_analyser->attachment< ::groupname>());
        if (!attachment.d_done
            && name == this->d_analyser->toplevel()
            && !this->d_analyser->group().empty()) {
            cool::csabase::Analyser& analyser(*this->d_analyser);
            std::string const& group(analyser.group());

            if (group.size() != 3) {
                analyser.report(where,
                                ::check_name,
                                "TR01: group names most consist of 3 "
                                "characters: '%0'",
                                true)
                    << group;
            }
            if (std::find_if(group.begin(), group.end(), &::not_groupchar)
                != group.end()) {
                analyser.report(where,
                                ::check_name,
                                "TR01: group names most consist of lower case "
                                "alphabetic characters only: '%0'",
                                true)
                    << group;
            }
            std::string path(analyser.directory() + "..");
                
            struct stat direct;
            if (!::stat(path.c_str(), &direct)) {
                std::string expect(analyser.directory() + "../../" + group);
                struct stat indirect;
                if (::stat(expect.c_str(), &indirect)
                    || direct.st_ino != indirect.st_ino) {
                    analyser.report(where,
                                    ::check_name,
                                    "TR01: component '%0' doesn't seems to be "
                                    "in package group '%1'",
                                    true)
                        << (analyser.package() + "_" + analyser.component())
                        << group
                        << expect;
                }
                attachment.d_done = true;
            }
        }
    }
    cool::csabase::Analyser* d_analyser;
};

static void
subscribe(cool::csabase::Analyser& analyser, cool::csabase::Visitor&, cool::csabase::PPObserver& observer)
{
    observer.onOpenFile  += ::on_group_open(analyser);
}

// ----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_observer(check_name, &::subscribe);
