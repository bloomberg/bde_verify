// -*-c++-*- groups/csa/csatr/csatr_groupname.cpp 
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_filenames.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <llvm/Support/Path.h>
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

static bool groupchar(unsigned char c)
{
    return isalpha(c) && islower(c);
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
        cool::csabase::Analyser& analyser(*d_analyser);
        groupname& attachment(analyser.attachment<groupname>());
        cool::csabase::FileName fn(name);
        if (   !attachment.d_done
            && name == analyser.toplevel()
            && fn.name().find("m_") != 0) {
            attachment.d_done = true;
            std::string const& group(analyser.group());
            if (!((group.size() == 3 &&
                  groupchar(group[0]) &&
                  groupchar(group[1]) &&
                  groupchar(group[2])) ||
                 (group.size() == 5 &&
                  groupchar(group[0]) &&
                  group[1] == '_' &&
                  groupchar(group[2]) &&
                  groupchar(group[3]) &&
                  groupchar(group[4])))) {
                analyser.report(where, check_name, "GN01",
                        "Group names must consist of three lower-case letters "
                        "possibly prefixed by a single lowercase letter and "
                        "underscore: '%0'", true)
                    << group;
            }

            llvm::SmallVector<char, 1024> vpath(fn.pkgdir().begin(),
                                                fn.pkgdir().end());
            llvm::sys::path::append(vpath, "..");
            std::string packagedir(vpath.begin(), vpath.end());
            struct stat direct;
            if (stat(packagedir.c_str(), &direct) == 0) {
                llvm::sys::path::append(vpath, "..", group);
                std::string groupdir(vpath.begin(), vpath.end());
                struct stat indirect;
                if (stat(groupdir.c_str(), &indirect)
                    || direct.st_ino != indirect.st_ino) {
                    analyser.report(where, check_name, "GN02",
                            "Component '%0' doesn't seem to be in package "
                            "group '%1'", true)
                        << analyser.component()
                        << group;
                }
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

static cool::csabase::RegisterCheck register_observer(check_name, &subscribe);
