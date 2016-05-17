// csabase_format.h                                                   -*-C++-*-

#ifndef INCLUDED_CSABASE_FORMAT
#define INCLUDED_CSABASE_FORMAT

namespace clang { class DiagnosticBuilder; }
namespace llvm { class raw_ostream; }

// ----------------------------------------------------------------------------

namespace csabase
{
template <typename T>
class Formatter
{
public:
    Formatter(T const& value);
    void print(llvm::raw_ostream&) const;
    void print(clang::DiagnosticBuilder&) const;

private:
    T const& value_;
};

template <typename T>
inline
Formatter<T>::Formatter(T const& value)
: value_(value)
{
}

// -----------------------------------------------------------------------------

template <typename T>
inline
Formatter<T> format(T const& value)
{
    return Formatter<T>(value);
}

// -----------------------------------------------------------------------------

template <typename T>
inline
llvm::raw_ostream& operator<<(llvm::raw_ostream& out,
                              Formatter<T> const& value)
{
    value.print(out);
    return out;
}

template <typename T>
inline
clang::DiagnosticBuilder& operator<<(clang::DiagnosticBuilder& builder,
                                     Formatter<T> const& value)
{
    value.print(builder);
    return builder;
}
}

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
