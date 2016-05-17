// csabase_cast_ptr.h                                                 -*-C++-*-

#ifndef INLCUDES_CSABASE_CAST_PTR
#define INLCUDES_CSABASE_CAST_PTR

// ----------------------------------------------------------------------------

namespace csabase
{
template <typename T>
class cast_ptr
{
public:
    template <typename P> cast_ptr(P const* p);
    operator bool() const;
    T const& operator*() const;
    T const* operator->() const;
    T const* get() const;

private:
    T const* d_pointer;
};

// ----------------------------------------------------------------------------

template <typename T>
template <typename P>
inline
cast_ptr<T>::cast_ptr(P const* p)
: d_pointer(llvm::dyn_cast<T>(p))
{
}

template <typename T>
inline
cast_ptr<T>::operator bool() const
{
    return d_pointer;
}

template <typename T>
inline
T const& cast_ptr<T>::operator*() const
{
    return *d_pointer;
}

template <typename T>
inline
T const* cast_ptr<T>::operator->() const
{
    return d_pointer;
}

template <typename T>
inline
T const* cast_ptr<T>::get() const
{
    return d_pointer;
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
