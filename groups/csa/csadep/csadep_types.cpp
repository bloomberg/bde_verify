// csadep_types.cpp                                                   -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csadep_types.h>
#include <csadep_dependencies.h>

#include <csabase_analyser.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_location.h>
#include <csabase_format.h>
#include <csabase_cast_ptr.h>
#include <algorithm>
#include <map>
#include <set>
#include <sstream>

using cool::csabase::cast_ptr;

// ----------------------------------------------------------------------------

static std::string const check_name("include_files");

// ----------------------------------------------------------------------------

static void
record_needed_declaration(cool::csabase::Analyser& analyser,
                          clang::Decl const*       decl,
                          clang::NamedDecl const*  named,
                          bool                     need_definition,
                          bool                     print = true)
{
    // print = true;
    if (print) {
        clang::SourceLocation location(decl->getLocStart());
        analyser.report(location, check_name, "IF01",
                "in file %2 need %0 for %1")
            << (need_definition? "definition": "declaration")
            << named->getQualifiedNameAsString()
            << analyser.get_location(decl).file()
            ;
    }
    cool::csadep::dependencies&
        deps(analyser.attachment<cool::csadep::dependencies>());
    deps.add(analyser.get_location(decl).file(),
             decl->getLocStart(),
             need_definition,
             named);
}
                      
// -----------------------------------------------------------------------------
// check_type() determines what sort of declaration is needed for
// the passed type. The kind of declaration may depend on whether
// the type is used in a declaraton or in a definition and the
// inDefinition flag indicates whether the type is used in a
// definition.

static void
check_type(cool::csabase::Analyser& analyser,
           clang::Decl const*       decl,
           clang::Type const*       type,
           bool                     inDefinition,
           bool                     print = false
           )
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
        break;
    case clang::Type::Pointer:
        {
            cast_ptr<clang::PointerType> const pointer(type);
            check_type(analyser, decl, pointer->getPointeeType().getTypePtr(), false, print);
        }
        break;
    case clang::Type::LValueReference:
    case clang::Type::RValueReference:
        {
            cast_ptr<clang::ReferenceType> const reference(type);
            check_type(analyser, decl, reference->getPointeeType().getTypePtr(), false, print);
        }
        break;
    case clang::Type::MemberPointer:
        {
            cast_ptr<clang::MemberPointerType> const member(type);
            check_type(analyser, decl, member->getPointeeType().getTypePtr(), false, print);
            check_type(analyser, decl, member->getClass(), false, print);
        }
        break;
    case clang::Type::ConstantArray:
        {
            cast_ptr<clang::ConstantArrayType> const array(type);
            check_type(analyser, decl, array->getElementType().getTypePtr(), inDefinition, print);
            //-dk:TODO check where the size is coming from!
        }
        break;
    case clang::Type::IncompleteArray:
        {
            cast_ptr<clang::IncompleteArrayType> const array(type);
            check_type(analyser, decl, array->getElementType().getTypePtr(), inDefinition, print);
        }
        break;
    case clang::Type::VariableArray:
        break; // variable sized arrays are not part of C++
    case clang::Type::DependentSizedArray:
        break; // these need to be checked during instantiation
    case clang::Type::DependentSizedExtVector:
    case clang::Type::Vector:
    case clang::Type::ExtVector:
        break; // SIMD vectors are not part of standard C++
    case clang::Type::FunctionProto: // fall through
    case clang::Type::FunctionNoProto:
        analyser.report(decl, check_name, "FP01", "TODO: function proto");
        break;
    case clang::Type::UnresolvedUsing:
        break; // these need to be checked during instantiation
    case clang::Type::Paren:
        {
            cast_ptr<clang::ParenType> const paren(type);
            check_type(analyser, decl, paren->getInnerType().getTypePtr(), inDefinition, print);
        }
        break;
    case clang::Type::Typedef:
        {
            cast_ptr<clang::TypedefType> const typeDef(type);
            if (inDefinition) {
                check_type(analyser, decl,
                           typeDef->getDecl()->getUnderlyingType().getTypePtr(), true, print);
            }
            record_needed_declaration(analyser, decl, typeDef->getDecl(), true, print);
        }
        break;
    case clang::Type::Decltype:
        break; // these become available with C++2011 only
    case clang::Type::Record:
        {
            cast_ptr<clang::RecordType> const record(type);
            record_needed_declaration(analyser, decl, record->getDecl(), inDefinition, print);
        }
        break;
    case clang::Type::Enum:
        {
            cast_ptr<clang::EnumType> const enumType(type);
            record_needed_declaration(analyser, decl, enumType->getDecl(), true, print);
        }
        break;
    case clang::Type::Elaborated:
        {
            cast_ptr<clang::ElaboratedType> const elaborated(type);
            check_type(analyser, decl, elaborated->getNamedType().getTypePtr(), inDefinition, print);
        }
        break;
    case clang::Type::SubstTemplateTypeParm:
        {
            cast_ptr<clang::SubstTemplateTypeParmType> const subst(type);
            check_type(analyser, decl, subst->getReplacementType().getTypePtr(), inDefinition, print);
        }
        break;
    case clang::Type::Attributed:
        break; // attributed types are not part of standard C++
    case clang::Type::TemplateTypeParm:
        break; // these need to be checked during instantiation
    case clang::Type::SubstTemplateTypeParmPack:
        break; // these become available with C++2011 only
    case clang::Type::TemplateSpecialization:
        {
            cast_ptr<clang::TemplateSpecializationType> templ(type);
            record_needed_declaration(analyser, decl, templ->getTemplateName().getAsTemplateDecl(), inDefinition, print);
            for (clang::TemplateSpecializationType::iterator it(templ->begin()), end(templ->end()); it != end; ++it)
            {
                if (it->getKind() == clang::TemplateArgument::Type)
                {
                    check_type(analyser, decl, it->getAsType().getTypePtr(), inDefinition, print);
                }
            }
        }
        break;
    case clang::Type::Auto:
        break; // these become available with C++2011 only
    case clang::Type::InjectedClassName:
        {
            cast_ptr<clang::InjectedClassNameType> injected(type);
            check_type(analyser, decl, injected->getInjectedSpecializationType().getTypePtr(), inDefinition, print);
        }
        break;
    case clang::Type::DependentName:
        break; // these need to be checked during instantiation
    case clang::Type::DependentTemplateSpecialization:
        break; // these need to be checked during instantiation
    case clang::Type::PackExpansion:
        break; // these become available with C++2011 only
    default:
        analyser.report(decl, check_name, "UT01",
                "unknown type class: %0")
            << cool::csabase::format(type->getTypeClass());
        break;
    }
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
    include_files& context(analyser.attachment<include_files>());
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
    on_include(analyser, from, file);
}

