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
#include <clang/Lex/Lexer.h>
#include <map>
#include <set>
#ident "$Id: allocator_forward.cpp 161 2011-12-28 00:20:28Z kuehl $"

using clang::CXXBindTemporaryExpr;
using clang::CXXConstructExpr;
using clang::CXXConstructorDecl;
using clang::CXXCtorInitializer;
using clang::CXXMethodDecl;
using clang::CXXRecordDecl;
using clang::Decl;
using clang::Expr;
using clang::FunctionDecl;
using clang::MaterializeTemporaryExpr;
using clang::ParmVarDecl;
using clang::QualType;
using clang::RecordDecl;
using clang::RecordType;
using clang::SourceLocation;
using clang::SourceRange;
using clang::TagDecl;
using clang::Type;
using clang::VarDecl;

using cool::csabase::Analyser;
using cool::csabase::RegisterCheck;

// -----------------------------------------------------------------------------

static std::string const check_name("allocator-forward");

// -----------------------------------------------------------------------------

namespace
{
    struct allocator_info
    {
        QualType bslma_allocator_;
        std::set<const Type*> takes_allocator_;
        std::set<std::pair<const Expr*, const Decl*> > bad_cexp_;

        void set_takes_allocator(const Type* type);
        bool takes_allocator(const Type* type) const;
    };

    void allocator_info::set_takes_allocator(const Type* type)
    {
        takes_allocator_.insert(type->getCanonicalTypeInternal().getTypePtr());
    }

    bool allocator_info::takes_allocator(const Type* type) const
    {
        return takes_allocator_.count(
                                type->getCanonicalTypeInternal().getTypePtr());
    }
}

// -----------------------------------------------------------------------------

static QualType
get_bslma_allocator(Analyser& analyser)
{
    return analyser.attachment<allocator_info>().bslma_allocator_;
}

// -----------------------------------------------------------------------------

static bool
is_allocp(QualType allocator, QualType type)
    // Return 'true' iff the specified 'type' is a pointer to the specified
    // 'allocator'.
{
    return type->isPointerType() &&
           type->getPointeeType()->getCanonicalTypeInternal() == allocator;
}

static bool
is_allocator(QualType allocator, const ParmVarDecl* parameter)
    // Return 'true' iff the type of the specified 'parameter' is pointer to
    // the specified 'allocator'.
{
    return is_allocp(allocator, parameter->getType());
}

static bool
takes_allocator(Analyser& analyser, CXXConstructorDecl const* constructor)
    // Return 'true' if the specified 'constructor' has parameters and its last
    // one is an allocator pointer.
{
    unsigned n = constructor->getNumParams();
    return n > 0
        && is_allocator(get_bslma_allocator(analyser),
                        constructor->getParamDecl(n - 1));
}

static bool
last_arg_is_explicit(const CXXConstructExpr* call)
    // Return 'false' iff the specified 'call' to a constructor has arguments
    // and the last argument is the default rather than explicitly passed.
{
    if (call) {
        unsigned n = call->getNumArgs();
        return n == 0 || !call->getArg(n - 1)->isDefaultArgument();   // RETURN
    }
    return true;
}

// -----------------------------------------------------------------------------

static void
find_allocator(Analyser& analyser, TagDecl const* decl)
    // Callback to set up the type "::BloombergLP::bslma::Allocator";
{
    static const std::string allocator = "BloombergLP::bslma::Allocator";
    if (allocator == decl->getQualifiedNameAsString()) {
        analyser.attachment<allocator_info>().bslma_allocator_ =
                            decl->getTypeForDecl()->getCanonicalTypeInternal();
    }
}

// -----------------------------------------------------------------------------

