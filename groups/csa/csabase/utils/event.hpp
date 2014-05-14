// -*-c++-*- event.hpp 
// -----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(UTILS_EVENT_HPP)
#define UTILS_EVENT_HPP 1
#ident "$Id: event.hpp 151 2011-10-09 12:30:33Z kuehl $"

#include "function.hpp"
#include <deque>
#include <list>

// -----------------------------------------------------------------------------

namespace utils
{

template <typename Signature> class event;

template <typename ...T>
class event<void(T...)>
{
  public:
    template <typename Functor>
    event& operator+=(Functor functor)
    {
        functions_.push_back(function<void(T...)>(functor));
        return *this;
    }

    void operator()(T...a) const
    {
        for (const auto &f : functions_) {
            f(a...);
        }
    }

    operator bool() const
    {
        return !functions_.empty();
    }

private:
    std::deque<function<void(T...)>> functions_;
};

}

#endif /* UTILS_EVENT_HPP */
