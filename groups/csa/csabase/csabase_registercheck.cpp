// csabase_registercheck.cpp                                          -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_registercheck.h>
#include <csabase_checkregistry.h>
#include <csabase_visitor.h>
#ident "$Id$"

// -----------------------------------------------------------------------------

namespace
{
    template <typename T>
    struct binder
    {
        binder(cool::csabase::Analyser& a, void (*check)(cool::csabase::Analyser&, T const*));
        void operator()(T const* argument);

        cool::csabase::Analyser& analyser_;
        void            (*check_)(cool::csabase::Analyser&, T const*);
    };
}

template <typename T>
::binder<T>::binder(cool::csabase::Analyser& a, void (*check)(cool::csabase::Analyser&, T const*)):
    analyser_(a),
    check_(check)
{
}

template <typename T>
void
::binder<T>::operator()(T const* argument)
{
    this->check_(this->analyser_, argument);
}

// -----------------------------------------------------------------------------

namespace
{
    template <typename T>
    struct add_to_event
    {
        add_to_event(cool::event<void(T const*)> cool::csabase::Visitor::*member, void (*check)(cool::csabase::Analyser&, T const*));
        void operator()(cool::csabase::Analyser& a, cool::csabase::Visitor& v, cool::csabase::PPObserver&);

        cool::event<void(T const*)> cool::csabase::Visitor::*member_;
        void                        (*check_)(cool::csabase::Analyser&, T const*);
    };
}

template <typename T>
::add_to_event<T>::add_to_event(cool::event<void(T const*)> cool::csabase::Visitor::*member, void (*check)(cool::csabase::Analyser&, T const*)):
    member_(member),
    check_(check)
{
}

template <typename T>
void
::add_to_event<T>::operator()(cool::csabase::Analyser& a, cool::csabase::Visitor& v, cool::csabase::PPObserver&)
{
    v.*(this->member_) += binder<T>(a, this->check_);
}

// -----------------------------------------------------------------------------

namespace
{
    template <typename> struct map_to_member;
}

// -----------------------------------------------------------------------------

template <typename T>
cool::csabase::RegisterCheck::RegisterCheck(std::string const& name, void (*check)(cool::csabase::Analyser&, T const*))
{
    cool::csabase::CheckRegistry::add_check(name, add_to_event<T>(map_to_member<T>::member(), check));
}

// -----------------------------------------------------------------------------

cool::csabase::RegisterCheck::RegisterCheck(std::string const& name, cool::csabase::CheckRegistry::Subscriber subscriber)
{
    cool::csabase::CheckRegistry::add_check(name, subscriber);
}

// -----------------------------------------------------------------------------

#define REGISTER(D)                                                     \
namespace                                                               \
{                                                                       \
    template <>                                                         \
        struct map_to_member<clang::D>                                  \
    {                                                                   \
        static cool::event<void(clang::D const*)> (cool::csabase::Visitor::* member()); \
    };                                                                  \
    cool::event<void(clang::D const*)>                                  \
        (cool::csabase::Visitor::* map_to_member<clang::D>::member())          \
    {                                                                   \
        return &cool::csabase::Visitor::on##D;                                   \
    }                                                                   \
}                                                                       \
 template cool::csabase::RegisterCheck::RegisterCheck(std::string const&, void (*)(cool::csabase::Analyser&, clang::D const*));

#define ABSTRACT_DECL(ARG) ARG
#define DECL(CLASS, BASE) REGISTER(CLASS##Decl)
DECL(,)
#include "clang/AST/DeclNodes.inc"

#define ABSTRACT_STMT(STMT)
#define STMT(CLASS, PARENT) REGISTER(CLASS)
STMT(Stmt,)
#include "clang/AST/StmtNodes.inc"
REGISTER(Expr)
