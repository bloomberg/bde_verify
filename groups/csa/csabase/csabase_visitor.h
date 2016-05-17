// csabase_visitor.h                                                  -*-C++-*-

#ifndef INCLUDED_CSABASE_VISITOR
#define INCLUDED_CSABASE_VISITOR

#include <csabase_abstractvisitor.h>
#include <utils/event.hpp>

namespace clang { class Decl; }
namespace clang { class Stmt; }

// -----------------------------------------------------------------------------

namespace csabase
{
class Visitor : public AbstractVisitor
{
public:
#define DECL(CLASS, BASE)                                                     \
    utils::event<void(clang::CLASS##Decl const*)> on##CLASS##Decl;            \
    void do_visit(clang::CLASS##Decl const*);
DECL(,)
#include "clang/AST/DeclNodes.inc"  // IWYU pragma: keep

#define STMT(CLASS, PARENT)                                                   \
    utils::event<void(clang::CLASS const*)> on##CLASS;                        \
    void do_visit(clang::CLASS const*);
STMT(Stmt,)
#include "clang/AST/StmtNodes.inc"  // IWYU pragma: keep
};
}

// -----------------------------------------------------------------------------

#endif

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
