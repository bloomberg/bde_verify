// -*-c++-*- utils/remove_reference.hpp 
// -----------------------------------------------------------------------------
// Copyright 2010 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(UTILS_REMOVE_REFERENCE_HPP)
#define UTILS_REMOVE_REFERENCE_HPP 1
#ident "$Id: remove_reference.hpp 141 2011-09-29 18:59:08Z kuehl $"

// -----------------------------------------------------------------------------

namespace utils
{
    template <typename> struct remove_reference;
    template <typename T> struct remove_reference<T&>;
    template <typename T> struct remove_reference<T&&>;
}

// -----------------------------------------------------------------------------

template <typename T>
struct utils::remove_reference
{
    typedef T type;
};

// -----------------------------------------------------------------------------

template <typename T>
struct utils::remove_reference<T&>
{
    typedef T type;
};

// -----------------------------------------------------------------------------

template <typename T>
struct utils::remove_reference<T&&>
{
    typedef T type;
};

// -----------------------------------------------------------------------------

#endif /* UTILS_REMOVE_REFERENCE_HPP */
