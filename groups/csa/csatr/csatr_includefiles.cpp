// -*-c++-*- checks/include_files.cpp 
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_location.h>
#include <csabase_format.h>
#include <algorithm>
#include <map>
#include <set>
#include <sstream>
#ident "$Id$"

// -----------------------------------------------------------------------------

static std::string const check_name("include_files");

// -----------------------------------------------------------------------------
// check_type() determines if the type argument needs a declaration.
// If so, it returns true.

static bool
check_type(cool::csabase::Analyser& analyser,
           clang::Decl const*       decl,
           clang::Type const* type)
{
    switch (type->getTypeClass())
    {
    case clang::Type::Builtin:           // Built-in types are alwyas there.
    case clang::Type::Complex:           // C-style complex not in C++.
    case clang::Type::BlockPointer:      // don't know what these are...
    case clang::Type::ObjCObject:        // don't care about Objective C
    case clang::Type::ObjCInterface:     // don't care about Objective C
    case clang::Type::ObjCObjectPointer: // don't care about Objective C
    case clang::Type::TypeOfExpr:        // gcc extension
    case clang::Type::TypeOf:            // gcc extension
        return false;
    case clang::Type::Pointer:
        ::check_type(analyser, decl, llvm::dyn_cast<clang::PointerType>(type)->getPointeeType().getTypePtr());
        break;
    case clang::Type::LValueReference:
    case clang::Type::RValueReference:
        ::check_type(analyser, decl, llvm::dyn_cast<clang::ReferenceType>(type)->getPointeeType().getTypePtr());
        break;
    case clang::Type::MemberPointer:
        {
            clang::MemberPointerType const* member(llvm::dyn_cast<clang::MemberPointerType>(type));
            ::check_type(analyser, decl, member->getPointeeType().getTypePtr());
            ::check_type(analyser, decl,  member->getClass());
        }
        break;
    case clang::Type::ConstantArray:
        {
            clang::ConstantArrayType const* array(llvm::dyn_cast<clang::ConstantArrayType>(type));
            ::check_type(analyser, decl, array->getElementType().getTypePtr());
            //-dk:TODO check where the size is coming from!
        }
        break;
    case clang::Type::IncompleteArray:
        ::check_type(analyser, decl, llvm::dyn_cast<clang::IncompleteArrayType>(type)->getElementType().getTypePtr());
        break;
    case clang::Type::VariableArray:
        {
            clang::VariableArrayType const* array(llvm::dyn_cast<clang::VariableArrayType>(type));
            ::check_type(analyser, decl, array->getElementType().getTypePtr());
            //-dk:TODO ::check_expr(analyser, decl, array->getSizeExpr());
        }
        break;
    case clang::Type::DependentSizedArray:
        {
            clang::DependentSizedArrayType const* array(llvm::dyn_cast<clang::DependentSizedArrayType>(type));
            ::check_type(analyser, decl, array->getElementType().getTypePtr());
            //-dk:TODO ::check_expr(analyser, decl, array->getSizeExpr());
        }
        break;
    case clang::Type::DependentSizedExtVector:
    case clang::Type::Vector:
    case clang::Type::ExtVector:
        break; //-dk:TODO
    case clang::Type::FunctionProto: // fall through
    case clang::Type::FunctionNoProto:
        {
            clang::FunctionType const* function(llvm::dyn_cast<clang::FunctionType>(type));
            ::check_type(analyser, decl, function->getResultType().getTypePtr());
        }
        break;
    case clang::Type::UnresolvedUsing:
        break; //-dk:TODO
    case clang::Type::Paren:
        ::check_type(analyser, decl, llvm::dyn_cast<clang::ParenType>(type)->getInnerType().getTypePtr());
        break;
    case clang::Type::Typedef:
        //-dk:TODO ::check_declref(analyser, decl, llvm::dyn_cast<clang::TypedefType>(type)->getDecl());
        break;
    case clang::Type::Decltype:
        break; //-dk:TODO C++0x
    case clang::Type::Record: // fall through
    case clang::Type::Enum:
        analyser.report(decl, ::check_name, "enum");
        //-dk:TODO ::check_declref(analyser, decl, llvm::dyn_cast<clang::TagType>(type)->getDecl());
        break;
    case clang::Type::Elaborated:
        ::check_type(analyser, decl, llvm::dyn_cast<clang::ElaboratedType>(type)->getNamedType().getTypePtr());
        break;
    case clang::Type::SubstTemplateTypeParm:
        break; // the substituted template type parameter needs to be already declared
    case clang::Type::Attributed:
    case clang::Type::TemplateTypeParm:
    case clang::Type::SubstTemplateTypeParmPack:
        analyser.report(decl, ::check_name, "TODO type class: %0") << cool::csabase::format(type->getTypeClass());
        break;
    case clang::Type::TemplateSpecialization:
        {
            clang::TemplateSpecializationType const* templ(llvm::dyn_cast<clang::TemplateSpecializationType>(type));
            //-dk:TODO ::check_declref(analyser, decl, templ->getTemplateName().getAsTemplateDecl());
            for (clang::TemplateSpecializationType::iterator it(templ->begin()), end(templ->end()); it != end; ++it)
            {
                if (it->getKind() == clang::TemplateArgument::Type)
                {
                    ::check_type(analyser, decl, it->getAsType().getTypePtr());
                }
            }
        }
        break;
    case clang::Type::Auto:
    case clang::Type::InjectedClassName:
    case clang::Type::DependentName:
    case clang::Type::DependentTemplateSpecialization:
    case clang::Type::PackExpansion:
        analyser.report(decl, ::check_name, "TODO type class: %0") << cool::csabase::format(type->getTypeClass());
        break;
        break; // don't care about objective C
    default:
        analyser.report(decl, ::check_name, "unknown type class: %0") << cool::csabase::format(type->getTypeClass());
        break;
    }

    return true;
}

