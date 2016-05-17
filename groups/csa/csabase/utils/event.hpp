// event.hpp                                                          -*-C++-*-

#ifndef INCLUDED_UTILS_EVENT_HPP
#define INCLUDED_UTILS_EVENT_HPP

#include <utils/function.hpp>
#include <deque>

// -----------------------------------------------------------------------------

namespace utils
{
template <typename Signature>
class event;

template <typename...T>
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

// This allows using decltype(function pointer) as the type parameter.
template <typename R, typename...T>
class event<R(*)(T...)> : public event<void(T...)>
{
};

// This allows using decltype(method pointer) as the type parameter.
template <typename R, typename C, typename...T>
class event<R(C::*)(T...)> : public event<void(T...)>
{
};
}

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
