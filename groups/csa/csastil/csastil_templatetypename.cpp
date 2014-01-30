// csastil_templatetypename.cpp                                       -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_registercheck.h>
#include <csabase_cast_ptr.h>
#include <llvm/Support/Regex.h>

// ----------------------------------------------------------------------------

static std::string const check_name("template-typename");

// ----------------------------------------------------------------------------

static llvm::Regex all_upper("^[[:upper:]](_?[[:upper:][:digit:]]+)*$");

static void
checkTemplate(cool::csabase::Analyser& analyser,
              clang::TemplateDecl const* decl)
{
    clang::TemplateParameterList const* parms = decl->getTemplateParameters();
    unsigned n = parms->size();
    for (unsigned i = 0; i < n; ++i) {
        const clang::TemplateTypeParmDecl *parm =
            llvm::dyn_cast<clang::TemplateTypeParmDecl>(parms->getParam(i));
        if (parm) {
            if (parm->wasDeclaredWithTypename()) {
                 analyser.report(parm, check_name, "TY01",
                     "Template parameter uses 'typename' instead of 'class'");
            }
            if (parm->getName().size() == 1) {
                 analyser.report(parm, check_name, "TY02",
                     "Template parameter uses single-letter name");
            }
            if (!all_upper.match(parm->getName())) {
                 analyser.report(parm, check_name, "TY03",
                     "Template parameter is not in ALL_CAPS format");
            }
        }
    }
}

// ----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_check(check_name, &checkTemplate);
