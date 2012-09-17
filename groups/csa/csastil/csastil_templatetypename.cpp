// csastil_templatetypename.cpp                                       -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_registercheck.h>
#include <csabase_cast_ptr.h>

// ----------------------------------------------------------------------------

static std::string const check_name("template-typename");

// ----------------------------------------------------------------------------

static void
checkTemplate(cool::csabase::Analyser& analyser,
              clang::TemplateDecl const* decl)
{
    if (decl) {
        clang::TemplateParameterList const* parms(decl->getTemplateParameters());
        typedef clang::TemplateParameterList::const_iterator const_iterator;
        for (const_iterator it(parms->begin()), end(parms->end()); it != end; ++it) {
            cool::csabase::cast_ptr<clang::TemplateTypeParmDecl> parm(*it);
            if (parm && parm->wasDeclaredWithTypename()) {
                analyser.report(decl, ::check_name,
                                "template parameter uses typename");
            }
        }
    }
}

// ----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_check(check_name, &::checkTemplate);
