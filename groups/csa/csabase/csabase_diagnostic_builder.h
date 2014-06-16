// -*-c++-*- framework/diagnostic_builder.hpp 
// -----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(FRAMEWORK_DIAGNOSTIC_BUILDER_HPP)
#define FRAMEWORK_DIAGNOSTIC_BUILDER_HPP 1
#ident "$Id: diagnostic_builder.hpp 141 2011-09-29 18:59:08Z kuehl $"

#include "clang/Basic/Diagnostic.h"

// -----------------------------------------------------------------------------

namespace csabase
{
class diagnostic_builder
{
  public:
    diagnostic_builder()
        : empty_(true), builder_(clang::DiagnosticBuilder::getEmpty())
    {
    }

    explicit diagnostic_builder(clang::DiagnosticBuilder builder)
        : empty_(false), builder_(builder)
    {
    }

    diagnostic_builder& operator<<(long long argument)
    {
        return *this << static_cast<int>(argument);
    }

    diagnostic_builder& operator<<(long argument)
    {
        return *this << static_cast<int>(argument);
    }

    template <typename T>
    diagnostic_builder& operator<<(T const& argument)
    {
        if (!empty_) {
            builder_ << argument;
        }
        return *this;
    }

  private:
    bool empty_;
    clang::DiagnosticBuilder builder_;
};
}

// -----------------------------------------------------------------------------

#endif /* FRAMEWORK_DIAGNOSTIC_BUILDER_HPP */
