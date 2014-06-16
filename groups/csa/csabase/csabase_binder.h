// csabase_binder.h                                                   -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#if !defined(INCLUDED_CSABASE_BINDER)
#define INCLUDED_CSABASE_BINDER 1

// ----------------------------------------------------------------------------

namespace csabase {
    template <typename Object, typename Function> class Binder;

    template <typename Object, typename... T>
    class Binder<Object, void(*)(Object, T...)>;

    template <typename Object, typename Function>
    Binder<Object, Function> bind(Object, Function);
} // close package namespace

// ----------------------------------------------------------------------------

template <typename Object, typename...T>
class csabase::Binder<Object, void(*)(Object, T...)>
{
private:
    Object d_object;
    void (*d_function)(Object, T...);

public:
    Binder(Object object, void (*function)(Object, T...))
        : d_object(object)
        , d_function(function)
    {
    }
    void operator()(T...a) const
    {
        d_function(d_object, a...);
    }
};

// ----------------------------------------------------------------------------

template <typename Object, typename Function>
csabase::Binder<Object, Function>
csabase::bind(Object object, Function function)
{
    return csabase::Binder<Object, Function>(object, function);
}

// ----------------------------------------------------------------------------

#endif
