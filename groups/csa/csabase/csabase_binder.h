// csabase_binder.h                                                   -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#if !defined(INCLUDED_CSABASE_BINDER)
#define INCLUDED_CSABASE_BINDER 1

// ----------------------------------------------------------------------------

namespace bde_verify
{
    namespace csabase
    {
        class Analyser;

        template <typename Function> class Binder;
        template <>
        class Binder<void(*)(Analyser*)>;
        template <typename T0>
        class Binder<void(*)(Analyser*, T0)>;
        template <typename T0, typename T1>
        class Binder<void(*)(Analyser*, T0, T1)>;
        template <typename T0, typename T1, typename T2>
        class Binder<void(*)(Analyser*, T0, T1, T2)>;

        template <typename Function>
        Binder<Function> bind(Analyser*, Function);
    }
}

// ----------------------------------------------------------------------------

template <>
class bde_verify::csabase::Binder<void(*)(bde_verify::csabase::Analyser*)>
{
private:
    Analyser* d_analyser;
    void      (*d_function)(Analyser*);

public:
    Binder(Analyser* analyser, void (*function)(Analyser*))
        : d_analyser(analyser)
        , d_function(function)
    {
    }
    void operator()() const
    {
        (d_function)(d_analyser);
    }
};

// ----------------------------------------------------------------------------

template <typename T0>
class bde_verify::csabase::Binder<void(*)(bde_verify::csabase::Analyser*, T0)>
{
private:
    Analyser* d_analyser;
    void      (*d_function)(Analyser*, T0);

public:
    Binder(Analyser* analyser, void (*function)(Analyser*, T0))
        : d_analyser(analyser)
        , d_function(function)
    {
    }
    void operator()(T0 a0) const
    {
        (d_function)(d_analyser, a0);
    }
};

// ----------------------------------------------------------------------------

template <typename T0, typename T1>
class bde_verify::csabase::Binder<void(*)(bde_verify::csabase::Analyser*, T0, T1)>
{
private:
    Analyser* d_analyser;
    void      (*d_function)(Analyser*, T0, T1);

public:
    Binder(Analyser* analyser, void (*function)(Analyser*, T0, T1))
        : d_analyser(analyser)
        , d_function(function)
    {
    }
    void operator()(T0 a0, T1 a1) const
    {
        (d_function)(d_analyser, a0, a1);
    }
};

// ----------------------------------------------------------------------------

template <typename T0, typename T1, typename T2>
class bde_verify::csabase::Binder<void(*)(bde_verify::csabase::Analyser*, T0, T1, T2)>
{
private:
    Analyser* d_analyser;
    void      (*d_function)(Analyser*, T0, T1, T2);

public:
    Binder(Analyser* analyser, void (*function)(Analyser*, T0, T1, T2))
        : d_analyser(analyser)
        , d_function(function)
    {
    }
    void operator()(T0 a0, T1 a1, T2 a2) const
    {
        (d_function)(d_analyser, a0, a1, a2);
    }
};

// ----------------------------------------------------------------------------

template <typename Function>
bde_verify::csabase::Binder<Function>
bde_verify::csabase::bind(bde_verify::csabase::Analyser* analyser, Function function)
{
    return bde_verify::csabase::Binder<Function>(analyser, function);
}

// ----------------------------------------------------------------------------

#endif
