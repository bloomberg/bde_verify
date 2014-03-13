// -*-c++-*- groups/csa/csamisc/csamisc_calls.cpp 
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <fstream>
#include <sstream>

static std::string const check_name("calls");

// -----------------------------------------------------------------------------

namespace
{
    struct files
    {
        std::ofstream d_header;
        std::ofstream d_source;
    };
}

// -----------------------------------------------------------------------------

static void
process(bde_verify::csabase::Analyser& analyser, clang::Expr const* expr, clang::Decl const* decl)
{
    if (const clang::FunctionDecl* function = decl? llvm::dyn_cast<clang::FunctionDecl>(decl): 0) {
        function = function->getCanonicalDecl();
        std::string name;
        clang::PrintingPolicy policy(analyser.context()->getLangOpts());
        function->getNameForDiagnostic(name, policy, true);

        std::ostringstream out;
        out << name << "(";
        for (clang::FunctionDecl::param_const_iterator it(function->param_begin()), end(function->param_end()); it != end; ++it) {
            if (it != function->param_begin()) {
                out << ", ";
            }
            clang::ParmVarDecl const* param(*it);
            out << param->getType().getAsString();
            if (param->isParameterPack()) {
                out << "...";
            }
        }
        out << ")";
        clang::CXXMethodDecl const* method(llvm::dyn_cast<clang::CXXMethodDecl>(function));
        if (method && !method->isStatic()) {
            if (method->getTypeQualifiers() & clang::Qualifiers::Const) {
                out << " const";
            }
            if (method->getTypeQualifiers() & clang::Qualifiers::Volatile) {
                out << " volatile";
            }
            if (method->getTypeQualifiers() & clang::Qualifiers::Restrict) {
                out << " restrict";
            }
        }

        name += out.str();

        //-dk:TODO analyser.report(expr, check_name, "function decl: '%0'")
        //-dk:TODO     << expr->getSourceRange()
        //-dk:TODO     << out.str()
            ;
    }
    else {
        analyser.report(expr, check_name, "UF01", "Unresolved function call")
            << expr->getSourceRange();
    }
}

// -----------------------------------------------------------------------------

static void
calls(bde_verify::csabase::Analyser& analyser, clang::CallExpr const* expr)
{
    process(analyser, expr, expr->getCalleeDecl());
}

// -----------------------------------------------------------------------------

static void
ctors(bde_verify::csabase::Analyser& analyser, clang::CXXConstructExpr const* expr)
{
    process(analyser, expr, expr->getConstructor());
}

// -----------------------------------------------------------------------------

static void 
open_file(bde_verify::csabase::Analyser& analyser, clang::SourceLocation where, std::string const&, std::string const& name)
{
    //llvm::errs() << "open_file(" << name << "): " << analyser.get_location(where) << "\n";
    //analyser.report(where, check_name, "open file: '%0'") << name;
}

static void 
close_file(bde_verify::csabase::Analyser& analyser, clang::SourceLocation where, std::string const&, std::string const& name)
{
    //llvm::errs() << "close_file(" << name << ")\n";
    //analyser.report(where, check_name, "close file: '%0'") << name;
}

// -----------------------------------------------------------------------------

namespace
{
    template <typename T>
    struct analyser_binder
    {
        analyser_binder(void (*function)(bde_verify::csabase::Analyser&, clang::SourceLocation, T, std::string const&),
                        bde_verify::csabase::Analyser& analyser):
            function_(function),
            analyser_(analyser)
        {
        }
        void operator()(clang::SourceLocation where, T arg, std::string const& name) const
        {
            function_(analyser_, where, arg, name);
        }
        void          (*function_)(bde_verify::csabase::Analyser&, clang::SourceLocation, T, std::string const&);
        bde_verify::csabase::Analyser& analyser_;
    };
}

// -----------------------------------------------------------------------------

static void
subscribe(bde_verify::csabase::Analyser& analyser, bde_verify::csabase::Visitor&, bde_verify::csabase::PPObserver& observer)
{
    observer.onOpenFile  += analyser_binder<std::string const&>(open_file, analyser);
    observer.onCloseFile  += analyser_binder<std::string const&>(close_file, analyser);
}

// -----------------------------------------------------------------------------

static bde_verify::csabase::RegisterCheck register_observer(check_name, &subscribe);
static bde_verify::csabase::RegisterCheck register_calls(check_name, &calls);
static bde_verify::csabase::RegisterCheck register_ctors(check_name, &ctors);