static void
on_skip(cool::csabase::Analyser& analyser, std::string const& from, std::string const& file)
{
    on_include(analyser, from, file);
}

// -----------------------------------------------------------------------------

static void
check_declref(cool::csabase::Analyser& analyser, clang::Decl const* decl, clang::Decl const* declref)
{
    std::string const& file(analyser.get_location(decl).file());
    include_files& context(analyser.attachment<include_files>());
    std::set<std::string> const& includes(context.includes_[file]);

    analyser.report(declref, check_name, "declref")
        << declref
        ;
    std::string header(analyser.get_location(declref).file());
    if (file != header
        && includes.find(header) == includes.end()
        // && context.reported_.insert(std::make_pair(file, header)).second
        )
    {
        analyser.report(decl, check_name,
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
                check_declref(analyser, decl, record);
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
        check_type(analyser, decl, type);
    }
}

// -----------------------------------------------------------------------------

static void
on_functiondecl(cool::csabase::Analyser& analyser, clang::FunctionDecl const* decl)
{
    for (clang::FunctionDecl::param_const_iterator it(decl->param_begin()), end(decl->param_end()); it != end; ++it)
    {
        clang::ParmVarDecl const* parameter(*it);
        check_type(analyser, parameter, parameter->getType());
        if (parameter->getDefaultArg())
        {
            check_expr(analyser, decl, parameter->getDefaultArg());
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
    //-dk:TODO analyser.report(expr, check_name, "expr");
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
            (*function_)(*analyser_, location, from, file);
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
            (*function_)(*analyser_, from, file);
        }
        void          (*function_)(cool::csabase::Analyser&, std::string const&, std::string const&);
        cool::csabase::Analyser* analyser_;
    };
}

static void
subscribe(cool::csabase::Analyser& analyser, cool::csabase::Visitor&, cool::csabase::PPObserver& observer)
{
    observer.onOpenFile += binder(on_open, analyser);
    observer.onSkipFile += skip_binder(on_skip, analyser);
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_check0(check_name, &on_decl);
static cool::csabase::RegisterCheck register_check1(check_name, &on_cxxrecorddecl);
static cool::csabase::RegisterCheck register_check3(check_name, &on_functiondecl);
static cool::csabase::RegisterCheck register_check4(check_name, &on_expr);
static cool::csabase::RegisterCheck register_observer(check_name, &subscribe);
static void
on_declref(cool::csabase::Analyser& analyser, clang::DeclRefExpr const* expr)
{
    clang::ValueDecl const* decl(expr->getDecl());
    analyser.report(decl, check_name, "declaration")
        << decl->getSourceRange()
        ;
    analyser.report(expr, check_name, "declref")
        << expr->getSourceRange()
        ;
}
static cool::csabase::RegisterCheck register_check0(check_name, &on_declref);
#else
// -----------------------------------------------------------------------------

static void
on_valuedecl(cool::csabase::Analyser& analyser, clang::VarDecl const* decl)
{
    check_type(analyser, decl, decl->getType().getTypePtr(),
                 !decl->hasExternalStorage());
}

// -----------------------------------------------------------------------------

static void
on_typedefdecl(cool::csabase::Analyser& analyser, clang::TypedefDecl const* decl)
{
    check_type(analyser, decl, decl->getUnderlyingType().getTypePtr(), false);
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_check_typedefdecl(check_name, &on_typedefdecl);
static cool::csabase::RegisterCheck register_check_valuedecl(check_name, &on_valuedecl);
#endif
