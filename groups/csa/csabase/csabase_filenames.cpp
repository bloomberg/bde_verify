// csabase_filenames.cpp                                              -*-C++-*-

#include <csabase_filenames.h>
#include <csabase_debug.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <stddef.h>

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

std::map<std::string, csabase::FileName> csabase::FileName::s_file_names_;

void csabase::FileName::reset(llvm::StringRef sr)
{
    auto i = s_file_names_.find(sr);
    if (i != s_file_names_.end()) {
        *this = i->second;
        return;                                                       // RETURN
    }
    if (sr.startswith("<")) {  // Not a real file
        name_ = full_ = sr;
        tag_ = "<";
    }
    else {
        char buf[4000];
#ifdef _MSC_VER
#define realpath(path, buf) _fullpath(buf, path, sizeof(buf))
#endif
        full_ = sr;
        const char *pc = realpath(full_.c_str(), buf);
        if (pc) {
            full_ = pc;
        }

        llvm::StringRef rfull = full_;

        directory_ = rfull;
        while (directory_.size() > 0 &&
               !llvm::sys::path::is_separator(directory_.back())) {
            directory_ = directory_.drop_back(1);
        }

        name_ = rfull.drop_front(directory_.size());
        extension_ = name_.slice(name_.rfind('.'), name_.npos);
        prefix_ = rfull.drop_back(extension_.size());
        extra_ = name_.slice(name_.find('.'), name_.rfind('.'));
        component_ = name_.slice(0, name_.find('.'));

        size_t under = component_.find('_');
        size_t under2 = component_.rfind('_');
        if (under == 1 && under2 != component_.npos) {
            // Typical non-library component file, e.g.,
            // "/some/path/applications/m_NAME/m_NAME_COMP.cpp".
            package_ = component_.slice(0, under2);
            pkgdir_ = subdir(directory_, package_);
            tag_ = component_.slice(0, 1);
        }
        else if (under != component_.npos) {
            // Typical library component file, e.g.,
            // "/some/path/groups/GRP/GRPPKG/GRPPKG_COMP.cpp".
            package_ = component_.slice(0, under);
            group_   = package_.slice(0, 3);
            pkgdir_ = subdir(directory_, package_);
            grpdir_ = subdir(pkgdir_, group_);
        }
        else {
            // Something else - don't look for package structure.
        }
    }
    s_file_names_[sr] = *this;

#if 0
    ERRS() << "component   " << component_; ERNL();
    ERRS() << "directory   " << directory_; ERNL();
    ERRS() << "extension   " << extension_; ERNL();
    ERRS() << "extra       " << extra_    ; ERNL();
    ERRS() << "full        " << full_     ; ERNL();
    ERRS() << "group       " << group_    ; ERNL();
    ERRS() << "grpdir      " << grpdir_   ; ERNL();
    ERRS() << "name        " << name_     ; ERNL();
    ERRS() << "package     " << package_  ; ERNL();
    ERRS() << "pkgdir      " << pkgdir_   ; ERNL();
    ERRS() << "prefix      " << prefix_   ; ERNL();
    ERRS() << "tag         " << tag_      ; ERNL();
    ERNL();
#endif
}

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
