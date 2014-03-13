// csabbg_enumvalue.cpp                                               -*-C++-*-

#include <csabase_registercheck.h>
#include <csabase_analyser.h>
#include <llvm/Support/raw_ostream.h>

// ----------------------------------------------------------------------------

static std::string const check_name("enum-value");

// ----------------------------------------------------------------------------

static void
enum_value(bde_verify::csabase::Analyser& analyser, clang::EnumDecl const *decl)
{
    if (analyser.is_component(decl) && decl->getNameAsString() == "Value") {
        analyser.report(decl, check_name, "EV01",
                        "Do not use 'Value' as enum name")
            << decl->getDeclName();
    }
}

// ----------------------------------------------------------------------------

static bde_verify::csabase::RegisterCheck c0(check_name, &enum_value);
