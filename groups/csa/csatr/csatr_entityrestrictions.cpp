// csatr_entityrestrictions.cpp                                       -*-C++-*-

#include <csabase_registercheck.h>
#include <csabase_analyser.h>
#include <llvm/Support/raw_ostream.h>

// ----------------------------------------------------------------------------

static std::string const check_name("entity-restrictions");

// ----------------------------------------------------------------------------

static void
enum_declaration(cool::csabase::Analyser&  analyser,
                 clang::EnumDecl const    *decl)
{
    if (llvm::dyn_cast<clang::NamespaceDecl>(decl->getDeclContext())
        && analyser.is_component_header(decl)) {
        analyser.report(decl, ::check_name,
                        "TR17: enum '%0' declared at global scope")
            << decl->getName();
    }
}

// ----------------------------------------------------------------------------

static void
var_declaration(cool::csabase::Analyser&  analyser,
                clang::VarDecl const     *decl)
{
    if (llvm::dyn_cast<clang::NamespaceDecl>(decl->getDeclContext())
        && analyser.is_component_header(decl)) {
        analyser.report(decl, ::check_name,
                        "TR17: variable '%0' declared at global scope")
            << decl->getName();
    }
}

// ----------------------------------------------------------------------------

static bool
is_swap(clang::FunctionDecl const* decl)
{
    return decl->getNameAsString() == "swap"
        && decl->getNumParams() == 2
        && decl->getParamDecl(0)->getType() == decl->getParamDecl(1)->getType()
        && decl->getParamDecl(0)->getType().getTypePtr()->isReferenceType()
        && !decl->getParamDecl(0)->getType().getNonReferenceType().isConstQualified();
}

static void
function_declaration(cool::csabase::Analyser&   analyser,
                     clang::FunctionDecl const *decl)
{
    if (llvm::dyn_cast<clang::NamespaceDecl>(decl->getDeclContext())
        && analyser.is_component_header(decl)
        && !decl->isOverloadedOperator()
        && !::is_swap(decl)
        && decl->getNameAsString() != "debugprint"
        && decl->isFirstDeclaration()
        ) {
        analyser.report(decl, ::check_name,
                        "TR17: function '%0' declared at global scope")
            << decl->getNameAsString();
    }
}

// -----------------------------------------------------------------------------

static void
typedef_declaration(cool::csabase::Analyser&  analyser,
                    clang::TypedefDecl const *decl)
{
    if (llvm::dyn_cast<clang::NamespaceDecl>(decl->getDeclContext())
        && analyser.is_component_header(decl)
        ) {
        analyser.report(decl, ::check_name,
                        "TR17: typedef '%0' declared at global scope")
            << decl->getNameAsString();
    }
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck c0(check_name, &::enum_declaration);
static cool::csabase::RegisterCheck c1(check_name, &::var_declaration);
static cool::csabase::RegisterCheck c2(check_name, &::function_declaration);
static cool::csabase::RegisterCheck c3(check_name, &::typedef_declaration);
