// csabase_filenames.h                                                -*-C++-*-

#ifndef INCLUDED_CSABASE_FILENAMES
#define INCLUDED_CSABASE_FILENAMES

#include <llvm/ADT/StringRef.h>
#include <string>
#include <map>

// ----------------------------------------------------------------------------

namespace csabase
{
class FileName
{
    // This class facilitates dealing with the various pieces of file names.
    //
    // A library component file is expected to have a name like
    //     /initial/path/groups/GRP/GRPPKG/GRPPKG_COMP.t.cpp
    // and would be broken up as
    //     component   GRPPKG_COMP
    //     directory   /initial/path/groups/GRP/GRPPKG/
    //     extension   .cpp
    //     extra       .t
    //     full        /initial/path/groups/GRP/GRPPKG/GRPPKG_COMP.t.cpp
    //     group       GRP
    //     grpdir      /initial/path/groups/GRP/
    //     name        GRPPKG_COMP.t.cpp
    //     package     GRPPKG
    //     pkgdir      /initial/path/groups/GRP/GRPPKG/
    //     prefix      /initial/path/groups/GRP/GRPPKG/GRPPKG_COMP.t
    //     tag         (empty)
    //
    // An application component file is expected to have a name like
    //     /initial/path/applications/m_APPL/m_APPL_COMP.t.cpp
    // and would be broken up as
    //     component   m_APPL_COMP
    //     directory   /initial/path/applications/m_APPL/
    //     extension   .cpp
    //     extra       .t
    //     full        /initial/path/applications/m_APPL/m_APPL_COMP.t.cpp
    //     group       (empty)
    //     grpdir      (empty)
    //     name        m_APPL_COMP.t.cpp
    //     package     m_APPL
    //     pkgdir      /initial/path/applications/m_APPL/
    //     prefix      /initial/path/applications/m_APPL/m_APPL_COMP.t
    //     tag         m
    //
    // A service component file is expected to have a name like
    //     /initial/path/services/s_SRVC/s_SRVC_COMP.t.cpp
    // and is broken up like an application name.
    //
    // An adapter component file is expected to have a name like
    //     /initial/path/services/a_ADPT/a_ADPT_COMP.t.cpp
    // and is broken up like an application name.

public:
    FileName() { }
    FileName(llvm::StringRef sr) { reset(sr); }
    void reset(llvm::StringRef sr = llvm::StringRef());

    llvm::StringRef component() const { return component_; }
    llvm::StringRef directory() const { return directory_; }
    llvm::StringRef extension() const { return extension_; }
    llvm::StringRef extra()     const { return extra_;     }
    llvm::StringRef full()      const { return full_;      }
    llvm::StringRef group()     const { return group_;     }
    llvm::StringRef grpdir()    const { return grpdir_;    }
    llvm::StringRef name()      const { return name_;      }
    llvm::StringRef package()   const { return package_;   }
    llvm::StringRef pkgdir()    const { return pkgdir_;    }
    llvm::StringRef prefix()    const { return prefix_;    }
    llvm::StringRef tag()       const { return tag_;       }

private:
    llvm::StringRef component_;
    llvm::StringRef directory_;
    llvm::StringRef extension_;
    llvm::StringRef extra_;
    std::string     full_;
    llvm::StringRef group_;
    llvm::StringRef grpdir_;
    llvm::StringRef name_;
    llvm::StringRef package_;
    llvm::StringRef pkgdir_;
    llvm::StringRef prefix_;
    llvm::StringRef tag_;

    static std::map<std::string, FileName> s_file_names_;
};
}

//-----------------------------------------------------------------------------

#endif

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
