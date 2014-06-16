// csabase_registercheck.cpp                                          -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_registercheck.h>
#include <csabase_binder.h>
#include <csabase_checkregistry.h>
#include <csabase_visitor.h>
#ident "$Id$"

using namespace csabase;
using namespace clang;

// -----------------------------------------------------------------------------

namespace
{
template <typename T>
struct add_to_event
{
    add_to_event(utils::event<void(T const*)> Visitor::*member,
                 void (*check)(Analyser&, T const*));
    void operator()(Analyser& a, Visitor& v, PPObserver&);

    utils::event<void(T const*)> Visitor::*member_;
    void (*check_)(Analyser&, T const*);
};

template <typename T>
add_to_event<T>::add_to_event(utils::event<void(T const*)> Visitor::*member,
                              void (*check)(Analyser&, T const*))
: member_(member), check_(check)
{
}

template <typename T>
void add_to_event<T>::operator()(Analyser& a, Visitor& v, PPObserver&)
{
    v.*member_ += Binder<Analyser&, void(*)(Analyser&, T const *)>(a, check_);
}
}

// -----------------------------------------------------------------------------

namespace csabase {

#define REGISTER(D)                                                  \
    template <>                                                      \
    RegisterCheck::RegisterCheck<D>(                                 \
        std::string const& name, void (*check)(Analyser&, D const*)) \
    {                                                                \
        CheckRegistry::add_check(                                    \
            name, add_to_event<D>(&Visitor::on##D, check));          \
    }

#define ABSTRACT_DECL(ARG) ARG
#define DECL(CLASS, BASE) REGISTER(CLASS##Decl)
DECL(, )
#include "clang/AST/DeclNodes.inc"

#define ABSTRACT_STMT(STMT)
#define STMT(CLASS, PARENT) REGISTER(CLASS)
STMT(Stmt, )
#include "clang/AST/StmtNodes.inc"
REGISTER(Expr)

csabase::RegisterCheck::RegisterCheck(
    std::string const& name,
    CheckRegistry::Subscriber subscriber)
{
    CheckRegistry::add_check(name, subscriber);
}

}
