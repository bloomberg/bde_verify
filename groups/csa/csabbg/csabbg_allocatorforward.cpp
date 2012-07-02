// -*-c++-*- checks/allocator_forward.cpp 
// -----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_format.h>
#include <csabase_registercheck.h>
#include <csabase_abstractvisitor.h>
#include <csabase_debug.h>
#include <llvm/Support/raw_ostream.h>
#include <clang/AST/ExprCXX.h>
#include <map>
#ident "$Id: allocator_forward.cpp 161 2011-12-28 00:20:28Z kuehl $"

// -----------------------------------------------------------------------------

static std::string const check_name("allocator-forward");

// -----------------------------------------------------------------------------

namespace
{
    struct allocator_info
    {
        allocator_info():
            bslma_allocator_(0)
        {
        }

        clang::TypeDecl* bslma_allocator_;
        std::map<void const*, bool> uses_allocator_;
    };
}

// -----------------------------------------------------------------------------

static clang::TypeDecl*
get_bslma_allocator(cool::csabase::Analyser& analyser)
{
    ::allocator_info& info(analyser.attachment< ::allocator_info>());
    if (!info.bslma_allocator_)
    {
        info.bslma_allocator_ = analyser.lookup_type("::BloombergLP::bslma_Allocator"); 
    }
    return info.bslma_allocator_;
}

// -----------------------------------------------------------------------------

namespace
{
    bool
    is_allocator(clang::QualType bslma_allocator, clang::ParmVarDecl* parameter)
    {
        return parameter->getType()->isPointerType()
            && bslma_allocator == parameter->getType()->getPointeeType()->getCanonicalTypeInternal();
    }

    void
    print_parameter(clang::ParmVarDecl* parameter)
    {
#if 0
        //-dk:TODO remove
        llvm::errs() << "parameter "
                     << "name=" << parameter->getName() << " "
                     << "type=" << parameter->getType().getAsString() << " "
                     << "\n";
#endif
    }
}

// -----------------------------------------------------------------------------

static bool
uses_allocator(cool::csabase::Analyser& analyser, clang::CXXConstructorDecl const* decl)
{
    clang::TypeDecl* bslma_allocator_type(::get_bslma_allocator(analyser));
    if (!bslma_allocator_type
        || !bslma_allocator_type->getTypeForDecl())
    {
        return false;
    }
    clang::QualType bslma_allocator(bslma_allocator_type->getTypeForDecl()->getCanonicalTypeInternal());
    
    // std::for_each(decl->param_begin(), decl->param_end(), ::print_parameter);
    return std::find_if(decl->param_begin(), decl->param_end(), std::bind1st(std::ptr_fun(::is_allocator), bslma_allocator))
        != decl->param_end();
}

// -----------------------------------------------------------------------------

#if 0
static void
print_ctor(cool::csabase::Analyser* analyser, clang::CXXConstructorDecl const* decl)
{
#if 0
    //-dk:TODO remove
    analyser->report(decl, ::check_name, "ctor template-kind=%0 copy-ctor=%1")
        << decl->getTemplatedKind()
        << decl->isCopyConstructor();
#endif
    std::for_each(decl->param_begin(), decl->param_end(), ::print_parameter);
}

static void
print_method(cool::csabase::Analyser* analyser, clang::CXXMethodDecl const* decl)
{
#if 0
    //-dk:TODO remove
    analyser->report(decl, ::check_name, "method template-kind=%0")
        << decl->getTemplatedKind();
#endif
    std::for_each(decl->param_begin(), decl->param_end(), ::print_parameter);
}
#endif

static bool
uses_allocator(cool::csabase::Analyser& analyser, clang::CXXRecordDecl const* record)
{
#if 0
    //-dk:TODO remove
    llvm::errs() << "cxx-record "
                 << "name=" << record->getName() << " "
                 << "\n";
#endif
    //std::for_each(record->ctor_begin(), record->ctor_end(),
    //              std::bind1st(std::ptr_fun(::print_ctor), &analyser));
    //std::for_each(record->method_begin(), record->method_end(),
    //              std::bind1st(std::ptr_fun(::print_method), &analyser));
    return false;
}

// -----------------------------------------------------------------------------

