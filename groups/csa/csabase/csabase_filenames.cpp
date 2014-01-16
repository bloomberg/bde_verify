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

namespace
{

llvm::StringRef subdir(llvm::StringRef path, llvm::StringRef dir)
    // Return the prefix of the specified 'path' whose final segement is the
    // specified 'dir', or 'path' if no such prefix exists.  The returned value
    // ends with a directory separator.
{
    size_t n = path.rfind(dir);
    while (n != 0 && n != path.npos) {
        if ((path.size() > n + dir.size() &&
             !llvm::sys::path::is_separator(path[n + dir.size()])) ||
            !llvm::sys::path::is_separator(path[n - 1])) {
            n = path.slice(0, n - 1).rfind(dir);
        } else {
            path = path.slice(0, n + dir.size() + 1);
            break;
        }
    }
    return path;
}

}

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

    size_t under = component_.find('_');
    package_ = component_.slice(0, under);
    group_   = package_.slice(0, 3);
    if (under == 1) {
        under = component_.find('_', under + 1);
        package_ = component_.slice(0, under);
        if (package_[0] != 'a') {
            group_ = package_.slice(0, 5);
        }
    }
    pkgdir_ = subdir(directory_, package_);
    grpdir_ = subdir(pkgdir_, group_);
#if 0
    llvm::errs() << __FUNCTION__ << " " << __LINE__ << "\n"
                 << " component " << component_ << "\n"
                 << " directory " << directory_ << "\n"
                 << " extension " << extension_ << "\n"
                 << " extra     " << extra_     << "\n"
                 << " full      " << full_      << "\n"
                 << " group     " << group_     << "\n"
                 << " grpdir    " << grpdir_    << "\n"
                 << " name      " << name_      << "\n"
                 << " package   " << package_   << "\n"
                 << " pkgdir    " << pkgdir_    << "\n"
                 << " prefix    " << prefix_    << "\n"
                 ;
#endif
}
