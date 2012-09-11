// -*-c++-*- framework/diagnostic_builder.hpp 
// -----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(FRAMEWORK_DIAGNOSTIC_BUILDER_HPP)
#define FRAMEWORK_DIAGNOSTIC_BUILDER_HPP 1
#ident "$Id: diagnostic_builder.hpp 141 2011-09-29 18:59:08Z kuehl $"

#if defined(COOL_CXX2011)
#  include "cool/unique_ptr.hpp"
#else
#  include <memory>
#endif
#include "clang/Basic/Diagnostic.h"

// -----------------------------------------------------------------------------

namespace cool
{
    class diagnostic_builder
    {
    public:
        diagnostic_builder():
            builder_()
        {
        }
        explicit diagnostic_builder(clang::DiagnosticBuilder builder):
            builder_(new clang::DiagnosticBuilder(builder))
        {
        }
#if defined(COOL_CXX2011)
        diagnostic_builder(diagnostic_builder&& other):
            builder_(static_cast<cool::unique_ptr<clang::DiagnosticBuilder>&&>(other.builder_))
#else
        diagnostic_builder(diagnostic_builder const& other):
            builder_(other.builder_.release())
#endif
        {
        }

        diagnostic_builder& operator<< (long argument)
        {
            if (this->builder_.get())
            {
                *this->builder_ << int(argument);
            }
            return *this;
        }
        template <typename T>
        diagnostic_builder& operator<< (T const& argument)
        {
            if (this->builder_.get())
            {
                *this->builder_ << argument;
            }
            return *this;
        }
    private:
#if defined(COOL_CXX2011)
        cool::unique_ptr<clang::DiagnosticBuilder> builder_;
#else
        mutable std::auto_ptr<clang::DiagnosticBuilder> builder_;
#endif
    };
}

// -----------------------------------------------------------------------------

#endif /* FRAMEWORK_DIAGNOSTIC_BUILDER_HPP */
