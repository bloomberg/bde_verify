// csabase_cast_ptr.hpp                                               -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#if !defined(INLCUDES_CSABASE_CAST_PTR)
#define INLCUDES_CSABASE_CAST_PTR 1

// ----------------------------------------------------------------------------

namespace bde_verify
{
    namespace csabase
    {
        template <typename T> class cast_ptr;
    }
}

// ----------------------------------------------------------------------------

template <typename T>
class bde_verify::csabase::cast_ptr
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
bde_verify::csabase::cast_ptr<T>::cast_ptr(P const* p)
  : d_pointer(llvm::dyn_cast<T>(p))
{
}

template <typename T>
bde_verify::csabase::cast_ptr<T>::operator bool() const
{
    return d_pointer;
}

template <typename T>
T const&
bde_verify::csabase::cast_ptr<T>::operator*() const
{
    return *d_pointer;
}

template <typename T>
T const*
bde_verify::csabase::cast_ptr<T>::operator->() const
{
    return d_pointer;
}

template <typename T>
T const*
bde_verify::csabase::cast_ptr<T>::get() const
{
    return d_pointer;
}

// ----------------------------------------------------------------------------

#endif /* INLCUDES_CSABASE_CAST_PTR */