// -----------------------------------------------------------------------------

#if 0
namespace
{
    struct include_files
    {
        std::map<std::string, std::set<std::string> >  includes_;
        std::set<std::pair<std::string, std::string> > reported_;
    };
}

// -----------------------------------------------------------------------------

static void
on_include(cool::csabase::Analyser& analyser, std::string const& from, std::string const& file)
{
    ::include_files& context(analyser.attachment<include_files>());
    context.includes_[from].insert(file);
    if (analyser.is_component_header(from))
    {
        context.includes_[analyser.toplevel()].insert(file);
    }
    //llvm::errs() << "on_include(" << from << ", " << file << ")\n";
}

// -----------------------------------------------------------------------------

static void
on_open(cool::csabase::Analyser& analyser, clang::SourceLocation, std::string const& from, std::string const& file)
{
    ::on_include(analyser, from, file);
}

static void
on_skip(cool::csabase::Analyser& analyser, std::string const& from, std::string const& file)
{
    ::on_include(analyser, from, file);
}

// -----------------------------------------------------------------------------

static void
check_declref(cool::csabase::Analyser& analyser, clang::Decl const* decl, clang::Decl const* declref)
{
    std::string const& file(analyser.get_location(decl).file());
    ::include_files& context(analyser.attachment<include_files>());
    std::set<std::string> const& includes(context.includes_[file]);

    analyser.report(declref, ::check_name, "declref")
        << declref
        ;
    std::string header(analyser.get_location(declref).file());
    if (file != header
        && includes.find(header) == includes.end()
        // && context.reported_.insert(std::make_pair(file, header)).second
        )
    {
        analyser.report(decl, ::check_name,
                        "header file '%0' only included indirectly!")
            << header;
    }
}

// -----------------------------------------------------------------------------

static void
on_cxxrecorddecl(cool::csabase::Analyser& analyser, clang::CXXRecordDecl const* decl)
{
    if (decl->hasDefinition())
    {
        for (clang::CXXRecordDecl::base_class_const_iterator it(decl->bases_begin()), end(decl->bases_end()); it != end; ++it)
        {
            clang::CXXRecordDecl* record(it->getType()->getAsCXXRecordDecl());
            if (record)
            {
                ::check_declref(analyser, decl, record);
            }
        }
    }
}

// -----------------------------------------------------------------------------

static void
check_expr(cool::csabase::Analyser& analyser, clang::Decl const* decl, clang::Expr const* expr)
{
    //-dk:TODO
}

// -----------------------------------------------------------------------------

