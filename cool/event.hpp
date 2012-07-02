// -*-c++-*- event.hpp 
// -----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(COOL_EVENT_HPP)
#define COOL_EVENT_HPP 1
#ident "$Id: event.hpp 151 2011-10-09 12:30:33Z kuehl $"

#include "function.hpp"
#include <deque>
#include <list>

// -----------------------------------------------------------------------------

namespace cool
{
    template <typename Signature> class event;
    template <>                                                   class event<void()>;
    template <typename T0>                                        class event<void(T0)>;
    template <typename T0, typename T1>                           class event<void(T0, T1)>;
    template <typename T0, typename T1, typename T2>              class event<void(T0, T1, T2)>;
    template <typename T0, typename T1, typename T2, typename T3> class event<void(T0, T1, T2, T3)>;
}

// -----------------------------------------------------------------------------

template <>
class cool::event<void()>
{
public:
    template <typename Functor> cool::event<void()>& operator+= (Functor functor);
    void operator()() const;
    operator bool() const;

private:
    std::deque<cool::function<void()> > functions_;
};

// -----------------------------------------------------------------------------

template <typename T0>
class cool::event<void(T0)>
{
public:
    template <typename Functor> cool::event<void(T0)>& operator+= (Functor functor);
    void operator()(T0) const;
    operator bool() const;

private:
    std::deque<cool::function<void(T0)> > functions_;
};

// -----------------------------------------------------------------------------

template <typename T0, typename T1>
class cool::event<void(T0, T1)>
{
public:
    template <typename Functor> cool::event<void(T0, T1)>& operator+= (Functor functor);
    void operator()(T0, T1) const;
    operator bool() const;

private:
    std::deque<cool::function<void(T0, T1)> > functions_;
};

// -----------------------------------------------------------------------------

template <typename T0, typename T1, typename T2>
class cool::event<void(T0, T1, T2)>
{
public:
    template <typename Functor> cool::event<void(T0, T1, T2)>& operator+= (Functor functor);
    void operator()(T0, T1, T2) const;
    operator bool() const;
    int size() const { return this->functions_.size(); }

private:
    typedef std::list<cool::function<void(T0, T1, T2)> > Container;
    Container functions_;
};

// -----------------------------------------------------------------------------

template <typename T0, typename T1, typename T2, typename T3>
class cool::event<void(T0, T1, T2, T3)>
{
public:
    template <typename Functor> cool::event<void(T0, T1, T2, T3)>& operator+= (Functor functor);
    void operator()(T0, T1, T2, T3) const;
    operator bool() const;

private:
    std::deque<cool::function<void(T0, T1, T2, T3)> > functions_;
};

// -----------------------------------------------------------------------------

template <typename Functor>
inline cool::event<void()>&
cool::event<void()>::operator+= (Functor functor)
{
    this->functions_.push_back(cool::function<void()>(functor));
    return *this;
}

inline cool::event<void()>::operator bool() const
{
    return !this->functions_.empty();
}

inline void
cool::event<void()>::operator()() const
{
    for (std::deque<cool::function<void()> >::const_iterator it(this->functions_.begin()), end(this->functions_.end());
         it != end; ++it)
    {
        (*it)();
    }
}

// -----------------------------------------------------------------------------

template <typename T0>
template <typename Functor>
cool::event<void(T0)>&
cool::event<void(T0)>::operator+= (Functor functor)
{
    this->functions_.push_back(cool::function<void(T0)>(functor));
    return *this;
}

template <typename T0>
cool::event<void(T0)>::operator bool() const
{
    return !this->functions_.empty();
}

template <typename T0>
void
cool::event<void(T0)>::operator()(T0 a0) const
{
    for (typename std::deque<cool::function<void(T0)> >::const_iterator it(this->functions_.begin()), end(this->functions_.end());
         it != end; ++it)
    {
        (*it)(a0);
    }
}

// -----------------------------------------------------------------------------

template <typename T0, typename T1>
template <typename Functor>
cool::event<void(T0, T1)>&
cool::event<void(T0, T1)>::operator+= (Functor functor)
{
    this->functions_.push_back(cool::function<void(T0, T1)>(functor));
    return *this;
}

template <typename T0, typename T1>
cool::event<void(T0, T1)>::operator bool() const
{
    return !this->functions_.empty();
}

template <typename T0, typename T1>
void
cool::event<void(T0, T1)>::operator()(T0 a0, T1 a1) const
{
    for (typename std::deque<cool::function<void(T0, T1)> >::const_iterator it(this->functions_.begin()),
             end(this->functions_.end()); it != end; ++it)
    {
        (*it)(a0, a1);
    }
}

// -----------------------------------------------------------------------------

template <typename T0, typename T1, typename T2>
template <typename Functor>
cool::event<void(T0, T1, T2)>&
cool::event<void(T0, T1, T2)>::operator+= (Functor functor)
{
    this->functions_.push_back(cool::function<void(T0, T1, T2)>(functor));
    return *this;
}

template <typename T0, typename T1, typename T2>
cool::event<void(T0, T1, T2)>::operator bool() const
{
    return !this->functions_.empty();
}

template <typename T0, typename T1, typename T2>
void
cool::event<void(T0, T1, T2)>::operator()(T0 a0, T1 a1, T2 a2) const
{
    for (typename Container::const_iterator it(this->functions_.begin()),
             end(this->functions_.end()); it != end; ++it)
    {
        (*it)(a0, a1, a2);
    }
}

// -----------------------------------------------------------------------------

template <typename T0, typename T1, typename T2, typename T3>
template <typename Functor>
cool::event<void(T0, T1, T2, T3)>&
cool::event<void(T0, T1, T2, T3)>::operator+= (Functor functor)
{
    this->functions_.push_back(cool::function<void(T0, T1, T2, T3)>(functor));
    return *this;
}

template <typename T0, typename T1, typename T2, typename T3>
cool::event<void(T0, T1, T2, T3)>::operator bool() const
{
    return !this->functions_.empty();
}

template <typename T0, typename T1, typename T2, typename T3>
void
cool::event<void(T0, T1, T2, T3)>::operator()(T0 a0, T1 a1, T2 a2, T3 a3) const
{
    for (typename std::deque<cool::function<void(T0, T1, T2, T3)> >::const_iterator it(this->functions_.begin()),
             end(this->functions_.end()); it != end; ++it)
    {
        (*it)(a0, a1, a2, a3);
    }
}

// -----------------------------------------------------------------------------

#endif /* COOL_EVENT_HPP */
