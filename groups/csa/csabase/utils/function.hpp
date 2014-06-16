// -*-c++-*- vfunction.hpp 
// -----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(UTILS_FUNCTION_HPP)
#define UTILS_FUNCTION_HPP 1
#ident "$Id$"

#include <algorithm>

// -----------------------------------------------------------------------------

namespace utils
{
    template <typename Signature> class function;
    template <typename RC, typename ...T> class function<RC(T...)>;
}

// -----------------------------------------------------------------------------

template <typename RC, typename... T>
class utils::function<RC(T...)>
{
  private:
    struct base
    {
        virtual ~base() { }
        virtual base* clone() = 0;
        virtual RC call(T...) = 0;
    } *function_;

    template <typename Functor>
    struct concrete : base
    {
        concrete(Functor functor) : functor_(functor) { }
        base* clone() { return new concrete(*this); }
        RC call(T... a) { return functor_(a...); }

        Functor functor_;
    };

  public:
    template <typename Functor>
    function(Functor const& functor)
        : function_(new concrete<Functor>(functor))
    {
    }

    function(function const& other) : function_(other.function_->clone())
    {
    }

    function& operator=(function const& other)
    {
        function(other).swap(*this);
        return *this;
    }

    ~function()
    {
        delete function_;
    }

    void swap(function& other)
    {
        std::swap(function_, other.function_);
    }

    RC operator()(T...a) const
    {
        return function_->call(a...);
    }
};

// -----------------------------------------------------------------------------

#endif /* UTILS_FUNCTION_HPP */