static void
check_type(cool::csabase::Analyser& analyser, clang::Decl const* decl, clang::Type const* type);

static void
check_type(cool::csabase::Analyser& analyser, clang::Decl const* decl, clang::QualType qual_type)
{
    if (clang::Type const* type = qual_type.getTypePtrOrNull())
    {
        ::check_type(analyser, decl, type);
    }
}

// -----------------------------------------------------------------------------

static void
on_functiondecl(cool::csabase::Analyser& analyser, clang::FunctionDecl const* decl)
{
    for (clang::FunctionDecl::param_const_iterator it(decl->param_begin()), end(decl->param_end()); it != end; ++it)
    {
        clang::ParmVarDecl const* parameter(*it);
        ::check_type(analyser, parameter, parameter->getType());
        if (parameter->getDefaultArg())
        {
            ::check_expr(analyser, decl, parameter->getDefaultArg());
        }
    }
}

// -----------------------------------------------------------------------------

static void
on_decl(cool::csabase::Analyser& analyser, clang::Decl const* decl)
{
#if 0
    clang::NamedDecl const* named(llvm::dyn_cast<clang::NamedDecl>(decl));
    llvm::errs() << "on_decl " << (named? named->getNameAsString(): "") << " "
                 << "loc=" << analyser.get_location(decl) << " "
                 << "\n";
#endif
}

// -----------------------------------------------------------------------------

static void
on_expr(cool::csabase::Analyser& analyser, clang::Expr const* expr)
{
    //-dk:TODO analyser.report(expr, ::check_name, "expr");
}

// -----------------------------------------------------------------------------

namespace
{
    struct binder
    {
        binder(void (*function)(cool::csabase::Analyser&, clang::SourceLocation, std::string const&, std::string const&),
               cool::csabase::Analyser& analyser):
            function_(function),
            analyser_(&analyser)
        {
        }
        void
        operator()(clang::SourceLocation location, std::string const& from, std::string const& file) const
        {
            (*this->function_)(*this->analyser_, location, from, file);
        }
        void          (*function_)(cool::csabase::Analyser&, clang::SourceLocation, std::string const&, std::string const&);
        cool::csabase::Analyser* analyser_;
    };
}

namespace
{
    struct skip_binder
    {
        skip_binder(void (*function)(cool::csabase::Analyser&, std::string const&, std::string const&), cool::csabase::Analyser& analyser):
            function_(function),
            analyser_(&analyser)
        {
        }
        void
        operator()(std::string const& from, std::string const& file)
        {
            (*this->function_)(*this->analyser_, from, file);
        }
        void          (*function_)(cool::csabase::Analyser&, std::string const&, std::string const&);
        cool::csabase::Analyser* analyser_;
    };
}

static void
subscribe(cool::csabase::Analyser& analyser, cool::csabase::Visitor&, cool::csabase::PPObserver& observer)
{
    observer.onOpenFile += ::binder(::on_open, analyser);
    observer.onSkipFile += ::skip_binder(::on_skip, analyser);
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_check0(check_name, &::on_decl);
static cool::csabase::RegisterCheck register_check1(check_name, &::on_cxxrecorddecl);
static cool::csabase::RegisterCheck register_check3(check_name, &::on_functiondecl);
static cool::csabase::RegisterCheck register_check4(check_name, &::on_expr);
static cool::csabase::RegisterCheck register_observer(check_name, &::subscribe);
static void
on_declref(cool::csabase::Analyser& analyser, clang::DeclRefExpr const* expr)
{
    clang::ValueDecl const* decl(expr->getDecl());
    analyser.report(decl, ::check_name, "declaration")
        << decl->getSourceRange()
        ;
    analyser.report(expr, ::check_name, "declref")
        << expr->getSourceRange()
        ;
}
static cool::csabase::RegisterCheck register_check0(check_name, &::on_declref);
#else
// -----------------------------------------------------------------------------

static void
on_valuedecl(cool::csabase::Analyser& analyser, clang::DeclaratorDecl const* decl)
{
    if (::check_type(analyser, decl, decl->getType().getTypePtr()))
    {
        analyser.report(decl, ::check_name, "value decl");
    }
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_check2(check_name, &::on_valuedecl);
#endif
