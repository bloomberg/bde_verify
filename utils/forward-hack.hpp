// -*-c++-*- utils/forward-hack.hpp 
// -----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(UTILS_FORWARD_HACK_HPP)
#define UTILS_FORWARD_HACK_HPP 1
#ident "$Id: forward-hack.hpp 141 2011-09-29 18:59:08Z kuehl $"

// -----------------------------------------------------------------------------
// This header tries to encapsulate some of the forwarding logic relevant for
// C++2011. It is a rather crude replacement, though.

namespace utils
{
    template <typename T>
    T const&
    forward(T const& ref)
    {
        return ref;
    }

    template <typename T>
    T&
    forward(T& ref)
    {
        return ref;
    }
}

// -----------------------------------------------------------------------------

#endif /* UTILS_FORWARD_HACK_HPP */