static void
check_wrong_parm(Analyser& analyser, const CXXConstructExpr* expr)
    // Check for constructor calls in which an explicitly passed allocator
    // argument initializes a non-allocator parameter.  The canonical case is
    //..
    //    struct X {
    //        bdef_Function<void(*)()> d_f;
    //        X(const bdef_Function<void(*)()>& f, bslma::Allocator *a = 0);
    //    };
    //    X x(bslma::Default::defaultAllocator());
    //..
    // typically occurring when a bdef_Function member is added to a class
    // which did not have one.
{
    allocator_info& info(analyser.attachment<allocator_info>());
    QualType bslma_allocator = get_bslma_allocator(analyser);

    const CXXConstructorDecl* decl = expr->getConstructor();
    unsigned n = expr->getNumArgs();
    const ParmVarDecl* lastp;
    const Expr* lastarg;

    // For the problem to possibly occur, we need each of the following:
    //: 1 The constructor has at least two parameters.
    //:
    //: 2 The constructor and the constructor expression have the same number
    //:   of parameters/arguments. (I believe this will always be true.)
    //:
    //: 3 The final constructor parameter has a default argument.
    //:
    //: 4 The final constructor argument expression is the default argument.
    //:
    //: 5 The type of the final constructor parameter is pointer to allocator.

    if (   n >= 2
        && decl->getNumParams() == n
        && (lastp = decl->getParamDecl(n - 1))->hasDefaultArg()
        && (lastarg = expr->getArg(n - 1))->isDefaultArgument()
        && is_allocp(bslma_allocator, lastp->getType())) {

        // The error will be that the second-to-last parameter is initialized
        // by the allocator.

        const ParmVarDecl* allocp = decl->getParamDecl(n - 1);
        const ParmVarDecl* wrongp = decl->getParamDecl(n - 2);

        // Descend into the expression, looking for a conversion from an
        // allocator.  The details of this come from an examination of the type
        // structure when a test case exhibiting the problem is encountered. We
        // use a loop because elements of the descent can repeat.

        const Expr* arg = expr->getArg(n - 2);
        for (;;) {
            if (const MaterializeTemporaryExpr* mte =
                               llvm::dyn_cast<MaterializeTemporaryExpr>(arg)) {
                arg = mte->GetTemporaryExpr();
                continue;
            }

            if (const CXXBindTemporaryExpr* bte =
                                   llvm::dyn_cast<CXXBindTemporaryExpr>(arg)) {
                arg = bte->getSubExpr();
                continue;
            }

            if (const CXXConstructExpr* ce =
                                       llvm::dyn_cast<CXXConstructExpr>(arg)) {
                unsigned i;
                for (i = ce->getNumArgs(); i > 0; --i) {
                    const Expr* carg = ce->getArg(i - 1);
                    if (!carg->isDefaultArgument()) {
                        // Get the rightmost non-defaulted argument expression.
                        arg = carg->IgnoreImpCasts();
                        break;
                    }
                }
                if (0 < i && i < ce->getNumArgs()) {
                    decl = expr->getConstructor();
                    allocp = decl->getParamDecl(i);
                    wrongp = decl->getParamDecl(i - 1);
                    continue;
                }
            }

            // At this point, we should have stripped off all the outer layers
            // of the argument expression which are performing the conversion
            // to the parameter type, and have the inner expression with its
            // actual type.  If that type is pointer-to-allocator, report the
            // problem if it is new.

            if (   is_allocp(bslma_allocator, arg->getType())
                && !info.bad_cexp_.count(std::make_pair(arg, decl))) {
                analyser.report(arg->getExprLoc(), check_name, "MA01: "
                                "allocator argument initializes non-allocator "
                                "parameter '%0' of type '%1' rather than "
                                "allocator parameter '%2'")
                    << wrongp->getName()
                    << wrongp->getType().getAsString()
                    << allocp->getName()
                    << arg->getSourceRange();
                info.bad_cexp_.insert(std::make_pair(arg, decl));
            }
            
            break;  // Done.
        }
    }
}

// -----------------------------------------------------------------------------

static void
check_not_forwarded(Analyser& analyser, CXXConstructorDecl const* decl)
    // Check whether the specified constructor 'decl' has an allocator
    // parameter but fails to forward it to base class and member constructors
    // which have allocator parameters.
{
    QualType bslma_allocator(get_bslma_allocator(analyser));
    if (bslma_allocator.isNull()) {
        // We have not seen the declaration for the allocator yet, so this
        // constructor cannot be using it.
        return;                                                       // RETURN
    }

    if (!takes_allocator(analyser, decl)) {
        // This constructor does not have a final allocator pointer parameter.
        return;                                                       // RETURN
    }

    // Record that the class type of the constructor takes an allocator.
    allocator_info& info(analyser.attachment<allocator_info>());
    const Type* classType =
                       decl->getParent()->getCanonicalDecl()->getTypeForDecl();
    info.set_takes_allocator(classType);

    if (!decl->hasBody()) {
        return;                                                       // RETURN
    }
    
    // The allocator parameter is the last one.
    const ParmVarDecl* palloc = decl->getParamDecl(decl->getNumParams() - 1);

    // Iterate through the base and member initializers and report those which
    // take an allocator parameter that we do not pass.

    CXXConstructorDecl::init_const_iterator itr = decl->init_begin();
    CXXConstructorDecl::init_const_iterator end = decl->init_end();
    while (itr != end) {
        const CXXCtorInitializer* init = *itr++;

        // Type of object being initialized.
        const Type* type = init->isBaseInitializer()
                         ? init->getBaseClass()
                         : init->getAnyMember()->getType().getTypePtr();

        if (!info.takes_allocator(type)) {
            // This type doesn't take an allocator.
            continue;
        }

        const CXXConstructExpr* ctor_expr =
                             llvm::dyn_cast<CXXConstructExpr>(init->getInit());

        if (last_arg_is_explicit(ctor_expr)) {
            // The allocator parameter is passed.
            continue;
        }

        SourceLocation loc;
        SourceRange range;

        if (init->isWritten()) {
            loc = ctor_expr->getExprLoc();
            range = init->getSourceRange();
        } else {
            loc = decl->getLocation();
            range = palloc->getSourceRange();
        }

        if (init->isBaseInitializer()) {
            analyser.report(loc, check_name,
                            "MA01: allocator not passed to base %0")
                << init->getBaseClass()->getCanonicalTypeInternal().
                                                        getAsString() << range;
        } else {
            analyser.report(loc, check_name,
                            "MA01: allocator not passed to member %0")
                << init->getAnyMember()->getNameAsString() << range;
        }
    }
}

// -----------------------------------------------------------------------------

static RegisterCheck c1(check_name, &find_allocator);
static RegisterCheck c2(check_name, &check_not_forwarded);
static RegisterCheck c3(check_name, &check_wrong_parm);
