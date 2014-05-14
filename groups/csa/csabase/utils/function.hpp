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

template <typename RC, typename ...T>
class utils::function<RC(T...)>
{
public:
    template <typename Functor> function(Functor const&);
    function(utils::function<RC(T...)> const&);
    utils::function<RC(T...)>& operator=(utils::function<RC(T...)> const&);
    ~function();
    void swap(utils::function<RC(T...)>&);

    RC operator()(T...) const;

private:
    struct base
    {
        virtual ~base() {}
        virtual base* clone() = 0;
        virtual RC call(T...) = 0;
    }* function_;

    template <typename Functor>
    struct concrete : base
    {
        concrete(Functor functor): functor_(functor) {}
        base* clone() { return new concrete<Functor>(*this); }
        RC    call(T...a) { return functor_(a...); }

        Functor functor_;
    };

    template <typename Functor>
    static base* make_function(Functor functor)
    {
        return new concrete<Functor>(functor);
    }
};

// -----------------------------------------------------------------------------

template <typename RC, typename ...T>
template <typename Functor>
utils::function<RC(T...)>::function(Functor const& functor):
    function_(make_function(functor))
{
}

template <typename RC, typename ...T>
utils::function<RC(T...)>::function(utils::function<RC(T...)> const& other):
    function_(other.function_->clone())
{
}

template <typename RC, typename ...T>
utils::function<RC(T...)>&
utils::function<RC(T...)>::operator=(utils::function<RC(T...)> const& other)
{
    utils::function<RC(T...)>(other).swap(*this);
    return *this;
}

template <typename RC, typename ...T>
utils::function<RC(T...)>::~function()
{
    delete function_;
}

template <typename RC, typename ...T>
void utils::function<RC(T...)>::swap(utils::function<RC(T...)>& other)
{
    std::swap(function_, other.function_);
}

template <typename RC, typename ...T>
RC utils::function<RC(T...)>::operator()(T...a) const
{
    return function_->call(a...);
}

// -----------------------------------------------------------------------------

#endif /* UTILS_FUNCTION_HPP */
