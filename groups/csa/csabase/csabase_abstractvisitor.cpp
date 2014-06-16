// csabase_abstractvisitor.h                                          -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_abstractvisitor.h>
#include <csabase_debug.h>
#include <csabase_format.h>
#include <llvm/Support/raw_ostream.h>
#include <algorithm>
#include <functional>
#ident "$Id$"

using namespace csabase;
using namespace clang;

// -----------------------------------------------------------------------------

csabase::AbstractVisitor::~AbstractVisitor()
{
}

// -----------------------------------------------------------------------------
// Entry points to visit the AST.

void csabase::AbstractVisitor::visit_decl(Decl const* decl)
{
    DeclVisitor<csabase::AbstractVisitor>::Visit(const_cast<Decl*>(decl));
}

void csabase::AbstractVisitor::visit(Decl const* decl)
{
    visit_decl(decl);
}

void csabase::AbstractVisitor::visit_stmt(Stmt const* stmt)
{
    if (stmt) {
        StmtVisitor<csabase::AbstractVisitor>::Visit(const_cast<Stmt*>(stmt));
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
    csabase::Debug d1("process DeclContext");
    std::for_each(
        context->decls_begin(),
        context->decls_end(),
        std::bind1st(
            std::mem_fun(&csabase::AbstractVisitor::visit_decl), this));
}

// -----------------------------------------------------------------------------

template <typename DECL>
static void process_decl(csabase::AbstractVisitor* visitor, DECL *decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(csabase::AbstractVisitor* visitor, NamespaceDecl* decl)
{
    visitor->visit_context(decl);
}

// -----------------------------------------------------------------------------

static void
process_decl(csabase::AbstractVisitor* visitor, VarDecl* decl)
{
    if (Expr* init = decl->getInit())
    {
        visitor->visit(init);
    }
}

// -----------------------------------------------------------------------------

static void
process_decl(csabase::AbstractVisitor* visitor, FunctionDecl* decl)
{
    csabase::Debug d1("process definition FunctionDecl");
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
process_decl(csabase::AbstractVisitor* visitor, TagDecl* decl)
{
    visitor->visit_context(decl);
}

// -----------------------------------------------------------------------------

static void
process_decl(csabase::AbstractVisitor* visitor, BlockDecl* decl)
{
    visitor->visit_context(decl);
}

// -----------------------------------------------------------------------------

static void
process_decl(csabase::AbstractVisitor* visitor, CXXRecordDecl* decl)
{
    csabase::Debug d("process_decl(..., CXXRecordDecl)");
}

// -----------------------------------------------------------------------------

static void
process_decl(csabase::AbstractVisitor* visitor, LinkageSpecDecl* decl)
{
    if (decl->hasBraces())
    {
        visitor->visit_context(decl);
    }
}

static void
process_decl(csabase::AbstractVisitor* visitor, ClassTemplateDecl* decl)
{
    csabase::Debug d("ClassTemplateDecl");
    visitor->visit(decl->getTemplatedDecl());
}

// -----------------------------------------------------------------------------

void
csabase::AbstractVisitor::visit_children(StmtRange const& range)
{
    std::for_each(
        range.first,
        range.second,
        std::bind1st(
            std::mem_fun(&csabase::AbstractVisitor::visit_stmt), this));
}

template <typename Children>
void csabase::AbstractVisitor::visit_children(Children const& children)
{
    std::for_each(
        children.begin(),
        children.end(),
        std::bind1st(
            std::mem_fun(&csabase::AbstractVisitor::visit_stmt), this));
}

// -----------------------------------------------------------------------------

template <typename Statement>
static void
process_stmt(csabase::AbstractVisitor* visitor, Statement* stmt)
{
}

// -----------------------------------------------------------------------------

static void process_stmt(csabase::AbstractVisitor* visitor, DeclStmt* stmt)
{
    std::for_each(
        stmt->decl_begin(),
        stmt->decl_end(),
        std::bind1st(
            std::mem_fun(&csabase::AbstractVisitor::visit_decl), visitor));
}

// -----------------------------------------------------------------------------

void csabase::AbstractVisitor::do_visit(Decl const*)
{
}

void csabase::AbstractVisitor::process_decl(Decl* decl, bool nest)
{
    csabase::Debug d("Decl", nest);
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
        csabase::Debug d(#CLASS "Decl (gen)", nest);                          \
        do_visit(decl);                                                       \
        ::process_decl(this, decl);                                           \
        process_decl(static_cast<BASE*>(decl), false);                        \
    }                                                                         \
    void csabase::AbstractVisitor::Visit##CLASS##Decl(CLASS##Decl* decl)      \
    {                                                                         \
        process_decl(decl, true);                                             \
    }
#include "clang/AST/DeclNodes.inc"

// -----------------------------------------------------------------------------

void csabase::AbstractVisitor::do_visit(Stmt const*)
{
}

void csabase::AbstractVisitor::process_stmt(Stmt* stmt, bool nest)
{
    csabase::Debug d("Stmt", nest);
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
        csabase::Debug d(#CLASS " (stmt)", nest);                       \
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
#include "clang/AST/StmtNodes.inc"
