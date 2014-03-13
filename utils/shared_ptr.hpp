// -*-c++-*- utils/memory/shared_ptr.hpp 
// -----------------------------------------------------------------------------
// Copyright 2010 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(UTILS_MEMORY_SHARED_PTR_HPP)
#define UTILS_MEMORY_SHARED_PTR_HPP 1
#ident "$Id: shared_ptr.hpp 144 2011-10-02 02:54:06Z kuehl $"

#include <algorithm>
#include <stddef.h>

// -----------------------------------------------------------------------------

namespace utils
{
    template <typename T> class shared_ptr;
    template <typename T, typename F> utils::shared_ptr<T> dynamic_pointer_cast(utils::shared_ptr<F>);
    //-dk:TODO? template <typename T> bool operator== (utils::shared_ptr<T> const&, utils::shared_ptr<T> const&);
    //-dk:TODO? template <typename T> bool operator!= (utils::shared_ptr<T> const&, utils::shared_ptr<T> const&);
}

// -----------------------------------------------------------------------------

//-dk:TODO shared_ptr is for now a very rough approximation...!
template <typename T>
class utils::shared_ptr
{
public:
    template <typename S> friend class utils::shared_ptr;

    explicit shared_ptr(T* ptr = 0): ptr_(ptr), count_(new size_t(1)) {}
    shared_ptr(utils::shared_ptr<T> const& other):
        ptr_(other.ptr_),
        count_(other.count_)
    {
        ++*count_;
    }
    template <typename S>
    shared_ptr(utils::shared_ptr<S> const& other):
        ptr_(other.ptr_),
        count_(other.count_)
    {
        ++*count_;
    }
    utils::shared_ptr<T>& operator= (utils::shared_ptr<T> const& other)
    {
        utils::shared_ptr<T>(other).swap(*this);
        return *this;
    }
    ~shared_ptr()
    {
        reset();
    }
    void reset()
    {
        if (count_ && !--*count_)
        {
            delete count_;
            delete ptr_;
        }
        count_ = 0;
        ptr_   = 0;
    }
    void swap(utils::shared_ptr<T>& other)
    {
        std::swap(ptr_, other.ptr_);
        std::swap(count_, other.count_);
    }
    operator bool() const { return ptr_; }
    T*       operator->() const { return ptr_; }
    T&       operator*() const  { return *ptr_; }
    T*       get() const { return ptr_; }

    template <typename S>
    utils::shared_ptr<S> cast() const
    {
        S* ptr(dynamic_cast<S*>(ptr_));
        return ptr? utils::shared_ptr<S>(ptr, count_): utils::shared_ptr<S>();
    }

private:
    shared_ptr(T* ptr, size_t* count): ptr_(ptr), count_(count) { ++*count_; }
    T*      ptr_;
    size_t* count_;
};

// -----------------------------------------------------------------------------

template <typename T, typename F>
utils::shared_ptr<T>
utils::dynamic_pointer_cast(utils::shared_ptr<F> ptr)
{
    return ptr.template cast<T>();
}

// -----------------------------------------------------------------------------

#endif /* UTILS_MEMORY_SHARED_PTR_HPP */
