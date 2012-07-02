// -*-c++-*- cool/forward.hpp 
// -----------------------------------------------------------------------------
// Copyright 2010 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(COOL_FORWARD_HPP)
#define COOL_FORWARD_HPP 1
#ident "$Id: forward.hpp 141 2011-09-29 18:59:08Z kuehl $"

#include "cool/remove_reference.hpp"

// -----------------------------------------------------------------------------

namespace cool
{
    template <typename T>
    T&&
    forward(typename cool::remove_reference<T>::type& arg);

    template <typename T>
    T&&
    forward(typename cool::remove_reference<T>::type&& arg);
}

// -----------------------------------------------------------------------------

template <typename T>
T&&
cool::forward(typename cool::remove_reference<T>::type& arg)
{
    return static_cast<T&&>(arg);
}

template <typename T>
T&&
cool::forward(typename cool::remove_reference<T>::type&& arg) 
{
    return static_cast<T&&>(arg);
}

// -----------------------------------------------------------------------------

#endif /* COOL_FORWARD_HPP */
