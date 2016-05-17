// csabase_registercheck.cpp                                          -*-C++-*-

#include <csabase_registercheck.h>
#include <csabase_binder.h>
#include <csabase_checkregistry.h>
#include <csabase_visitor.h>

namespace clang { class Decl; }
namespace clang { class Expr; }
namespace clang { class Stmt; }
namespace utils { template <typename Signature> class event; }

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

namespace csabase
{
#define REGISTER(D)                                                  \
    template <>                                                      \
    RegisterCheck::RegisterCheck(                                    \
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

RegisterCheck::RegisterCheck(
    std::string const & name, CheckRegistry::Subscriber subscriber)
{
    CheckRegistry::add_check(name, subscriber);
}

}

// ----------------------------------------------------------------------------
// Copyright (C) 2014 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
