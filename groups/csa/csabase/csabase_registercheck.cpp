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
        binder(csabase::Analyser& a, void (*check)(csabase::Analyser&, T const*));
        void operator()(T const* argument);

        csabase::Analyser& analyser_;
        void            (*check_)(csabase::Analyser&, T const*);
    };
}

template <typename T>
::binder<T>::binder(csabase::Analyser& a, void (*check)(csabase::Analyser&, T const*)):
    analyser_(a),
    check_(check)
{
}

template <typename T>
void
::binder<T>::operator()(T const* argument)
{
    check_(analyser_, argument);
}

// -----------------------------------------------------------------------------

namespace
{
    template <typename T>
    struct add_to_event
    {
        add_to_event(utils::event<void(T const*)> csabase::Visitor::*member, void (*check)(csabase::Analyser&, T const*));
        void operator()(csabase::Analyser& a, csabase::Visitor& v, csabase::PPObserver&);

        utils::event<void(T const*)> csabase::Visitor::*member_;
        void                        (*check_)(csabase::Analyser&, T const*);
    };
}

template <typename T>
::add_to_event<T>::add_to_event(utils::event<void(T const*)> csabase::Visitor::*member, void (*check)(csabase::Analyser&, T const*)):
    member_(member),
    check_(check)
{
}

template <typename T>
void
::add_to_event<T>::operator()(csabase::Analyser& a, csabase::Visitor& v, csabase::PPObserver&)
{
    v.*(member_) += binder<T>(a, check_);
}

// -----------------------------------------------------------------------------

namespace
{
    template <typename> struct map_to_member;
}

// -----------------------------------------------------------------------------

template <typename T>
csabase::RegisterCheck::RegisterCheck(std::string const& name, void (*check)(csabase::Analyser&, T const*))
{
    csabase::CheckRegistry::add_check(name, add_to_event<T>(map_to_member<T>::member(), check));
}

// -----------------------------------------------------------------------------

csabase::RegisterCheck::RegisterCheck(std::string const& name, csabase::CheckRegistry::Subscriber subscriber)
{
    csabase::CheckRegistry::add_check(name, subscriber);
}

// -----------------------------------------------------------------------------

#define REGISTER(D)                                                     \
namespace                                                               \
{                                                                       \
    template <>                                                         \
        struct map_to_member<clang::D>                                  \
    {                                                                   \
        static utils::event<void(clang::D const*)> (csabase::Visitor::* member()); \
    };                                                                  \
    utils::event<void(clang::D const*)>                                  \
        (csabase::Visitor::* map_to_member<clang::D>::member())          \
    {                                                                   \
        return &csabase::Visitor::on##D;                                   \
    }                                                                   \
}                                                                       \
 template csabase::RegisterCheck::RegisterCheck(std::string const&, void (*)(csabase::Analyser&, clang::D const*));

#define ABSTRACT_DECL(ARG) ARG
#define DECL(CLASS, BASE) REGISTER(CLASS##Decl)
DECL(,)
#include "clang/AST/DeclNodes.inc"

#define ABSTRACT_STMT(STMT)
#define STMT(CLASS, PARENT) REGISTER(CLASS)
STMT(Stmt,)
#include "clang/AST/StmtNodes.inc"
REGISTER(Expr)
