// csabase_visitor.cpp                                                -*-C++-*-

#include <csabase_visitor.h>
#include <csabase_debug.h>
#include <utils/event.hpp>

namespace clang { class Decl; }
namespace clang { class Stmt; }

using namespace csabase;
using namespace clang;

// -----------------------------------------------------------------------------

#define DECL(CLASS, BASE)                                    \
    void csabase::Visitor::do_visit(CLASS##Decl const* decl) \
    {                                                        \
        if (on##CLASS##Decl) {                               \
            Debug d("event on" #CLASS "Decl");               \
            on##CLASS##Decl(decl);                           \
        }                                                    \
    }
DECL(,)
#include "clang/AST/DeclNodes.inc"  // IWYU pragma: keep

#define STMT(CLASS, PARENT)                            \
    void csabase::Visitor::do_visit(CLASS const* stmt) \
    {                                                  \
        on##CLASS(stmt);                               \
    }
STMT(Stmt,)
#include "clang/AST/StmtNodes.inc"  // IWYU pragma: keep

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
