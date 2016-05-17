// csabase_binder.h                                                   -*-C++-*-

#ifndef INCLUDED_CSABASE_BINDER
#define INCLUDED_CSABASE_BINDER

// ----------------------------------------------------------------------------

namespace csabase
{
template <typename Object, typename Function>
class Binder;

template <typename Object, typename...T>
class Binder<Object, void(*)(Object, T...)>
{
private:
    Object d_object;
    void (*d_function)(Object, T...);

public:
    Binder(Object object, void (*function)(Object, T...));
    void operator()(T...a) const;
};

template <typename Object, typename... T>
inline
Binder<Object, void (*)(Object, T...)>::Binder(Object object,
                                               void (*function)(Object, T...))
: d_object(object)
, d_function(function)
{
}

template <typename Object, typename... T>
inline
void Binder<Object, void (*)(Object, T...)>::operator()(T... a) const
{
    d_function(d_object, a...);
}

template <typename Object, typename Function>
inline
Binder<Object, Function> bind(Object object, Function function)
{
    return Binder<Object, Function>(object, function);
}
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
