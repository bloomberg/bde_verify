// csabase_filenames.h
// -----------------------------------------------------------------------------
// Copyright 2013 Hyman Rosen (hrosen4@bloomberg.net)
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(INCLUDED_CSABASE_FILENAMES)
#define INCLUDED_CSABASE_FILENAMES 1
#ident "$Id$"

#include <llvm/ADT/StringRef.h>

// -----------------------------------------------------------------------------

namespace cool
{
    namespace csabase
    {
        class FileName {
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
        };
    }
}

// -----------------------------------------------------------------------------

#endif /* INCLUDED_FRAMEWORK_FORMAT */
