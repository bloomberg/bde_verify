// csabase_diagnostic_builder.h                                       -*-C++-*-

#ifndef FRAMEWORK_DIAGNOSTIC_BUILDER_HPP
#define FRAMEWORK_DIAGNOSTIC_BUILDER_HPP

#include <csabase_debug.h>
#include <clang/Basic/Diagnostic.h>

// -----------------------------------------------------------------------------

namespace csabase
{
class diagnostic_builder
{
  public:
    diagnostic_builder();
    diagnostic_builder(clang::DiagnosticBuilder other, bool always = true);
    diagnostic_builder& operator<<(long long argument);
    diagnostic_builder& operator<<(long argument);
    diagnostic_builder& operator<<(const llvm::StringRef &ref);

    template <typename T>
    diagnostic_builder& operator<<(T const& argument);
    explicit operator bool() const;

  private:
    bool empty_;
    clang::DiagnosticBuilder builder_;
};

inline
diagnostic_builder::diagnostic_builder()
: empty_(true), builder_(clang::DiagnosticBuilder::getEmpty())
{
}

inline diagnostic_builder::diagnostic_builder(clang::DiagnosticBuilder other,
                                              bool                     always)
: empty_(false), builder_(other)
{
    if (always) {
        builder_.setForceEmit();
    }
}

inline
diagnostic_builder& diagnostic_builder::operator<<(long long argument)
{
    return *this << static_cast<int>(argument);
}

inline
diagnostic_builder& diagnostic_builder::operator<<(const llvm::StringRef &ref)
{
    return *this << ref.str();
}

inline
diagnostic_builder& diagnostic_builder::operator<<(long argument)
{
    return *this << static_cast<int>(argument);
}

template <typename T>
inline
diagnostic_builder& diagnostic_builder::operator<<(T const& argument)
{
    if (!empty_) {
        builder_ << argument;
    }
    return *this;
}

inline
diagnostic_builder::operator bool() const
{
    return !empty_;
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
