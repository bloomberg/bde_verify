// -*-c++-*- cool/memory/shared_ptr.hpp 
// -----------------------------------------------------------------------------
// Copyright 2010 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(COOL_MEMORY_SHARED_PTR_HPP)
#define COOL_MEMORY_SHARED_PTR_HPP 1
#ident "$Id: shared_ptr.hpp 144 2011-10-02 02:54:06Z kuehl $"

#include <algorithm>
#include <stddef.h>

// -----------------------------------------------------------------------------

namespace cool
{
    template <typename T> class shared_ptr;
    template <typename T, typename F> cool::shared_ptr<T> dynamic_pointer_cast(cool::shared_ptr<F>);
    //-dk:TODO? template <typename T> bool operator== (cool::shared_ptr<T> const&, cool::shared_ptr<T> const&);
    //-dk:TODO? template <typename T> bool operator!= (cool::shared_ptr<T> const&, cool::shared_ptr<T> const&);
}

// -----------------------------------------------------------------------------

//-dk:TODO shared_ptr is for now a very rough approximation...!
template <typename T>
class cool::shared_ptr
{
public:
    template <typename S> friend class cool::shared_ptr;

    explicit shared_ptr(T* ptr = 0): ptr_(ptr), count_(new size_t(1)) {}
    shared_ptr(cool::shared_ptr<T> const& other):
        ptr_(other.ptr_),
        count_(other.count_)
    {
        ++*this->count_;
    }
    template <typename S>
    shared_ptr(cool::shared_ptr<S> const& other):
        ptr_(other.ptr_),
        count_(other.count_)
    {
        ++*this->count_;
    }
    cool::shared_ptr<T>& operator= (cool::shared_ptr<T> const& other)
    {
        cool::shared_ptr<T>(other).swap(*this);
        return *this;
    }
    ~shared_ptr()
    {
        this->reset();
    }
    void reset()
    {
        if (this->count_ && !--*this->count_)
        {
            delete this->count_;
            delete this->ptr_;
        }
        this->count_ = 0;
        this->ptr_   = 0;
    }
    void swap(cool::shared_ptr<T>& other)
    {
        std::swap(this->ptr_, other.ptr_);
        std::swap(this->count_, other.count_);
    }
    operator bool() const { return this->ptr_; }
    T*       operator->() const { return this->ptr_; }
    T&       operator*() const  { return *this->ptr_; }
    T*       get() const { return this->ptr_; }

    template <typename S>
    cool::shared_ptr<S> cast() const
    {
        S* ptr(dynamic_cast<S*>(this->ptr_));
        return ptr? cool::shared_ptr<S>(ptr, this->count_): cool::shared_ptr<S>();
    }

private:
    shared_ptr(T* ptr, size_t* count): ptr_(ptr), count_(count) { ++*this->count_; }
    T*      ptr_;
    size_t* count_;
};

// -----------------------------------------------------------------------------

template <typename T, typename F>
cool::shared_ptr<T>
cool::dynamic_pointer_cast(cool::shared_ptr<F> ptr)
{
    return ptr.template cast<T>();
}

// -----------------------------------------------------------------------------

#endif /* COOL_MEMORY_SHARED_PTR_HPP */