static void
check(cool::csabase::Analyser& analyser, clang::CXXConstructorDecl const* decl)
{
    bool uses_allocator(::uses_allocator(analyser, decl));
#if 0
    //-dk:TODO
    llvm::errs() << "allocator forward "
                 << "parent=" << decl->getParent()->getName() << " "
                 << "body=" << decl->hasBody() << " "
                 << "uses=" << uses_allocator << " "
                 << "\n";
#endif
    if (uses_allocator)
    {
        ::allocator_info& info(analyser.attachment< ::allocator_info>());
        clang::Type const* parent(decl->getParent()->getCanonicalDecl()->getTypeForDecl()->getCanonicalTypeInternal().getTypePtr());
        info.uses_allocator_[parent] = true;
    }

    if (uses_allocator && decl->hasBody())
    {
        ::allocator_info& info(analyser.attachment< ::allocator_info>());
        clang::QualType bslma_allocator(get_bslma_allocator(analyser)->getTypeForDecl()->getCanonicalTypeInternal());
        clang::FunctionDecl::param_const_iterator pit(std::find_if(decl->param_begin(), decl->param_end(),
                                                                   std::bind1st(std::ptr_fun(::is_allocator), bslma_allocator)));
        clang::VarDecl const* var(*pit);
        clang::CXXRecordDecl const* cxx_record(decl->getParent());
        for (clang::RecordDecl::field_iterator fit(cxx_record->field_begin()), fend(cxx_record->field_end()); fit != fend; ++fit)
        {
            clang::Type const* type(fit->getType().getCanonicalType().getTypePtr());

#if 0
            //-dk:TODO
            llvm::errs() << "field=" << fit->getName() << " "
                         << "type=" << fit->getType().getAsString() << " "
                         << "type-class=" << cool::csabase::format(type->getTypeClass()) << " "
                         << "uses=" << info.uses_allocator_[type] << " "
                         << "\n";
#endif
            if (fit->getType().getCanonicalType()->getTypeClass() == clang::Type::Record)
            {
                clang::RecordType const* record_type(llvm::dyn_cast<clang::RecordType>(type));
                clang::RecordDecl const* record_decl(record_type? record_type->getDecl(): 0);
                clang::CXXRecordDecl const* cxx_record_decl(llvm::dyn_cast<clang::CXXRecordDecl>(record_decl));
                if (cxx_record_decl)
                {
                    ::uses_allocator(analyser, cxx_record_decl);
                }
            }
        }

        for (clang::CXXConstructorDecl::init_const_iterator it(decl->init_begin()), end(decl->init_end()); it != end; ++it)
        {
            //clang::QualType qual_type((*it)->isBaseInitializer()
            //                           ? (*it)->getBaseClass()->getCanonicalTypeInternal()
            //                           : (*it)->getAnyMember()->getType()->getCanonicalTypeInternal());
            //-dk:TODO remove analyser.report(decl, ::check_name, "type name=%0") << qual_type;
            clang::Type const* type((*it)->isBaseInitializer()
                                    ? (*it)->getBaseClass()->getCanonicalTypeInternal().getTypePtr()
                                    : (*it)->getAnyMember()->getType()->getCanonicalTypeInternal().getTypePtr());
            if (info.uses_allocator_[type])
            {
                clang::CXXConstructExpr* ctor_expr(llvm::dyn_cast<clang::CXXConstructExpr>((*it)->getInit()));
                if (ctor_expr && !::uses_allocator(analyser, ctor_expr->getConstructor()))
                {
                    //-dk:TODO locate the definition of the constructor!
                    clang::SourceLocation loc((*it)->isWritten() ? ctor_expr->getExprLoc() : decl->getLocation());
                    clang::SourceRange range((*it)->isWritten()? (*it)->getSourceRange(): var->getSourceRange());
                    if ((*it)->isBaseInitializer())
                    {
                        analyser.report(loc, check_name, "allocator not passed to base %0")
                            << (*it)->getBaseClass()->getCanonicalTypeInternal().getAsString() << range;

                    }
                    else
                    {
                        analyser.report(loc, check_name, "allocator not passed to member %0")
                            << (*it)->getAnyMember()->getNameAsString()
                            << range;
                    }
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck registerCheck(check_name, &::check);
