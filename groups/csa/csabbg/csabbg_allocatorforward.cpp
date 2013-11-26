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
#include <clang/Sema/Sema.h>
#include <map>
#include <set>
#include <sstream>
#ident "$Id: allocator_forward.cpp 161 2011-12-28 00:20:28Z kuehl $"

using cool::csabase::Analyser;
using cool::csabase::Debug;
using cool::csabase::PPObserver;
using cool::csabase::Visitor;
using namespace clang;

#define _c(x, y) x ## y
#define _(x, y) _c(x, y)
#define _e(d,b) Debug d(__FUNCTION__, b ); d << __LINE__ << " "
#define D  _e(_(x, __LINE__), true )
#define D_ _e(_(x, __LINE__), false)

// -----------------------------------------------------------------------------

static std::string const check_name("allocator-forward");

// -----------------------------------------------------------------------------

namespace
{

struct data
    // Data stored for this set of checks.
{
    QualType bslma_allocator_;
        // The type of 'BloombergLP::bslma::Allocator'.

    typedef std::set<std::pair<const Expr*, const Decl*> > BadCexp;
    BadCexp bad_cexp_;
        // The set of constructor expressions with an explicit allocator
        // argument where that argument does not initialize an allocator
        // parameter.

    typedef std::set<const CXXConstructorDecl*> Ctors;
    Ctors ctors_;
        // The set of constructor declarations seen.

    typedef std::set<const CXXConstructExpr*> Cexprs;
    Cexprs cexprs_;
        // The set of constructor expressions seen.

    typedef std::map<const Type*, bool> TypeTakesAllocator;
    TypeTakesAllocator type_takes_allocator_;
        // A map of whether a type has a constructor with an allocator
        // parameter.

    typedef std::map<const CXXConstructorDecl*, bool> CtorTakesAllocator;
    CtorTakesAllocator ctor_takes_allocator_;
        // A map of whether a constructor has an allocator parameter.
};

struct report
    // This class two static analysis checkers, one to detect object
    // constructors with allocator parameters which do not pass an allocator to
    // the constructors of their base classes and members with allocator
    // parameters, and a second which detects constructor expressions with an
    // explicit allocator argument in which that argument does not initialize
    // an allocator parameter.  It also contains a variety of utility methods
    // used in implementing those checks.
{
    report(cool::csabase::Analyser& analyser);
        // Create an object of this type, using the specified 'analyser' for
        // access to compiler data structures.

    CXXRecordDecl *get_record_decl(QualType type);
        // Return the record declaration for the specified 'type' and a null
        // pointer if it does not have one.

    bool is_allocator(QualType type);
        // Return 'true' iff the specified 'type' is pointer to
        // 'bslma::Allocator'.

    bool last_arg_is_explicit(const CXXConstructExpr* call);
        // Return 'false' iff the specified 'call' to a constructor has
        // arguments and the last argument is the default rather than
        // explicitly passed.

    bool takes_allocator(QualType type, bool conv = true);
        // Return 'true' if the 'specified' type has a constructor which has a
        // final allocator pointer paramater.  Optionally specify 'conv' as
        // 'true' to return 'true' if the type has a constructor with a final
        // parameter which is a 'const' reference to a class implicitly
        // constructible from an allocator pointer.  Return 'false' otherwise.

    bool takes_allocator(CXXConstructorDecl const* constructor,
                         bool conv = true);
        // Return 'true' if the specified 'constructor' has a final allocator
        // pointer paramater.  Optionally specify 'conv' as 'true' to return
        // 'true' if the 'constructor' has a final parameter which is a 'const'
        // reference to a class implicitly constructible from an allocator
        // pointer.  Return 'false' otherwise.

    void operator()();
        // Invoke the checking procedures.

    template <typename Iter>
    void check_not_forwarded(Iter begin, Iter end);
        // Invoke the forwarding check on the items in the range from the
        // specified 'begin' up to but not including the specified 'end'.

    void check_not_forwarded(const CXXConstructorDecl *decl);
        // If the specified constructor 'decl' takes an allocator parameter,
        // check whether it passes it to its subobjects.

    template <typename Iter>
    void check_not_forwarded(Iter begin, Iter end, const ParmVarDecl* palloc);
        // Check if the items in the sequence from the specified 'begin' up to
        // but not including the specified 'end' are passed the specified
        // 'palloc' allocator parameter.

    void check_not_forwarded(const CXXCtorInitializer* init,
                             const ParmVarDecl* palloc);
        // Check if the specified 'init' initializer is passed the specified
        // 'palloc' allocator parameter, if the initialized object is capable
        // of being so initialized.

    std::string parm_name(const ParmVarDecl* parm, int position);
        // Construct a descriptive name string for the specified 'parm'
        // parameter, incorporating the specified 'position'.

    template <typename Iter>
    void check_wrong_parm(Iter begin, Iter end);
        // Check if the items in the sequence from the specified 'begin' up to
        // but not including the specified 'end' initialize a non-allocator
        // parameter from an explicit allocator argument.

    void check_wrong_parm(const CXXConstructExpr *expr);
        // Check whether the specified 'expr' constructor expression contains a
        // final explicit allocator pointer argument used to initialize a non-
        // allocator parameter.  The canonical case is
        //..
        //  struct X {
        //      bdef_Function<void(*)()> d_f;
        //      X(const bdef_Function<void(*)()>& f, bslma::Allocator *a = 0);
        //  };
        //  X x(bslma::Default::defaultAllocator());
        //..
        // typically occurring when a bdef_Function member is added to a class
        // which did not have one.

    cool::csabase::Analyser &analyser_;  // afford access to compiler data
    data &data_;                         // data held for this set of checks
};

report::report(cool::csabase::Analyser& analyser)
: analyser_(analyser)
, data_(analyser.attachment<data>())
{
}

CXXRecordDecl *report::get_record_decl(QualType type)
{
    const TemplateSpecializationType *tst =
        llvm::dyn_cast<TemplateSpecializationType>(type.getTypePtr());

    if (tst) {
        type = tst->desugar();
    }

    const SubstTemplateTypeParmType *sttpt =
        llvm::dyn_cast<SubstTemplateTypeParmType>(type.getTypePtr());

    if (sttpt) {
        type = sttpt->desugar();
    }

    CXXRecordDecl *rdecl = type->getAsCXXRecordDecl();

    return rdecl;
}

bool report::is_allocator(QualType type)
    // Return 'true' iff the specified 'type' is pointer to
    // 'bslma::Allocator'.
{
    return type->isPointerType()
        && type->getPointeeType()->getCanonicalTypeInternal() ==
           data_.bslma_allocator_;
}

bool report::last_arg_is_explicit(const CXXConstructExpr* call)
    // Return 'false' iff the specified 'call' to a constructor has
    // arguments and the last argument is the default rather than
    // explicitly passed.
{
    unsigned n = call ? call->getNumArgs() : 0;

    return n == 0 || !call->getArg(n - 1)->isDefaultArgument();   // RETURN
}

bool report::takes_allocator(QualType type, bool conv)
{
    const Type *tp = type.getTypePtr();
    data::TypeTakesAllocator::iterator itr =
        data_.type_takes_allocator_.find(tp);
    if (itr != data_.type_takes_allocator_.end()) {
        return itr->second;                                       // RETURN
    }

    data_.type_takes_allocator_[tp] = false;

    CXXRecordDecl *rdecl = get_record_decl(type);

    if (!rdecl) {
        return false;                                             // RETURN
    }

    DeclContextLookupResult r = analyser_.sema().LookupConstructors(rdecl);

    for (size_t i = 0; i < r.size(); ++i) {
        if (const CXXConstructorDecl *ctor =
                llvm::dyn_cast<CXXConstructorDecl>(r[i])) {
            if (takes_allocator(ctor, conv)) {
                return data_.type_takes_allocator_[tp] = true;    // RETURN
            }
        }
    }

    return false;
}

bool report::takes_allocator(CXXConstructorDecl const* constructor, bool conv)
{
    data::CtorTakesAllocator::iterator itr =
        data_.ctor_takes_allocator_.find(constructor);
    if (itr != data_.ctor_takes_allocator_.end()) {
        return itr->second;
    }
    data_.ctor_takes_allocator_[constructor] = false;
    unsigned n = constructor->getNumParams();

    if (n == 0) {
        return false;                                             // RETURN
    }

    QualType type = constructor->getParamDecl(n - 1)->getType();

    if (is_allocator(type)) {
        return data_.ctor_takes_allocator_[constructor] = true;   // RETURN
    }

    if (!conv) {
        //return false;
    }

    const ReferenceType *ref =
        llvm::dyn_cast<ReferenceType>(type.getTypePtr());

    if (!ref) {
        return false;                                             // RETURN
    }

    type = ref->getPointeeType();

    if (!type.isConstQualified()) {
        return false;                                             // RETURN
    }

    return data_.ctor_takes_allocator_[constructor] =
        takes_allocator(type, false);
}

void report::operator()()
{
    check_not_forwarded(data_.ctors_.begin(), data_.ctors_.end());
    check_wrong_parm(data_.cexprs_.begin(), data_.cexprs_.end());
}

template <typename Iter>
void report::check_not_forwarded(Iter begin, Iter end)
{
    while (begin != end) {
        check_not_forwarded(*begin++);
    }
}

void report::check_not_forwarded(const CXXConstructorDecl *decl)
{
    if (data_.bslma_allocator_.isNull()) {
        // We have not seen the declaration for the allocator yet, so this
        // constructor cannot be using it.
        return;                                                   // RETURN
    }

    if (!decl->hasBody()) {
        return;                                                   // RETURN
    }

    if (!takes_allocator(decl, true)) {
        return;                                                   // RETURN
    }

    // The allocator parameter is the last one.
    const ParmVarDecl* palloc =
        decl->getParamDecl(decl->getNumParams() - 1);

    // Iterate through the base and member initializers and report those
    // which take an allocator parameter that we do not pass.

    check_not_forwarded(decl->init_begin(), decl->init_end(), palloc);
}

template <typename Iter>
void report::check_not_forwarded(Iter begin,
                                 Iter end,
                                 const ParmVarDecl* palloc)
{
    while (begin != end) {
        check_not_forwarded(*begin++, palloc);
    }
}

void report::check_not_forwarded(const CXXCtorInitializer* init,
                                 const ParmVarDecl* palloc)
{

    // Type of object being initialized.
    const Type* type = init->isBaseInitializer()
        ? init->getBaseClass()
        : init->getAnyMember()->getType().getTypePtr();

    if (!takes_allocator(type->getCanonicalTypeInternal(), true)) {
        return;                                                   // RETURN
    }

    const CXXConstructExpr* ctor_expr =
        llvm::dyn_cast<CXXConstructExpr>(init->getInit());

    if (takes_allocator(ctor_expr->getConstructor()) &&
        last_arg_is_explicit(ctor_expr)) {
        // The allocator parameter is passed.
        return;                                                   // RETURN
    }

    SourceLocation loc;
    SourceRange range;

    if (init->isWritten()) {
        loc = ctor_expr->getExprLoc();
        range = init->getSourceRange();
    } else {
        loc = palloc->getLocation();
        range = palloc->getSourceRange();
    }

    if (init->isBaseInitializer()) {
        analyser_.report(loc, check_name,
                "MA01: allocator not passed to base %0")
            << init->getBaseClass()->getCanonicalTypeInternal().
            getAsString() << range;
    } else {
        analyser_.report(loc, check_name,
                "MA01: allocator not passed to member %0")
            << init->getAnyMember()->getNameAsString() << range;
    }
}

std::string report::parm_name(const ParmVarDecl* parm, int position)
{
    std::ostringstream s;

    s << "parameter " << position;

    std::string name = parm->getNameAsString();

    if (name.length() > 0) {
        s << " ('" << name << "')";
    }

    return s.str();
}

template <typename Iter>
void report::check_wrong_parm(Iter begin, Iter end)
{
    while (begin != end) {
        check_wrong_parm(*begin++);
    }
}

void report::check_wrong_parm(const CXXConstructExpr *expr)
{
    const CXXConstructorDecl *decl = expr->getConstructor();
    unsigned n = expr->getNumArgs();
    const ParmVarDecl *lastp;
    const Expr *lastarg;

    // For the problem to possibly occur, we need each of the following:
    //: 1 The constructor has at least two parameters.
    //:
    //: 2 The constructor and the constructor expression have the same
    //:   number of parameters/arguments.  (I believe this will always be
    //:   true.)
    //:
    //: 3 The final constructor parameter has a default argument.
    //:
    //: 4 The final constructor argument expression is the default
    //:   argument.
    //:
    //: 5 The type of the final constructor parameter is pointer to
    //:   allocator.

    if (   n >= 2
        && decl->getNumParams() == n
        && (lastp = decl->getParamDecl(n - 1))->hasDefaultArg()
        && (lastarg = expr->getArg(n - 1))->isDefaultArgument()
        && is_allocator(lastp->getType())) {

        // The error will be that the second-to-last parameter is
        // initialized by the allocator.

        const ParmVarDecl* allocp = decl->getParamDecl(n - 1);
        const ParmVarDecl* wrongp = decl->getParamDecl(n - 2);

        // Descend into the expression, looking for a conversion from an
        // allocator.  The details of this come from an examination of the
        // type structure when a test case exhibiting the problem is
        // encountered.  We use a loop because elements of the descent can
        // repeat.

        const Expr* arg = expr->getArg(n - 2);
        for (;;) {
            arg = arg->IgnoreImpCasts();
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
                        // Get the rightmost non-defaulted argument
                        // expression.

                        arg = carg->IgnoreImpCasts();
                        break;
                    }
                }

                if (i > 0) {
                    continue;
                }
            }

