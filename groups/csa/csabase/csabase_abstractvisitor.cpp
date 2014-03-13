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

bde_verify::csabase::AbstractVisitor::~AbstractVisitor()
{
}

// -----------------------------------------------------------------------------
// Entry points to visit the AST.

void
bde_verify::csabase::AbstractVisitor::visit_decl(clang::Decl const* decl)
{
    clang::DeclVisitor<bde_verify::csabase::AbstractVisitor>::Visit(const_cast<clang::Decl*>(decl));
}

void
bde_verify::csabase::AbstractVisitor::visit(clang::Decl const* decl)
{
    visit_decl(decl);
}

void
bde_verify::csabase::AbstractVisitor::visit_stmt(clang::Stmt const* stmt)
{
    if (stmt)
    {
        clang::StmtVisitor<bde_verify::csabase::AbstractVisitor>::Visit(const_cast<clang::Stmt*>(stmt));
    }
}

void
bde_verify::csabase::AbstractVisitor::visit(clang::Stmt const* stmt)
{
    visit_stmt(stmt);
}

// -----------------------------------------------------------------------------

void
bde_verify::csabase::AbstractVisitor::visit_context(void const*)
{
}

void
bde_verify::csabase::AbstractVisitor::visit_context(clang::DeclContext const* context)
{
    bde_verify::csabase::Debug d1("process DeclContext");
    std::for_each(context->decls_begin(), context->decls_end(),
                  std::bind1st(std::mem_fun(&bde_verify::csabase::AbstractVisitor::visit_decl), this));
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::TranslationUnitDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::NamedDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::LabelDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::NamespaceDecl* decl)
{
    visitor->visit_context(decl);
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::ValueDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::DeclaratorDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::VarDecl* decl)
{
    if (clang::Expr* init = decl->getInit())
    {
        visitor->visit(init);
    }
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::ImplicitParamDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::ParmVarDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::FunctionDecl* decl)
{
    //-dk:TODO deal with parameter
    //-dk:TODO deal context
    bde_verify::csabase::Debug d1("process definition FunctionDecl");
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
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::FieldDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::EnumConstantDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::IndirectFieldDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::TypeDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::TypedefDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::TagDecl* decl)
{
#if 0
    bde_verify::csabase::Debug d("process_decl(..., TagDecl*)");
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
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::EnumDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::RecordDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::FileScopeAsmDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::BlockDecl* decl)
{
    visitor->visit_context(decl);
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::AccessSpecDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::CXXRecordDecl* decl)
{
    bde_verify::csabase::Debug d("process_decl(..., CXXRecordDecl)");
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::CXXMethodDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::CXXConstructorDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::CXXDestructorDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::CXXConversionDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::LinkageSpecDecl* decl)
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
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::NamespaceAliasDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::UsingShadowDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::UsingDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::UnresolvedUsingTypenameDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, clang::StaticAssertDecl* decl)
{
}

// -----------------------------------------------------------------------------

static void
process_decl(bde_verify::csabase::AbstractVisitor*, clang::ObjCPropertyImplDecl*)
{
    bde_verify::csabase::Debug d("TODO ObjCPropertyImplDecl");
}

#if 0
static void
process_decl(bde_verify::csabase::AbstractVisitor*, clang::ObjCForwardProtocolDecl*)
{
    bde_verify::csabase::Debug d("TODO ObjCForwardProtocolDecl");
}
#endif

#if 0
static void
process_decl(bde_verify::csabase::AbstractVisitor*, clang::ObjCClassDecl*)
{
    bde_verify::csabase::Debug d("TODO ObjCClassDecl");
}
#endif

#if 0
static void
process_decl(bde_verify::csabase::AbstractVisitor*, clang::ImportDecl*)
{
    bde_verify::csabase::Debug d("TODO ImportDecl");
}
#endif

static void
process_decl(bde_verify::csabase::AbstractVisitor*, clang::FriendTemplateDecl*)
{
    bde_verify::csabase::Debug d("TODO FriendTemplateDecl");
}

static void
process_decl(bde_verify::csabase::AbstractVisitor*, clang::FriendDecl*)
{
    bde_verify::csabase::Debug d("TODO FriendDecl");
}

#if 0
static void
process_decl(bde_verify::csabase::AbstractVisitor*, clang::ClassScopeFunctionSpecializationDecl*)
{
    bde_verify::csabase::Debug d("TODO ClassScopeFunctionSpecializationDecl");
}
#endif

static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor,
             clang::ClassTemplateDecl* decl)
{
    bde_verify::csabase::Debug d("ClassTemplateDecl");
    visitor->visit(decl->getTemplatedDecl());
}

