// -*-c++-*- function.hpp 
// -----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(COOL_FUNCTION_HPP)
#define COOL_FUNCTION_HPP 1
#ident "$Id: function.hpp 141 2011-09-29 18:59:08Z kuehl $"

#include <algorithm>

// -----------------------------------------------------------------------------

namespace cool
{
    template <typename Signature> class function;
    template <typename RC>                                        class function<RC()>;
    template <typename RC, typename T0>                           class function<RC(T0)>;
    template <typename RC, typename T0, typename T1>              class function<RC(T0, T1)>;
    template <typename RC, typename T0, typename T1, typename T2> class function<RC(T0, T1, T2)>;
}

// -----------------------------------------------------------------------------

template <typename RC>
class cool::function<RC()>
{
public:
    template <typename Functor> function(Functor const&);
    function(cool::function<RC()> const&);
    cool::function<RC()>& operator=(cool::function<RC()> const&);
    ~function();
    void swap(cool::function<RC()>&);

    RC operator()() const;

private:
    struct base
    {
        virtual ~base() {}
        virtual base* clone() = 0;
        virtual RC call() = 0;
    }* function_;

    template <typename Functor>
    struct concrete:
        base
    {
        concrete(Functor functor): functor_(functor) {}
        base* clone() { return new concrete<Functor>(*this); }
        RC    call() { return this->functor_(); }

        Functor functor_;
    };
    
    template <typename Functor>
    static base* make_function(Functor functor) { return new concrete<Functor>(functor); }
};

// -----------------------------------------------------------------------------

template <typename RC, typename T0>
class cool::function<RC(T0)>
{
public:
    template <typename Functor> function(Functor const&);
    function(cool::function<RC(T0)> const&);
    cool::function<RC(T0)>& operator=(cool::function<RC(T0)> const&);
    ~function();
    void swap(cool::function<RC(T0)>&);

    RC operator()(T0) const;

private:
    struct base
    {
        virtual ~base() {}
        virtual base* clone() = 0;
        virtual RC call(T0) = 0;
    }* function_;

    template <typename Functor>
    struct concrete:
        base
    {
        concrete(Functor functor): functor_(functor) {}
        base* clone() { return new concrete<Functor>(*this); }
        RC    call(T0 a0) { return this->functor_(a0); }

        Functor functor_;
    };
    
    template <typename Functor>
    static base* make_function(Functor functor) { return new concrete<Functor>(functor); }
};

// -----------------------------------------------------------------------------

template <typename RC, typename T0, typename T1>
class cool::function<RC(T0, T1)>
{
public:
    template <typename Functor> function(Functor const&);
    function(cool::function<RC(T0, T1)> const&);
    cool::function<RC(T0, T1)>& operator=(cool::function<RC(T0, T1)> const&);
    ~function();
    void swap(cool::function<RC(T0, T1)>&);

    RC operator()(T0, T1) const;

private:
    struct base
    {
        virtual ~base() {}
        virtual base* clone() = 0;
        virtual RC call(T0, T1) = 0;
    }* function_;

    template <typename Functor>
    struct concrete:
        base
    {
        concrete(Functor functor): functor_(functor) {}
        base* clone() { return new concrete<Functor>(*this); }
        RC    call(T0 a0, T1 a1) { return this->functor_(a0, a1); }

        Functor functor_;
    };
    
    template <typename Functor>
    static base* make_function(Functor functor) { return new concrete<Functor>(functor); }
};

// -----------------------------------------------------------------------------

template <typename RC, typename T0, typename T1, typename T2>
class cool::function<RC(T0, T1, T2)>
{
public:
    template <typename Functor> function(Functor const&);
    function(cool::function<RC(T0, T1, T2)> const&);
    cool::function<RC(T0, T1, T2)>& operator=(cool::function<RC(T0, T1, T2)> const&);
    ~function();
    void swap(cool::function<RC(T0, T1, T2)>&);

    RC operator()(T0, T1, T2) const;

private:
    struct base
    {
        virtual ~base() {}
        virtual base* clone() = 0;
        virtual RC call(T0, T1, T2) = 0;
    }* function_;

    template <typename Functor>
    struct concrete:
        base
    {
        concrete(Functor functor): functor_(functor) {}
        base* clone() { return new concrete<Functor>(*this); }
        RC    call(T0 a0, T1 a1, T2 a2) { return this->functor_(a0, a1, a2); }

        Functor functor_;
    };
    
    template <typename Functor>
    static base* make_function(Functor functor) { return new concrete<Functor>(functor); }
};