            // At this point, we should have stripped off all the outer
            // layers of the argument expression which are performing the
            // conversion to the parameter type, and have the inner
            // expression with its actual type.  If that type is
            // pointer-to-allocator, report the problem if it is new.

            if (   is_allocator(arg->getType())
                && !data_.bad_cexp_.count(std::make_pair(arg, decl))) {
                analyser_.report(arg->getExprLoc(), check_name, "MA02: "
                                "allocator argument initializes "
                                "non-allocator %0 of type '%1' rather than "
                                "allocator %2")
                    << parm_name(wrongp, n - 1)
                    << wrongp->getType().getAsString()
                    << parm_name(allocp, n)
                    << arg->getSourceRange();
            }

            break;  // Done.
        }
    }
}

void subscribe(Analyser& analyser, Visitor&, PPObserver&)
    // Create a callback within the specified 'analyser' which will be invoked
    // after a translation unit has been processed.
{
    analyser.onTranslationUnitDone += report(analyser);
}

// -----------------------------------------------------------------------------

static void
find_allocator(Analyser& analyser, TagDecl const* decl)
    // Callback to set up the type "BloombergLP::bslma::Allocator";
{
    static const std::string allocator = "BloombergLP::bslma::Allocator";
    if (allocator == decl->getQualifiedNameAsString()) {
        analyser.attachment<data>().bslma_allocator_ =
                            decl->getTypeForDecl()->getCanonicalTypeInternal();
    }
}

// -----------------------------------------------------------------------------

static void
gather_ctor_exprs(Analyser& analyser, const CXXConstructExpr* expr)
    // Accumulate the specified 'expr' within the specified 'analyser'.
{
    data& info(analyser.attachment<data>());
    info.cexprs_.insert(expr);
}

// -----------------------------------------------------------------------------

static void
gather_ctor_decls(Analyser& analyser, CXXConstructorDecl const* decl)
    // Accumulate the specified 'decl' within the specified 'analyser'.
{
    data& info(analyser.attachment<data>());
    info.ctors_.insert(decl);
}

// -----------------------------------------------------------------------------

}  // close anonymous namespace

static cool::csabase::RegisterCheck c1(check_name, &find_allocator);
static cool::csabase::RegisterCheck c2(check_name, &gather_ctor_decls);
static cool::csabase::RegisterCheck c3(check_name, &gather_ctor_exprs);
static cool::csabase::RegisterCheck c4(check_name, &subscribe);
