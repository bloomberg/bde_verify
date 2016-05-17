// function.hpp                                                       -*-C++-*-

#ifndef INCLUDED_UTILS_FUNCTION_HPP
#define INCLUDED_UTILS_FUNCTION_HPP

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
