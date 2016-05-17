// csabase_debug.h                                                    -*-C++-*-

#ifndef INCLUDED_CSABASE_DEBUG
#define INCLUDED_CSABASE_DEBUG

#include <llvm/Support/raw_ostream.h>
#include <stdio.h>

// -----------------------------------------------------------------------------

#define ERRS() (llvm::errs() << __FUNCTION__ << " " << __LINE__ << " ")
#define ERNL() (llvm::errs() << "\n")

namespace csabase
{

class Debug
{
public:
    static void set_debug(bool);
    static bool get_debug();

    Debug(char const*, bool nest = true);
    ~Debug();
    template <typename T> llvm::raw_ostream& operator<< (T const& value) const;

private:
    Debug(Debug const&);
    void operator= (Debug const&);
    llvm::raw_ostream& indent() const;

    char const*  message_;
    unsigned int nest_;
};

// -----------------------------------------------------------------------------

template <typename T>
inline
llvm::raw_ostream& Debug::operator<<(T const& value) const
{
    return indent() << value;
}

}

// -----------------------------------------------------------------------------

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