static void
process_decl(bde_verify::csabase::AbstractVisitor*, clang::Decl* decl)
{
    bde_verify::csabase::Debug d("TODO process_decl(..., Decl)");
    d << "kind: " << bde_verify::csabase::format(decl->getKind()) << "\n";
}

// -----------------------------------------------------------------------------

#if 0
template <typename Declaration>
static void
process_decl(bde_verify::csabase::AbstractVisitor* visitor, Declaration* decl)
{
    bde_verify::csabase::Debug d("process_decl(..., Declaration)");
#if 0
    if (decl->hasBody() && decl->getBody())
    {
        visitor->visit(decl->getBody());
    }
    visitor->visit_context(decl);
#endif
}
#endif

// -----------------------------------------------------------------------------

void
bde_verify::csabase::AbstractVisitor::visit_children(clang::StmtRange const& range)
{
    std::for_each(range.first, range.second,
                  std::bind1st(std::mem_fun(&bde_verify::csabase::AbstractVisitor::visit_stmt), this));
}

template <typename Children>
void
bde_verify::csabase::AbstractVisitor::visit_children(Children const& children)
{
    std::for_each(children.begin(), children.end(),
                  std::bind1st(std::mem_fun(&bde_verify::csabase::AbstractVisitor::visit_stmt), this));
}

// -----------------------------------------------------------------------------

static void
process_stmt(bde_verify::csabase::AbstractVisitor* visitor, clang::DeclStmt* stmt)
{
    std::for_each(stmt->decl_begin(), stmt->decl_end(),
                  std::bind1st(std::mem_fun(&bde_verify::csabase::AbstractVisitor::visit_decl), visitor));
}


// -----------------------------------------------------------------------------

template <typename Statement>
static void
process_stmt(bde_verify::csabase::AbstractVisitor* visitor, Statement* stmt)
{
    // visitor->visit_children(stmt->children());
}

// -----------------------------------------------------------------------------

void
bde_verify::csabase::AbstractVisitor::do_visit(clang::Decl const*)
{
}

void
bde_verify::csabase::AbstractVisitor::process_decl(clang::Decl* decl, bool nest)
{
    bde_verify::csabase::Debug d("Decl", nest);
    do_visit(decl);
    ::process_decl(this, decl);
}

void
bde_verify::csabase::AbstractVisitor::VisitDecl (clang::Decl* decl)
{
    process_decl(decl, true);
}

// -----------------------------------------------------------------------------

#define DECL(CLASS, BASE)                                                     \
void                                                                          \
bde_verify::csabase::AbstractVisitor::do_visit(clang::CLASS##Decl const*)           \
{                                                                             \
}                                                                             \
void                                                                          \
bde_verify::csabase::AbstractVisitor::process_decl(clang::CLASS##Decl* decl,        \
                                             bool nest)                       \
{                                                                             \
    bde_verify::csabase::Debug d(#CLASS "Decl (gen)", nest);                        \
    do_visit(decl);                                                           \
    ::process_decl(this, decl);                                               \
    process_decl(static_cast<clang::BASE *>(decl), false);                    \
}                                                                             \
void                                                                          \
bde_verify::csabase::AbstractVisitor::Visit##CLASS##Decl (clang::CLASS##Decl* decl) \
{                                                                             \
    process_decl(decl, true);                                                 \
}
#include "clang/AST/DeclNodes.inc"

// -----------------------------------------------------------------------------

void
bde_verify::csabase::AbstractVisitor::do_visit(clang::Stmt const*)
{
}

void
bde_verify::csabase::AbstractVisitor::process_stmt(clang::Stmt* stmt, bool nest)
{
    bde_verify::csabase::Debug d("Stmt", nest);
    do_visit(stmt);
    ::process_stmt(this, stmt);
}

void
bde_verify::csabase::AbstractVisitor::VisitStmt(clang::Stmt* stmt)
{
    process_stmt(stmt, true);
}

// -----------------------------------------------------------------------------

#define STMT(CLASS, BASE)                                                     \
void                                                                          \
bde_verify::csabase::AbstractVisitor::do_visit(clang::CLASS const*)                 \
{                                                                             \
}                                                                             \
void                                                                          \
bde_verify::csabase::AbstractVisitor::process_stmt(clang::CLASS* stmt, bool nest)   \
{                                                                             \
    bde_verify::csabase::Debug d(#CLASS " (stmt)", nest);                           \
    do_visit(stmt);                                                           \
    if (nest) { visit_children(stmt->children()); }                           \
    ::process_stmt(this, stmt);                                               \
    process_stmt(static_cast<clang::BASE*>(stmt), false);                     \
}                                                                             \
void                                                                          \
bde_verify::csabase::AbstractVisitor::Visit##CLASS(clang::CLASS* stmt)              \
{                                                                             \
    process_stmt(stmt, true);                                                 \
}
#include "clang/AST/StmtNodes.inc"
