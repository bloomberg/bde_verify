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

// -----------------------------------------------------------------------------

cool::csabase::AbstractVisitor::~AbstractVisitor()
{
}

// -----------------------------------------------------------------------------
// Entry points to visit the AST.

void
cool::csabase::AbstractVisitor::visit_decl(clang::Decl const* decl)
{
    this->clang::DeclVisitor<cool::csabase::AbstractVisitor>::Visit(const_cast<clang::Decl*>(decl));
}

void
cool::csabase::AbstractVisitor::visit(clang::Decl const* decl)
{
    this->visit_decl(decl);
}

void
cool::csabase::AbstractVisitor::visit_stmt(clang::Stmt const* stmt)
{
    if (stmt)
    {
        this->clang::StmtVisitor<cool::csabase::AbstractVisitor>::Visit(const_cast<clang::Stmt*>(stmt));
    }
}

void
cool::csabase::AbstractVisitor::visit(clang::Stmt const* stmt)
{
    this->visit_stmt(stmt);
}

// -----------------------------------------------------------------------------

void
cool::csabase::AbstractVisitor::visit_context(void const*)
{
}

void
cool::csabase::AbstractVisitor::visit_context(clang::DeclContext const* context)
{
    cool::csabase::Debug d1("process DeclContext");
    std::for_each(context->decls_begin(), context->decls_end(),
                  std::bind1st(std::mem_fun(&cool::csabase::AbstractVisitor::visit_decl), this));
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::TranslationUnitDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::NamedDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::LabelDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::NamespaceDecl* decl)
{
    visitor->visit_context(decl);
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::ValueDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::DeclaratorDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::VarDecl* decl)
{
    if (clang::Expr* init = decl->getInit())
    {
        visitor->visit(init);
    }
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::ImplicitParamDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::ParmVarDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::FunctionDecl* decl)
{
    //-dk:TODO deal with parameter
    //-dk:TODO deal context
    cool::csabase::Debug d1("process definition");
    if (decl->isThisDeclarationADefinition())
    {
        visitor->visit(decl->getBody());
    }
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::FieldDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::EnumConstantDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::IndirectFieldDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::TypeDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::TypedefDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::TagDecl* decl)
{
#if 0
    cool::csabase::Debug d("process_decl(..., TagDecl*)");
    d << "name=" << decl->getNameAsString() << " "
      << "is-def=" << decl->isThisDeclarationADefinition() << " "
      << "is-comp-def=" << decl->isCompleteDefinition() << " "
      << "is-being-def=" << decl->isBeingDefined() << " "
      << "is-embedded=" << decl->isEmbeddedInDeclarator() << " "
      << "is-freestanding=" << decl->isFreeStanding() << " "
      << "is-dependent=" << decl->isDependentType() << " "
      << "\n";
#endif
#if 0 //-dk:TODO
    if (decl->isCompleteDefinition())
#endif
    {
        visitor->visit_context(decl);
    }
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::EnumDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::RecordDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::FileScopeAsmDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::BlockDecl* decl)
{
    visitor->visit_context(decl);
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::AccessSpecDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::CXXRecordDecl* decl)
{
    cool::csabase::Debug d("process_decl(..., CXXRecordDecl)");
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::CXXMethodDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::CXXConstructorDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::CXXDestructorDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::CXXConversionDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::LinkageSpecDecl* decl)
{
    if (decl->hasBraces())
    {
        visitor->visit_context(decl);
    }
    else
    {
        //-dk:TODO visitor->Visit(*decl->decls_begin());
    }
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::NamespaceAliasDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::UsingShadowDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::UsingDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::UnresolvedUsingTypenameDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor* visitor, clang::StaticAssertDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(cool::csabase::AbstractVisitor*, clang::ObjCPropertyImplDecl*)
{
    cool::csabase::Debug d("TODO ObjCPropertyImplDecl");
}

#if 0
static void
process_decl(cool::csabase::AbstractVisitor*, clang::ObjCForwardProtocolDecl*)
{
    cool::csabase::Debug d("TODO ObjCForwardProtocolDecl");
}
#endif

#if 0
static void
process_decl(cool::csabase::AbstractVisitor*, clang::ObjCClassDecl*)
{
    cool::csabase::Debug d("TODO ObjCClassDecl");
}
#endif

#if 0
static void
process_decl(cool::csabase::AbstractVisitor*, clang::ImportDecl*)
{
    cool::csabase::Debug d("TODO ImportDecl");
}
#endif

static void
process_decl(cool::csabase::AbstractVisitor*, clang::FriendTemplateDecl*)
{
    cool::csabase::Debug d("TODO FriendTemplateDecl");
}

static void
process_decl(cool::csabase::AbstractVisitor*, clang::FriendDecl*)
{
    cool::csabase::Debug d("TODO FriendDecl");
}

#if 0
static void
process_decl(cool::csabase::AbstractVisitor*, clang::ClassScopeFunctionSpecializationDecl*)
{
    cool::csabase::Debug d("TODO ClassScopeFunctionSpecializationDecl");
}
#endif

static void
process_decl(cool::csabase::AbstractVisitor*, clang::Decl* decl)
{
    cool::csabase::Debug d("TODO process_decl(..., Decl)");
    d << "kind: " << cool::csabase::format(decl->getKind()) << "\n";
}

// -----------------------------------------------------------------------------

#if 0
template <typename Declaration>
static void
process_decl(cool::csabase::AbstractVisitor* visitor, Declaration* decl)
{
    cool::csabase::Debug d("process_decl(..., Declaration)");
#if 0
    if (decl->hasBody())
    {
        visitor->visit(decl->getBody());
    }
    visitor->visit_context(decl);
#endif
}
#endif

// -----------------------------------------------------------------------------

void
cool::csabase::AbstractVisitor::visit_children(clang::StmtRange const& range)
{
    std::for_each(range.first, range.second,
                  std::bind1st(std::mem_fun(&cool::csabase::AbstractVisitor::visit_stmt), this));
}

template <typename Children>
void
cool::csabase::AbstractVisitor::visit_children(Children const& children)
{
    std::for_each(children.begin(), children.end(),
                  std::bind1st(std::mem_fun(&cool::csabase::AbstractVisitor::visit_stmt), this));
}

// -----------------------------------------------------------------------------

static void
process_stmt(cool::csabase::AbstractVisitor* visitor, clang::DeclStmt* stmt)
{
    std::for_each(stmt->decl_begin(), stmt->decl_end(),
                  std::bind1st(std::mem_fun(&cool::csabase::AbstractVisitor::visit_decl), visitor));
}


// -----------------------------------------------------------------------------

template <typename Statement>
static void
process_stmt(cool::csabase::AbstractVisitor* visitor, Statement* stmt)
{
    // visitor->visit_children(stmt->children());
}

// -----------------------------------------------------------------------------

void
cool::csabase::AbstractVisitor::do_visit(clang::Decl const*)
{
}

void
cool::csabase::AbstractVisitor::process_decl(clang::Decl* decl, bool nest)
{
    cool::csabase::Debug d("Decl", nest);
    this->do_visit(decl);
    ::process_decl(this, decl);
}

void
cool::csabase::AbstractVisitor::VisitDecl (clang::Decl* decl)
{
    this->process_decl(decl, true);
}

// -----------------------------------------------------------------------------

#define DECL(CLASS, BASE)                                                         \
void                                                                              \
cool::csabase::AbstractVisitor::do_visit(clang::CLASS##Decl const*)               \
{                                                                                 \
}                                                                                 \
void                                                                              \
cool::csabase::AbstractVisitor::process_decl(clang::CLASS##Decl* decl, bool nest) \
{                                                                                 \
    cool::csabase::Debug d(#CLASS "Decl (gen)", nest);                            \
    this->do_visit(decl);                                                         \
    ::process_decl(this, decl);                                                   \
    this->process_decl(static_cast<clang::BASE *>(decl), false);                  \
}                                                                                 \
void                                                                              \
cool::csabase::AbstractVisitor::Visit##CLASS##Decl (clang::CLASS##Decl* decl)     \
{                                                                                 \
    this->process_decl(decl, true);                                               \
}
#include "clang/AST/DeclNodes.inc"

// -----------------------------------------------------------------------------

void
cool::csabase::AbstractVisitor::do_visit(clang::Stmt const*)
{
}

void
cool::csabase::AbstractVisitor::process_stmt(clang::Stmt* stmt, bool nest)
{
    cool::csabase::Debug d("Stmt", nest);
    this->do_visit(stmt);
    ::process_stmt(this, stmt);
}

void
cool::csabase::AbstractVisitor::VisitStmt(clang::Stmt* stmt)
{
    this->process_stmt(stmt, true);
}

// -----------------------------------------------------------------------------

#define STMT(CLASS, BASE)                                                   \
void                                                                        \
cool::csabase::AbstractVisitor::do_visit(clang::CLASS const*)               \
{                                                                           \
}                                                                           \
void                                                                        \
cool::csabase::AbstractVisitor::process_stmt(clang::CLASS* stmt, bool nest) \
{                                                                           \
    cool::csabase::Debug d(#CLASS " (stmt)", nest);                         \
    this->do_visit(stmt);                                                   \
    if (nest) { this->visit_children(stmt->children()); }                   \
    ::process_stmt(this, stmt);                                             \
    this->process_stmt(static_cast<clang::BASE*>(stmt), false);             \
}                                                                           \
void                                                                        \
cool::csabase::AbstractVisitor::Visit##CLASS(clang::CLASS* stmt)            \
{                                                                           \
    this->process_stmt(stmt, true);                                         \
}
#include "clang/AST/StmtNodes.inc"
