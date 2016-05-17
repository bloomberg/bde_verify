// csabase_abstractvisitor.h                                          -*-C++-*-

#ifndef INCLUDED_CSABASE_ABSTRACTVISITOR
#define INCLUDED_CSABASE_ABSTRACTVISITOR

#include <clang/AST/DeclVisitor.h>
#include <clang/AST/StmtVisitor.h>

namespace clang { class Decl; }
namespace clang { class DeclContext; }
namespace clang { class Stmt; }
namespace clang { struct StmtRange; }

// -----------------------------------------------------------------------------

namespace csabase
{
class AbstractVisitor:
    public clang::DeclVisitor<AbstractVisitor>,
    public clang::StmtVisitor<AbstractVisitor>
{
public:
    virtual ~AbstractVisitor();

    void visit(clang::Decl const*);
    void visit(clang::Stmt const*);

#define DECL(CLASS, BASE)                                               \
    virtual void do_visit(clang::CLASS##Decl const*);                   \
    void process_decl(clang::CLASS##Decl*, bool nest = false);          \
    void Visit##CLASS##Decl (clang::CLASS##Decl*);
DECL(,void)
#include "clang/AST/DeclNodes.inc"  // IWYU pragma: keep

#define STMT(CLASS, BASE)                                      \
    virtual void do_visit(clang::CLASS const*);                \
    void process_stmt(clang::CLASS*, bool nest = false);       \
    void Visit##CLASS(clang::CLASS*);
STMT(Stmt,void)
#include "clang/AST/StmtNodes.inc"  // IWYU pragma: keep

    void visit_decl(clang::Decl const*);
    void visit_stmt(clang::Stmt const*);
    void visit_context(void const*);
    void visit_context(clang::DeclContext const*);
    void visit_children(clang::Stmt::child_range const&);
    template <typename Children> void visit_children(Children const&);
};
}

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
