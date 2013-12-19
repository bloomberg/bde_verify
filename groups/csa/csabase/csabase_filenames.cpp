// csabase_filenames.cpp                                              -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2013 Hyman Rosen (hrosen4@bloomberg.net)
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_filenames.h>
#ident "$Id$"

#include <llvm/Support/Debug.h>
#include <llvm/Support/Path.h>

// -----------------------------------------------------------------------------

void cool::csabase::FileName::reset(llvm::StringRef sr)
{
    full_ = sr;
    extension_ = sr.slice(sr.rfind('.'), sr.npos);
    prefix_ = sr.drop_back(extension_.size());
    directory_ = prefix_;
    while (directory_.size() > 0 &&
           !llvm::sys::path::is_separator(directory_.back())) {
        directory_ = directory_.drop_back(1);
    }
    name_ = sr.drop_front(directory_.size());
    extra_ = name_.slice(name_.find('.'), name_.rfind('.'));
    component_ = name_.slice(0, name_.find('.'));

    size_t under = component_.rfind('_');
    package_ = component_.slice(0, under);

    size_t grouplen = 3;
    if (component_.size() > 1 &&
        component_[1] == '_' &&
        component_[0] != 'a') {
        grouplen += 2;
    }
    group_ = package_.slice(0, grouplen);
#if 0
    llvm::errs() << __FUNCTION__ << " " << __LINE__ << "\n"
                 << " component " << component_ << "\n"
                 << " directory " << directory_ << "\n"
                 << " extension " << extension_ << "\n"
                 << " extra     " << extra_     << "\n"
                 << " full      " << full_      << "\n"
                 << " group     " << group_     << "\n"
                 << " name      " << name_      << "\n"
                 << " package   " << package_   << "\n"
                 << " prefix    " << prefix_    << "\n"
                 ;
#endif
}
