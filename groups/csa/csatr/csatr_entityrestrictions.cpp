// csatr_entityrestrictions.cpp                                       -*-C++-*-

#include <csabase_registercheck.h>
#include <csabase_analyser.h>
#include <llvm/Support/raw_ostream.h>

// ----------------------------------------------------------------------------

static std::string const check_name("entity-restrictions");

// ----------------------------------------------------------------------------

static void
enum_declaration(bde_verify::csabase::Analyser&  analyser,
                 clang::EnumDecl const    *decl)
{
    if (llvm::dyn_cast<clang::NamespaceDecl>(decl->getDeclContext())
        && analyser.is_component_header(decl)
        && !analyser.is_standard_namespace(decl->getQualifiedNameAsString())) {
        analyser.report(decl, check_name, "TR17",
                        "Enum '%0' declared at global scope")
            << decl->getName();
    }
}

// ----------------------------------------------------------------------------

static void
var_declaration(bde_verify::csabase::Analyser&  analyser,
                clang::VarDecl const     *decl)
{
    if (llvm::dyn_cast<clang::NamespaceDecl>(decl->getDeclContext())
        && analyser.is_component_header(decl)
        && !analyser.is_standard_namespace(decl->getQualifiedNameAsString())) {
        analyser.report(decl, check_name, "TR17",
                        "Variable '%0' declared at global scope")
            << decl->getName();
    }
}

// ----------------------------------------------------------------------------

static bool
is_swap(clang::FunctionDecl const* decl)
{
    return decl->getNameAsString() == "swap"
        && decl->getNumParams() == 2
        && decl->getParamDecl(0)->getType()->getCanonicalTypeInternal() ==
           decl->getParamDecl(1)->getType()->getCanonicalTypeInternal()
        && decl->getParamDecl(0)->getType().getTypePtr()->isReferenceType()
        && !decl->getParamDecl(0)->getType().getNonReferenceType()
                .isConstQualified();
}

static void
function_declaration(bde_verify::csabase::Analyser&   analyser,
                     clang::FunctionDecl const *decl)
{
    if (llvm::dyn_cast<clang::NamespaceDecl>(decl->getDeclContext())
        && analyser.is_component_header(decl)
        && !decl->isOverloadedOperator()
        && !is_swap(decl)
        && decl->getNameAsString() != "debugprint"
        && decl->isFirstDecl()
        && !analyser.is_ADL_candidate(decl)
        && !analyser.is_standard_namespace(decl->getQualifiedNameAsString())
        ) {
        analyser.report(decl, check_name, "TR17",
                        "Function '%0' declared at global scope")
            << decl->getNameAsString()
            << decl->getNameInfo().getSourceRange();
    }
}

// -----------------------------------------------------------------------------

static void
typedef_declaration(bde_verify::csabase::Analyser&  analyser,
                    clang::TypedefDecl const *decl)
{
    // Allow global scope typedef to a name that begins with "package_" for
    // legacy support of "typedef package::name package_name;".
    std::string package = analyser.package() + "_";
    if (package.find("bslfwd_") == 0) {
        package = package.substr(7);
    }
    if (llvm::dyn_cast<clang::NamespaceDecl>(decl->getDeclContext())
        && analyser.is_component_header(decl)
        && decl->getNameAsString().find(package) != 0
        && !analyser.is_standard_namespace(decl->getQualifiedNameAsString())
        ) {
        analyser.report(decl, check_name, "TR17",
                        "Typedef '%0' declared at global scope")
            << decl->getNameAsString();
    }
}

// -----------------------------------------------------------------------------

static bde_verify::csabase::RegisterCheck c0(check_name, &enum_declaration);
static bde_verify::csabase::RegisterCheck c1(check_name, &var_declaration);
static bde_verify::csabase::RegisterCheck c2(check_name, &function_declaration);
static bde_verify::csabase::RegisterCheck c3(check_name, &typedef_declaration);
