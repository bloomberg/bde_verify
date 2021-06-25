// csabase_abstractvisitor.cpp                                        -*-C++-*-

#include <csabase_abstractvisitor.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclFriend.h>
#include <clang/AST/DeclObjC.h>
#include <clang/AST/DeclOpenMP.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <clang/AST/ExprObjC.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/StmtCXX.h>
#include <clang/AST/StmtIterator.h>
#include <clang/AST/StmtObjC.h>
#include <clang/AST/StmtOpenMP.h>
#include <csabase_debug.h>
#include <algorithm>
#include <functional>

using namespace csabase;
using namespace clang;

using namespace std::placeholders;

// -----------------------------------------------------------------------------

csabase::AbstractVisitor::~AbstractVisitor()
{
}

// -----------------------------------------------------------------------------
// Entry points to visit the AST.

void csabase::AbstractVisitor::visit_decl(Decl const* decl)
{
    DeclVisitor<AbstractVisitor>::Visit(const_cast<Decl*>(decl));
}

void csabase::AbstractVisitor::visit(Decl const* decl)
{
    visit_decl(decl);
}

void csabase::AbstractVisitor::visit_stmt(Stmt const* stmt)
{
    if (stmt) {
        StmtVisitor<AbstractVisitor>::Visit(const_cast<Stmt*>(stmt));
    }
}

void csabase::AbstractVisitor::visit(Stmt const* stmt)
{
    visit_stmt(stmt);
}

// -----------------------------------------------------------------------------

void csabase::AbstractVisitor::visit_context(void const*)
{
}

void csabase::AbstractVisitor::visit_context(DeclContext const* context)
{
    Debug d1("process DeclContext");
    std::for_each(
        context->decls_begin(),
        context->decls_end(),
        std::bind(&AbstractVisitor::visit_decl, this, _1));
}

// -----------------------------------------------------------------------------

template <typename DECL>
static void process_decl(AbstractVisitor* visitor, DECL *decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(AbstractVisitor* visitor, NamespaceDecl* decl)
{
    visitor->visit_context(decl);
}

// -----------------------------------------------------------------------------

static void
process_decl(AbstractVisitor* visitor, VarDecl* decl)
{
    if (Expr* init = decl->getInit())
    {
        visitor->visit(init);
    }
}

// -----------------------------------------------------------------------------

static void
process_decl(AbstractVisitor* visitor, FunctionDecl* decl)
{
    Debug d1("process definition FunctionDecl");
    if (decl->getDescribedFunctionTemplate()) {
        visitor->visit(decl->getDescribedFunctionTemplate());
    }

    // Note that due to the potential use of '-fdelayed-template-parsing' in
    // Windows, it is possible for 'decl->hasBody()' to return 'true' but for
    // 'decl->getBody()' to return a null pointer.
    if (decl->isThisDeclarationADefinition() && decl->getBody())
    {
        visitor->visit(decl->getBody());
    }
}

// -----------------------------------------------------------------------------

static void
process_decl(AbstractVisitor* visitor, TagDecl* decl)
{
    visitor->visit_context(decl);
}

// -----------------------------------------------------------------------------

static void
process_decl(AbstractVisitor* visitor, BlockDecl* decl)
{
    visitor->visit_context(decl);
}

// -----------------------------------------------------------------------------

static void
process_decl(AbstractVisitor* visitor, CXXRecordDecl* decl)
{
    Debug d("process_decl(..., CXXRecordDecl)");
}

// -----------------------------------------------------------------------------

static void
process_decl(AbstractVisitor* visitor, LinkageSpecDecl* decl)
{
    if (decl->hasBraces())
    {
        visitor->visit_context(decl);
    }
}

static void
process_decl(AbstractVisitor* visitor, ClassTemplateDecl* decl)
{
    Debug d("ClassTemplateDecl");
    visitor->visit(decl->getTemplatedDecl());
}

// -----------------------------------------------------------------------------

void
csabase::AbstractVisitor::visit_children(Stmt::child_range const& range)
{
    std::for_each(
        range.begin(),
        range.end(),
        std::bind(&AbstractVisitor::visit_stmt, this, _1));
}

template <typename Children>
void csabase::AbstractVisitor::visit_children(Children const& children)
{
    std::for_each(
        children.begin(),
        children.end(),
        std::bind(&AbstractVisitor::visit_stmt, this, _1));
}

// -----------------------------------------------------------------------------

template <typename Statement>
static void
process_stmt(AbstractVisitor* visitor, Statement* stmt)
{
}

// -----------------------------------------------------------------------------

static void process_stmt(AbstractVisitor* visitor, DeclStmt* stmt)
{
    std::for_each(
        stmt->decl_begin(),
        stmt->decl_end(),
        std::bind(&AbstractVisitor::visit_decl, visitor, _1));
}

// -----------------------------------------------------------------------------

void csabase::AbstractVisitor::do_visit(Decl const*)
{
}

void csabase::AbstractVisitor::process_decl(Decl* decl, bool nest)
{
    Debug d("Decl", nest);
    do_visit(decl);
    ::process_decl(this, decl);
}

void csabase::AbstractVisitor::VisitDecl(Decl* decl)
{
    process_decl(decl, true);
}

// -----------------------------------------------------------------------------

#define DECL(CLASS, BASE)                                                     \
    void csabase::AbstractVisitor::do_visit(CLASS##Decl const*)               \
    {                                                                         \
    }                                                                         \
    void csabase::AbstractVisitor::process_decl(CLASS##Decl* decl, bool nest) \
    {                                                                         \
        Debug d(#CLASS "Decl (gen)", nest);                                   \
        do_visit(decl);                                                       \
        ::process_decl(this, decl);                                           \
        process_decl(static_cast<BASE*>(decl), false);                        \
    }                                                                         \
    void csabase::AbstractVisitor::Visit##CLASS##Decl(CLASS##Decl* decl)      \
    {                                                                         \
        process_decl(decl, true);                                             \
    }
#include "clang/AST/DeclNodes.inc"  // IWYU pragma: keep

// -----------------------------------------------------------------------------

void csabase::AbstractVisitor::do_visit(Stmt const*)
{
}

void csabase::AbstractVisitor::process_stmt(Stmt* stmt, bool nest)
{
    Debug d("Stmt", nest);
    do_visit(stmt);
    ::process_stmt(this, stmt);
}

void csabase::AbstractVisitor::VisitStmt(Stmt* stmt)
{
    process_stmt(stmt, true);
}

// -----------------------------------------------------------------------------

#define STMT(CLASS, BASE)                                               \
    void csabase::AbstractVisitor::do_visit(CLASS const*)               \
    {                                                                   \
    }                                                                   \
    void csabase::AbstractVisitor::process_stmt(CLASS* stmt, bool nest) \
    {                                                                   \
        Debug d(#CLASS " (stmt)", nest);                                \
        do_visit(stmt);                                                 \
        if (nest) {                                                     \
            visit_children(stmt->children());                           \
        }                                                               \
        ::process_stmt(this, stmt);                                     \
        process_stmt(static_cast<BASE*>(stmt), false);                  \
    }                                                                   \
    void csabase::AbstractVisitor::Visit##CLASS(CLASS* stmt)            \
    {                                                                   \
        process_stmt(stmt, true);                                       \
    }
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
