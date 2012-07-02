// -*-c++-*- framework/cast_ptr.hpp 
// -----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(FRAMEWORK_CAST_PTR_HPP)
#define FRAMEWORK_CAST_PTR_HPP 1
#ident "$Id: cast_ptr.hpp 141 2011-09-29 18:59:08Z kuehl $"

// -----------------------------------------------------------------------------

namespace cool
{
    template <typename T> class cast_ptr;
}

// -----------------------------------------------------------------------------

template <typename T>
class cool::cast_ptr
{
public:
    template <typename P> cast_ptr(P* p): ptr_(clang::dyn_cast<T>(p)) {}
    operator bool() const { return this->ptr_; }
    T& operator*() const  { return *this->ptr_; }
    T* operator->() const { return this->ptr_; }

private:
    T* ptr_;
};

// -----------------------------------------------------------------------------

#endif /* FRAMEWORK_CAST_PTR_HPP */
