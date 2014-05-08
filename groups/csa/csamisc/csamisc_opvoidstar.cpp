// csastil_opvoidstar.cpp                                             -*-C++-*-
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_registercheck.h>

#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("operator-void-star");

// ----------------------------------------------------------------------------

using clang::CXXConversionDecl;
using clang::QualType;

using csabase::Analyser;
using csabase::Location;

namespace
{

void conversions(Analyser& analyser, const CXXConversionDecl* conv)
    // Callback function for inspecting conversion declarations.
{
    Location loc(analyser.get_location(conv->getLocStart()));
    if (analyser.is_component(loc.file()))
    {
        QualType type = conv->getConversionType();
        if (   !conv->isExplicit()
            && (   type->isBooleanType()
                || (   type->isPointerType()
                    && type->getPointeeType()->isVoidType()
                    )
                )
            ) {
            analyser.report(conv, check_name, "CB01",
                            "Consider using conversion to "
                            "bsls::UnspecifiedBool<%0>::BoolType instead")
                << conv->getParent()->getNameAsString();
        }
    }
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static csabase::RegisterCheck c1(check_name, &conversions);
