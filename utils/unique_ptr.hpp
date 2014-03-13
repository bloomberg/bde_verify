// -*-c++-*- utils/unique_ptr.hpp 
// -----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(UTILS_UNIQUE_PTR_HPP)
#define UTILS_UNIQUE_PTR_HPP 1
#ident "$Id: unique_ptr.hpp 141 2011-09-29 18:59:08Z kuehl $"

// -----------------------------------------------------------------------------

namespace utils
{
    template <typename T> class unique_ptr;

    template<class T>
    void swap(utils::unique_ptr<T>&, utils::unique_ptr<T>&);

    template <typename T0, typename T1>
    bool operator==(utils::unique_ptr<T0> const&, utils::unique_ptr<T1> const&);
    template <typename T0, typename T1>
    bool operator!=(utils::unique_ptr<T0> const&, utils::unique_ptr<T1> const&);
    template <typename T0, typename T1>
    bool operator< (utils::unique_ptr<T0> const&, utils::unique_ptr<T1> const&);
    template <typename T0, typename T1>
    bool operator<=(utils::unique_ptr<T0> const&, utils::unique_ptr<T1> const&);
    template <typename T0, typename T1>
    bool operator> (utils::unique_ptr<T0> const&, utils::unique_ptr<T1> const&);
    template <typename T0, typename T1>
    bool operator>=(utils::unique_ptr<T0> const&, utils::unique_ptr<T1> const&);
}

// -----------------------------------------------------------------------------

template <typename T>
class utils::unique_ptr
{
public:
    typedef T* pointer;
    typedef T element_type;

    unique_ptr();
    unique_ptr(pointer);
    unique_ptr(utils::unique_ptr<T>&) = delete;
    unique_ptr(utils::unique_ptr<T>&&);

    template <typename TO>
    unique_ptr(utils::unique_ptr<TO>&&);

    ~unique_ptr();

    utils::unique_ptr<T>& operator= (utils::unique_ptr<T>&) = delete;
    utils::unique_ptr<T>& operator= (utils::unique_ptr<T>&&);
    template <typename TO>
    utils::unique_ptr<T>& operator= (utils::unique_ptr<TO>&&);

    T&     operator*() const;
    pointer operator->() const;
    pointer get() const;
    operator bool() const;

    pointer release();
    void    reset(pointer = pointer());
    void    swap(utils::unique_ptr<T>&);

private:
    pointer pointer_;
};

// -----------------------------------------------------------------------------

template <typename T>
utils::unique_ptr<T>::unique_ptr():
    pointer_()
{
}

template <typename T>
utils::unique_ptr<T>::unique_ptr(pointer p):
    pointer_(p)
{
}

template <typename T>
utils::unique_ptr<T>::unique_ptr(utils::unique_ptr<T>&& other):
    pointer_(other.release())
{
}

template <typename T>
    template <typename TO>
utils::unique_ptr<T>::unique_ptr(utils::unique_ptr<TO>&& other):
    pointer_(other.release())
{
}

// -----------------------------------------------------------------------------

template <typename T>
utils::unique_ptr<T>::~unique_ptr()
{
    delete get();
}

// -----------------------------------------------------------------------------

template <typename T>
utils::unique_ptr<T>&
utils::unique_ptr<T>::operator= (utils::unique_ptr<T>&& other)
{
    reset(other.release());
    return *this;
}

template <typename T>
    template <typename TO>
utils::unique_ptr<T>&
utils::unique_ptr<T>::operator= (utils::unique_ptr<TO>&& other)
{
    reset(other.release());
    return *this;
}

// -----------------------------------------------------------------------------

template <typename T>
T&
utils::unique_ptr<T>::operator*() const
{
    return *pointer_;
}

template <typename T>
typename utils::unique_ptr<T>::pointer
utils::unique_ptr<T>::operator->() const
{
    return pointer_;
}

template <typename T>
typename utils::unique_ptr<T>::pointer
utils::unique_ptr<T>::get() const
{
    return pointer_;
}

template <typename T>
utils::unique_ptr<T>::operator bool() const
{
    return pointer_ != nullptr;
}

// -----------------------------------------------------------------------------

template <typename T>
typename utils::unique_ptr<T>::pointer
utils::unique_ptr<T>::release()
{
    typename utils::unique_ptr<T>::pointer rc(pointer_);
    pointer_ = 0;
    return rc;
}

template <typename T>
void
utils::unique_ptr<T>::reset(pointer value)
{
    pointer rel(pointer_);
    pointer_ = value;
    if (rel)
    {
        delete get();
    }
}

// -----------------------------------------------------------------------------

template<class T>
void
utils::unique_ptr<T>::swap(utils::unique_ptr<T>& p)
{
    utils::swap(pointer_, p.pointer_);
}

template<class T>
void
utils::swap(utils::unique_ptr<T>& p0, utils::unique_ptr<T>& p1)
{
    p0.swap(p1);
}

// -----------------------------------------------------------------------------

template <typename T0, typename T1>
bool
utils::operator==(utils::unique_ptr<T0> const& p0, utils::unique_ptr<T1> const& p1)
{
    return p0.get() == p1.get();
}

template <typename T0, typename T1>
bool
utils::operator!=(utils::unique_ptr<T0> const& p0, utils::unique_ptr<T1> const& p1)
{
    return !(p0 == p1);
}

template <typename T0, typename T1>
bool
utils::operator< (utils::unique_ptr<T0> const& p0, utils::unique_ptr<T1> const& p1)
{
    return p0.get() < p1.get();
}

template <typename T0, typename T1>
bool
utils::operator<=(utils::unique_ptr<T0> const& p0, utils::unique_ptr<T1> const& p1)
{
    return !(p1 < p0);
}

template <typename T0, typename T1>
bool
utils::operator> (utils::unique_ptr<T0> const& p0, utils::unique_ptr<T1> const& p1)
{
    return p1 < p0;
}

template <typename T0, typename T1>
bool
utils::operator>=(utils::unique_ptr<T0> const& p0, utils::unique_ptr<T1> const& p1)
{
    return !(p0 < p1);
}

// -----------------------------------------------------------------------------

#endif /* UTILS_UNIQUE_PTR_HPP */