// -----------------------------------------------------------------------------

template <typename RC>
template <typename Functor>
cool::function<RC()>::function(Functor const& functor):
    function_(make_function(functor))
{
}

template <typename RC>
cool::function<RC()>::function(cool::function<RC()> const& other):
    function_(other.function_->clone())
{
}

template <typename RC>
cool::function<RC()>&
cool::function<RC()>::operator=(cool::function<RC()> const& other)
{
    cool::function<RC()>(other).swap(*this);
    return *this;
}

template <typename RC>
cool::function<RC()>::~function()
{
    delete this->function_;
}

template <typename RC>
void cool::function<RC()>::swap(cool::function<RC()>& other)
{
    std::swap(this->function_, other.function_);
}

template <typename RC>
RC cool::function<RC()>::operator()() const
{
    return this->function_->call();
}

// -----------------------------------------------------------------------------

template <typename RC, typename T0>
template <typename Functor>
cool::function<RC(T0)>::function(Functor const& functor):
    function_(make_function(functor))
{
}

template <typename RC, typename T0>
cool::function<RC(T0)>::function(cool::function<RC(T0)> const& other):
    function_(other.function_->clone())
{
}

template <typename RC, typename T0>
cool::function<RC(T0)>&
cool::function<RC(T0)>::operator=(cool::function<RC(T0)> const& other)
{
    cool::function<RC(T0)>(other).swap(*this);
    return *this;
}

template <typename RC, typename T0>
cool::function<RC(T0)>::~function()
{
    delete this->function_;
}

template <typename RC, typename T0>
void cool::function<RC(T0)>::swap(cool::function<RC(T0)>& other)
{
    std::swap(this->function_, other.function_);
}

template <typename RC, typename T0>
RC cool::function<RC(T0)>::operator()(T0 a0) const
{
    return this->function_->call(a0);
}

// -----------------------------------------------------------------------------

template <typename RC, typename T0, typename T1>
template <typename Functor>
cool::function<RC(T0, T1)>::function(Functor const& functor):
    function_(make_function(functor))
{
}

template <typename RC, typename T0, typename T1>
cool::function<RC(T0, T1)>::function(cool::function<RC(T0, T1)> const& other):
    function_(other.function_->clone())
{
}

template <typename RC, typename T0, typename T1>
cool::function<RC(T0, T1)>&
cool::function<RC(T0, T1)>::operator=(cool::function<RC(T0, T1)> const& other)
{
    cool::function<RC(T0, T1)>(other).swap(*this);
    return *this;
}

template <typename RC, typename T0, typename T1>
cool::function<RC(T0, T1)>::~function()
{
    delete this->function_;
}

template <typename RC, typename T0, typename T1>
void cool::function<RC(T0, T1)>::swap(cool::function<RC(T0, T1)>& other)
{
    std::swap(this->function_, other.function_);
}

template <typename RC, typename T0, typename T1>
RC cool::function<RC(T0, T1)>::operator()(T0 a0, T1 a1) const
{
    return this->function_->call(a0, a1);
}

// -----------------------------------------------------------------------------

template <typename RC, typename T0, typename T1, typename T2>
template <typename Functor>
cool::function<RC(T0, T1, T2)>::function(Functor const& functor):
    function_(make_function(functor))
{
}

template <typename RC, typename T0, typename T1, typename T2>
cool::function<RC(T0, T1, T2)>::function(cool::function<RC(T0, T1, T2)> const& other):
    function_(other.function_->clone())
{
}

template <typename RC, typename T0, typename T1, typename T2>
cool::function<RC(T0, T1, T2)>&
cool::function<RC(T0, T1, T2)>::operator=(cool::function<RC(T0, T1, T2)> const& other)
{
    cool::function<RC(T0, T1, T2)>(other).swap(*this);
    return *this;
}

template <typename RC, typename T0, typename T1, typename T2>
cool::function<RC(T0, T1, T2)>::~function()
{
    delete this->function_;
}

template <typename RC, typename T0, typename T1, typename T2>
void cool::function<RC(T0, T1, T2)>::swap(cool::function<RC(T0, T1, T2)>& other)
{
    std::swap(this->function_, other.function_);
}

template <typename RC, typename T0, typename T1, typename T2>
RC cool::function<RC(T0, T1, T2)>::operator()(T0 a0, T1 a1, T2 a2) const
{
    return this->function_->call(a0, a1, a2);
}

// -----------------------------------------------------------------------------

#endif /* COOL_FUNCTION_HPP */
