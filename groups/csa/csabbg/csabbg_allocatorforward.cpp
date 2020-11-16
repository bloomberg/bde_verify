// csabbg_allocatorforward.cpp                                        -*-C++-*-

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <clang/AST/CXXInheritance.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/TemplateBase.h>
#include <clang/AST/TemplateName.h>
#include <clang/AST/Type.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchersInternal.h>
#include <clang/ASTMatchers/ASTMatchersMacros.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/Specifiers.h>
#include <clang/Sema/Sema.h>
#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_filenames.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_util.h>
#include <csaglb_comments.h>
#include <csaglb_includes.h>
#include <llvm/ADT/APSInt.h>
#include <llvm/ADT/Optional.h>
#include <llvm/Support/Casting.h>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace csabase { class PPObserver; }
namespace csabase { class Visitor; }

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::ast_matchers::internal;
using namespace csabase;

// -----------------------------------------------------------------------------

static std::string const check_name("allocator-forward");

// -----------------------------------------------------------------------------

namespace {

std::string prune(std::string name, std::string kill) {
    size_t pos;
    while ((pos = contains_word(name, kill)) != name.npos) {
        name.erase(pos, kill.length());
        while (name[pos] == ' ') {
            name.erase(pos, 1);
        }
        while (pos > 0 && name[pos - 1] == ' ') {
            name.erase(--pos, 1);
        }
    }
    return name;
};

bool is_allocator(QualType    type,
                  ASTContext& c,
                  bool        includeBases = true,
                  bool        excludeConst = false)
    // Return 'true' iff the specified 'type' is a pointer or reference to an
    // allocator.  If the optionally specified 'includeBases' is false, do not
    // consider base classes of 'type'.  If the optionally specified
    // 'excludeConst' is true, do not consider const-qualified pointers.
{
    static const std::string a1 = "BloombergLP::bslma::Allocator";
    static const std::string a2 = "bsl::allocator";

    type = type.getDesugaredType(c);

    if (type->isPointerType()) {
        type = type->getPointeeType().getDesugaredType(c);
        if (excludeConst && type.isConstQualified()) {
            return false;
        }
    } else if (type->isReferenceType()) {
        type = type->getPointeeType().getDesugaredType(c);
        if (type->isPointerType()) {
            type = type->getPointeeType().getDesugaredType(c);
            if (excludeConst && type.isConstQualified()) {
                return false;
            }
        }
    }
    bool is = false;
    if (auto r = type->getAsCXXRecordDecl()) {
        auto all_true = [](const CXXRecordDecl *decl) { return true; };
        auto not_alloc = [](const CXXRecordDecl *decl) {
            std::string t = decl->getQualifiedNameAsString();
            return t != a1 && t != a2;
        };
        auto rd = r->getDefinition();
        is = !not_alloc(r) ||
             (includeBases &&
              rd &&
              rd->forallBases(all_true) &&
              !rd->forallBases(not_alloc));
    }
    return is;
}

bool is_allocator(const Type& type,
                  ASTContext& c,
                  bool        includeBases = true,
                  bool        excludeConst = false)
{
    return is_allocator(QualType(&type, 0), c, includeBases, excludeConst);
}

bool is_allocator_tag(QualType type, ASTContext& c)
    // Return 'true' iff the specified type is 'bsl::allocator_arg_t'.
{
    static const std::string aa = "bsl::allocator_arg_t";

    if (auto r = type.getDesugaredType(c)->getAsCXXRecordDecl()) {
        if (r->hasDefinition()) {
            return aa == r->getDefinition()->getQualifiedNameAsString();
        }
    }
    return false;
}

bool is_allocator_tag(const Type& type, ASTContext& c)
{
    return is_allocator_tag(QualType(&type, 0), c);
}

}

namespace clang {
namespace ast_matchers {

AST_MATCHER_P(TemplateArgument, equalsIntegral, unsigned, N) {
  return Node.getKind() == TemplateArgument::Integral &&
         Node.getAsIntegral() == N;
}

AST_MATCHER_P(FunctionDecl, hasLastParameter,
               internal::Matcher<ParmVarDecl>, InnerMatcher) {
    unsigned np = 0;
    if (const auto *ctor = llvm::dyn_cast<CXXConstructorDecl>(&Node)) {
        if (ctor->isCopyOrMoveConstructor()) {
            np = 1;
        }
    }

    for (unsigned i = Node.getNumParams(); i > np; --i) {
        const auto &p = *Node.getParamDecl(i - 1);
        if (InnerMatcher.matches(p, Finder, Builder)) {
            return true;
        }
        if (!p.hasDefaultArg()) {
            break;
        }
    }
    return false;
}

AST_MATCHER_P(FunctionDecl, hasFirstParameter,
               internal::Matcher<ParmVarDecl>, InnerMatcher) {
    return Node.getNumParams() > 0 &&
           InnerMatcher.matches(*Node.getParamDecl(0), Finder, Builder);
}

AST_MATCHER_P(FunctionDecl, hasSecondParameter,
               internal::Matcher<ParmVarDecl>, InnerMatcher) {
    return Node.getNumParams() > 1 &&
           InnerMatcher.matches(*Node.getParamDecl(1), Finder, Builder);
}

AST_MATCHER(Type, isAllocator) {
    return is_allocator(Node, Finder->getASTContext(), true, true);
}

AST_MATCHER(Type, isAllocatorTag) {
    return is_allocator_tag(Node, Finder->getASTContext());
}

}
}

namespace
{

enum AllocatorLocation { a_None, a_Last, a_Second };

struct data
    // Data stored for this set of checks.
{
    typedef std::vector<const CXXConstructorDecl*> Ctors;
    Ctors ctors_;
        // The set of constructor declarations seen.

    typedef std::set<const NamedDecl*> DeclsWithAllocatorTrait;
    DeclsWithAllocatorTrait decls_with_true_allocator_trait_;
        // The set of declarations having a true allocator trait.

    DeclsWithAllocatorTrait decls_with_false_allocator_trait_;
        // The set of declarations having a false allocator trait.

    DeclsWithAllocatorTrait decls_with_dependent_allocator_trait_;
        // The set of declarations having a dependent allocator trait.

    typedef std::set<const CXXConstructExpr*> Cexprs;
    Cexprs cexprs_;
        // The set of constructor expressions seen.

    typedef std::map<const Expr *, const CXXCtorInitializer*> Cinits;
    Cinits cinits_;
        // The set of constructor initializers seen.

    typedef std::map<const Type*, AllocatorLocation> TypeTakesAllocator;
    TypeTakesAllocator type_takes_allocator_;
        // A map of whether a type has a constructor with an allocator
        // parameter.

    typedef std::map<const CXXConstructorDecl *, AllocatorLocation>
                       CtorTakesAllocator;
    CtorTakesAllocator ctor_takes_allocator_;
        // A map of whether a constructor has an allocator parameter.

    typedef std::set<const ReturnStmt*> Returns;
    Returns returns_;

    typedef std::set<const VarDecl*> Globals;
    Globals globals_;

    typedef std::map<FileID, std::set<llvm::StringRef>> AddedFiles;
    AddedFiles added_;

    typedef std::map<const CXXRecordDecl *, const CXXMethodDecl *>
                     AllocatorMethods;
    AllocatorMethods allocator_methods_;
        // The 'allocator()' methods of records.

    typedef std::map<const CXXRecordDecl *, const FieldDecl *>
        AllocatorMembers;
    AllocatorMembers allocator_members_;
        // A map of the 'd_allocator_p' members of records.
};

struct report : Report<data>
    // This class two static analysis checkers, one to detect object
    // constructors with allocator parameters which do not pass an allocator to
    // the constructors of their base classes and members with allocator
    // parameters, and a second which detects constructor expressions with an
    // explicit allocator argument in which that argument does not initialize
    // an allocator parameter.  It also contains a variety of utility methods
    // used in implementing those checks.
{
    INHERIT_REPORT_CTOR(report, Report, data);

    const CXXRecordDecl *get_record_decl(QualType type);
        // Return the record declaration for the specified 'type' and a null
        // pointer if it does not have one.

    bool is_allocator(QualType type,
                      bool     includeBases = true,
                      bool     excludeConst = false);
        // Return 'true' iff the specified 'type' is pointer or reference to
        // 'bslma::Allocator'.  If the optionally specified 'includeBases' is
        // false, do not consider base classes of 'type'.  If the optionally
        // specified 'excludeConst' is true, do not consider const-qualified
        // pointers.

    bool is_allocator_tag(QualType type);
        // Return 'true' iff the specified 'type' is 'bsl::allocator_arg_t'.

    bool last_arg_is_explicit_allocator(const CXXConstructExpr* call);
        // Return 'true' iff the specified 'call' to a constructor has
        // arguments and the last argument is an explicitly passed allocator.

    AllocatorLocation takes_allocator(QualType type);
        // Return 'true' iff the 'specified' type has a constructor which has a
        // final allocator paramater.

    AllocatorLocation takes_allocator(CXXConstructorDecl const* constructor);
        // Return 'true' iff the specified 'constructor' has a final allocator
        // pointer paramater.

    void match_nested_allocator_trait(const BoundNodes& nodes);
        // Callback for classes with nested allocator traits.

    void match_force_implicit(const BoundNodes& nodes);
        // Callback for classes in 'allocator_transform' config.

    SourceLocation semiAfterRecordDecl(const CXXRecordDecl *record);
        // Return the location of the semicolon after the full declaration that
        // declares the specfied 'record'.  For example, it would return the
        // location of the semicolon given 'struct X{}D[2];'.

    void force_implicit_definitions(const CXXRecordDecl *record);
        // Force the instantiation of the various implicit members of the
        // specified 'record' that the compiler is lazily trying to put off.

    void match_class_using_allocator(const BoundNodes& nodes);
        // Callback for classes having constructors with allocator parameters.

    void match_allocator_trait(data::DeclsWithAllocatorTrait* set,
                               const BoundNodes& nodes);
        // Method to insert discovered classes with allocator traits contained
        // within the specifed 'nodes' into the specified 'set'.

    void match_negative_allocator_trait(const BoundNodes& nodes);
        // Callback for discovered classes with negative allocator traits
        // contained within the specifed 'nodes'.

    void match_positive_allocator_trait(const BoundNodes& nodes);
        // Callback for discovered classes with positive allocator traits
        // contained within the specifed 'nodes'.

    void match_dependent_allocator_trait(const BoundNodes& nodes);
        // Callback for discovered classes with dependent allocator traits
        // contained within the specifed 'nodes'.

    void match_ctor_expr(const BoundNodes& nodes);
        // Callback for constructor expressions.

    void match_ctor_init(const BoundNodes& nodes);
        // Callback for constructor initializers.

    void match_return_stmt(const BoundNodes& nodes);
        // Callback for return statements.

    void match_var_decl(const BoundNodes& nodes);
        // Callback for variable declarations.

    void match_ctor_decl(const BoundNodes& nodes);
        // Callback for constructor declarations.

    bool hasRVCognate(const FunctionDecl *func);
        // Return 'true' iff the specified 'func' (which returns 'void' and
        // returns a value through a pointer first parameter) has a cognate
        // function that returns by value.

    void match_should_return_by_value(const BoundNodes& nodes);
        // Callback for functions which could return by value instead of
        // through a pointer.

    void operator()();
        // Invoke the checking procedures.

    void check_globals_use_allocator(data::Globals::const_iterator begin,
                                     data::Globals::const_iterator end);

    bool has_public_copy_constructor(const CXXRecordDecl *decl);
        // Whether the class has a publicly accessible copy constructor.

    bool should_transform(const CXXRecordDecl *record);
        // Whether the class name is in the 'allocator_transform' config.

    void match_allocator_method(const BoundNodes& nodes);
        // Callback for allocator method declaration.

    void match_allocator_member(const BoundNodes& nodes);
        // Callback for 'd_allocator_p' member declaration.

    const FieldDecl* has_array_member(const CXXRecordDecl *record);
        // If the specified 'record' has array members, return one of them,
        // otherwise return null.

    void check_not_forwarded(data::Ctors::const_iterator begin,
                             data::Ctors::const_iterator end);
        // Invoke the forwarding check on the items in the range from the
        // specified 'begin' up to but not including the specified 'end'.

    void check_not_forwarded(const CXXConstructorDecl *decl);
        // If the specified constructor 'decl' takes an allocator parameter,
        // check whether it passes the parameter to its subobjects.

    void check_not_forwarded(CXXConstructorDecl::init_const_iterator begin,
                             CXXConstructorDecl::init_const_iterator end,
                             const ParmVarDecl* palloc);
        // Check if the items in the sequence from the specified 'begin' up to
        // but not including the specified 'end' are passed the specified
        // 'palloc' allocator parameter.

    bool write_allocator_trait(const CXXRecordDecl *record, bool bslma);
        // Write out a true allocator trait for the specified 'record'.  Write
        // a 'UsesBslmaAllocator' trait if the specified 'bslma' is true, and a
        // 'UsesAllocatorArgT' trait otherwise.  Return true if the trait was
        // writen, and false if it was not.

    bool write_allocator_method_declaration(const CXXRecordDecl    *record,
                                            AllocatorLocation       kind,
                                            const CXXBaseSpecifier *base,
                                            const FieldDecl        *field);
        // Write out an allocator method declaration for the specified
        // 'record', basing its return type on whether the specified 'kind'
        // refers to a 'bslma::Allocator *' or a 'bsl::allocator_arg'.  Return
        // true if the declaration was written, and false if it was not.

    bool write_allocator_method_definition(const CXXRecordDecl    *record,
                                           AllocatorLocation       kind,
                                           const CXXBaseSpecifier *base,
                                           const FieldDecl        *field);
        // Write out an allocator method definition for the specified 'record',
        // basing its return type on whether the specified 'kind' refers to a
        // 'bslma::Allocator *' or a 'bsl::allocator_arg'.  Return true if the
        // definition was written, and false if it was not.  The method
        // forwards to the first of the specified non-null 'base' or 'field' if
        // one exists, and to an assumed 'd_allocator_p' member otherwise.

    bool write_in_class_allocator_method_definition(
                                                const CXXRecordDecl    *record,
                                                AllocatorLocation       kind,
                                                const CXXBaseSpecifier *base,
                                                const FieldDecl        *field);
        // Write out an allocator method definition for the specified 'record'
        // inside the class definition, basing its return type on whether the
        // specified 'kind' refers to a 'bslma::Allocator *' or a
        // 'bsl::allocator_arg'.  Return true if the definition was written,
        // and false if it was not.  The method forwards to the first of the
        // specified non-null 'base' or 'field' if one exists, and to an
        // assumed 'd_allocator_p' member otherwise.

    bool write_assignment_declaration(const CXXRecordDecl *record);
        // Write out an assignment operator declaration for the specified
        // 'record'.  Return true if the declaration was written, and false if
        // it was not.  Note that this is needed for the case when we add a
        // 'd_allocator_p' field to the class, because assignment should not
        // reassign allocators.

    bool write_assignment_definition(const CXXRecordDecl *record);
        // Write out an assignment operator definition for the specified
        // 'record'.  Return true if the definition was written, and false if
        // it was note

    bool write_in_class_assignment_definition(const CXXRecordDecl *record);
        // Write out an assignment operator definition for the specified
        // 'record' inside the class definition.  Return true if the definition
        // was written, and false if it was note

    bool write_d_allocator_p_declaration(const CXXRecordDecl *record);
        // Write out the declaration for a private 'd_allocator_p' pointer
        // member for the specified 'record'.  Return true if the declaration
        // was written, and false if it was not.

    bool write_ctor_with_allocator_declaration(
                                              const CXXRecordDecl      *record,
                                              const CXXConstructorDecl *decl);
        // Write out the declaration for a constructor for the specified
        // 'record' that mirrors the specified 'decl' constructor but which
        // also takes a final allocator parameter.  Return true if the
        // declaration was written, and false if it was not.

    bool write_ctor_with_allocator_definition(
                                  const CXXRecordDecl      *record,
                                  const CXXConstructorDecl *decl,
                                  bool                      init_allocator,
                                  bool                      default_allocator);
        // Write out the definition for a constructor for the specified
        // 'record' that mirrors the specified 'decl' constructor but which
        // also takes a final allocator parameter.  Return true if the
        // definition was written, and false if it was not.

    void include(SourceLocation loc, llvm::StringRef name);
        // Generate a file inclusion of the specified 'name' in the file
        // containing the specified 'loc'.

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

    template <typename Iter>
    void check_uses_allocator(Iter begin, Iter end);
        // Check if the items in the sequence from the specified 'begin' up to
        // but not including the specified 'end' initialize an allocator
        // parameter from an object allocator.

    void check_uses_allocator(const CXXConstructExpr *expr);
        // Check whether the specified 'expr' constructor expression contains a
        // final explicit allocator pointer used to initialize an allocator
        // parameter from an object allocator.  The canonical case is
        //..
        //  struct X {
        //      bslma::Allocator *allocator() const;
        //      X& operator=(const X& o) {
        //          X(o, allocator()).swap(*this);
        //          return *this;
        //      }
        //  };
        //..
        // which can run out of memory when the object allocator does not free
        // memory (as in sequential allocators).

    template <typename Iter>
    void check_alloc_returns(Iter begin, Iter end);
        // Check that the return statements in the specified half-open range
        // '[ begin .. end )' do not return items that take allocators.

    void check_alloc_return(const ReturnStmt* stmt);
        // Check that the specified return 'stmt' does not return an item that
        // takes allocators.
};

const CXXRecordDecl *report::get_record_decl(QualType type)
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

    const CXXRecordDecl *rdecl = type->getAsCXXRecordDecl();

    if (!rdecl) {
        rdecl = type->getPointeeCXXRecordDecl();
    }

    if (rdecl) {
        rdecl = rdecl->getCanonicalDecl();
    }

    return rdecl;
}

bool report::is_allocator(QualType type, bool includeBases, bool excludeConst)
{
    return ::is_allocator(type, *a.context(), includeBases, excludeConst);
}

bool report::is_allocator_tag(QualType type)
{
    return ::is_allocator_tag(type, *a.context());
}

bool report::last_arg_is_explicit_allocator(const CXXConstructExpr* call)
{
    if (call) {
        for (unsigned n = call->getNumArgs(); n > 0; --n) {
            const Expr *last = call->getArg(n - 1);
            if (last->isDefaultArgument()) {
                continue;
            }
            if (is_allocator(last->getType())) {
                return true;
            }
        }
    }
    return false;
}

AllocatorLocation report::takes_allocator(QualType type)
{
    while (type->isArrayType()) {
        type = QualType(type->getArrayElementTypeNoTypeQual(), 0);
    }
    return d.type_takes_allocator_
        [type.getTypePtr()->getCanonicalTypeInternal().getTypePtr()];
}

AllocatorLocation report::takes_allocator(
                                         CXXConstructorDecl const *constructor)
{
    data::CtorTakesAllocator::iterator itr =
        d.ctor_takes_allocator_.find(constructor);
    if (itr != d.ctor_takes_allocator_.end()) {
        return itr->second;
    }

    unsigned n = constructor->getNumParams();

    if (n == 0 || (constructor->isCopyOrMoveConstructor() && n == 1)) {
        return d.ctor_takes_allocator_[constructor] = a_None;         // RETURN
    }

    if (n > 1) {
        QualType t1 = constructor->getParamDecl(0)->getType();
        QualType t2 = constructor->getParamDecl(1)->getType();
        if (is_allocator_tag(t1) && is_allocator(t2, true, true)) {
            return d.ctor_takes_allocator_[constructor] = a_Second;
        }
    }

    while (n-- > 0) {
        QualType type = constructor->getParamDecl(n)->getType();
        if (is_allocator(type, true, true)) {
            return d.ctor_takes_allocator_[constructor] = a_Last;
        }
        else if (!constructor->getParamDecl(n)->hasDefaultArg()) {
            break;
        }
    }
    return d.ctor_takes_allocator_[constructor] = a_None;
}

static internal::DynTypedMatcher nested_allocator_trait_matcher()
    // Return an AST matcher which looks for nested traits.  Expanded from
    // macros, allocator traits look like:
    //..
    //  class MyClass { operator bslalg::TypeTraitUsesBslmaAllocator::
    //                           NestedTraitDeclaration<MyClass>() const ... };
    //..
    // or
    //..
    //  class MyClass { operator BloombergLP::bslmf::NestedTraitDeclaration<
    //                                      MyClass,
    //                                      bslma::UsesBslmaAllocator,
    //                                      true>() const ... };
    //..
    // In the second case above, the final boolean parameter may also be false
    // or missing.  The details of the classes involved are too hairy to tease
    // out in the AST matcher; instead the matcher looks for a superset of
    // methods and the callback looks for further structure.
{
    return decl(forEachDescendant(
        cxxMethodDecl(
            matchesName("::operator NestedTraitDeclaration($|<)"),
            returns(qualType().bind("type")),
            ofClass(recordDecl().bind("class"))
        ).bind("trait")
    ));
}

void report::match_nested_allocator_trait(const BoundNodes& nodes)
{
    CXXRecordDecl const* decl = nodes.getNodeAs<CXXRecordDecl>("class");
    QualType qt = *nodes.getNodeAs<QualType>("type");
    std::string type = qt.getAsString();

    if (contains_word(type, decl->getNameAsString()) == StringRef::npos) {
        auto report = a.report(nodes.getNodeAs<CXXMethodDecl>("trait"),
                         check_name, "BT01",
                         "Trait declaration does not mention its class '%0'");
        report << decl->getNameAsString();
    }

    const NamedDecl *nd = llvm::dyn_cast<NamedDecl>(decl->getCanonicalDecl());

    if (type.find("bslalg::struct TypeTraitUsesBslmaAllocator::"
                  "NestedTraitDeclaration<") == 0 ||
        type.find("bslalg_TypeTraitUsesBslmaAllocator::"
                  "NestedTraitDeclaration<") == 0 ||
        type.find("bdealg_TypeTraitUsesBdemaAllocator::"
                  "NestedTraitDeclaration<") == 0) {
        d.decls_with_true_allocator_trait_.insert(nd);
    }
    else if (type.find("BloombergLP::bslmf::NestedTraitDeclaration<") == 0 &&
             (type.find(", bslma::UsesBslmaAllocator") != type.npos ||
              type.find(", bslmf::UsesAllocatorArgT") != type.npos ||
              type.find(", BloombergLP::bslma::UsesBslmaAllocator") !=
                  type.npos ||
              type.find(", BloombergLP::bslmf::UsesAllocatorArgT") !=
                  type.npos)) {
        auto ts = qt->getAs<TemplateSpecializationType>();
        if (ts && ts->getNumArgs() == 3) {
            const TemplateArgument& ta = ts->getArg(2);
            if (!ta.isDependent()) {
                bool value, found;
                if (ta.getKind() == TemplateArgument::Integral) {
                    value = ta.getAsIntegral() != 0;
                    found = true;
                } else if (ta.getKind() == TemplateArgument::Expression &&
                           ta.getAsExpr()->EvaluateAsBooleanCondition(
                               value, *a.context())) {
                    found = true;
                } else {
                    found = false;
                }
                if (found) {
                    (value ? d.decls_with_true_allocator_trait_
                           : d.decls_with_false_allocator_trait_)
                        .insert(nd);
                    return;
                }
            }
        }
        if (type.find(", bslma::UsesBslmaAllocator, true>") != type.npos ||
            type.find(", bslma::UsesBslmaAllocator>") != type.npos ||
            type.find(", bslmf::UsesAllocatorArgT, true>") != type.npos ||
            type.find(", bslmf::UsesAllocatorArgT>") != type.npos ||
            type.find(", BloombergLP::bslma::UsesBslmaAllocator, true>") !=
                type.npos ||
            type.find(", BloombergLP::bslma::UsesBslmaAllocator>") !=
                type.npos ||
            type.find(", BloombergLP::bslmf::UsesAllocatorArgT, true>") !=
                type.npos ||
            type.find(", BloombergLP::bslmf::UsesAllocatorArgT>") !=
                type.npos) {
            d.decls_with_true_allocator_trait_.insert(nd);
        }
        else if (type.find(", bslma::UsesBslmaAllocator, false>") !=
                     type.npos ||
                 type.find(", bslmf::UsesAllocatorArgT, false>") !=
                     type.npos ||
                 type.find(
                     ", BloombergLP::bslma::UsesBslmaAllocator, false>") !=
                     type.npos ||
                 type.find(
                     ", BloombergLP::bslmf::UsesAllocatorArgT, false>") !=
                     type.npos) {
            d.decls_with_false_allocator_trait_.insert(nd);
        }
        else if (type.find(", bslma::UsesBslmaAllocator,") != type.npos ||
                 type.find(", bslmf::UsesAllocatorArgT,") != type.npos ||
                 type.find(", BloombergLP::bslma::UsesBslmaAllocator,") !=
                     type.npos ||
                 type.find(", BloombergLP::bslmf::UsesAllocatorArgT,") !=
                     type.npos) {
            d.decls_with_dependent_allocator_trait_.insert(nd);
        }
    }
}

static internal::DynTypedMatcher class_using_allocator_matcher()
    // Matcher for classes that have constructors with a final parameter that
    // is a pointer or reference to an allocator or a reference to a class that
    // has such a constructor.
{
    return decl(forEachDescendant(recordDecl(eachOf(
            has(cxxConstructorDecl(hasLastParameter(parmVarDecl(anyOf(
                hasType(pointerType(isAllocator())),
                hasType(referenceType(isAllocator()))
            )))).bind("last")),
            has(cxxConstructorDecl(allOf(
                hasFirstParameter(hasType(isAllocatorTag())),
                hasSecondParameter(parmVarDecl(anyOf(
                    hasType(pointerType(isAllocator())),
                    hasType(referenceType(isAllocator()))
                )))
            )).bind("second"))
    )).bind("r")));
}

SourceLocation report::semiAfterRecordDecl(const CXXRecordDecl *record)
{
    SourceLocation end = record->getEndLoc();
    const Decl *decl = record;
    while (nullptr != (decl = decl->getNextDeclInContext())) {
        if (m.isBeforeInTranslationUnit(decl->getBeginLoc(), end)) {
            if (m.isBeforeInTranslationUnit(end, decl->getEndLoc())) {
                end = decl->getEndLoc();
            }
        }
        else {
            break;
        }
    }
    // Based on clang::arcmt::trans::findSemiAfterLocation
    auto &lo = a.context()->getLangOpts();
    end = Lexer::getLocForEndOfToken(end, 0, m, lo);
    auto i = m.getDecomposedLoc(end);
    bool invalid = false;
    StringRef file = m.getBufferData(i.first, &invalid);
    if (!invalid) {
        auto b = file.data() + i.second;
        Lexer lexer(
            m.getLocForStartOfFile(i.first), lo, file.begin(), b, file.end());
        Token tok;
        lexer.LexFromRawLexer(tok);
        if (tok.is(tok::semi)) {
            end = tok.getLocation();
        }
    }
    return end;
}

void report::force_implicit_definitions(const CXXRecordDecl *record)
{
    auto r = const_cast<CXXRecordDecl *>(record);
    if (r->isDependentContext()) {
        // ???
        // This is a template.  If it doesn't have user-declared copy, move, or
        // defualt constructors, we can't force clang to make them for us.
        // Just punt.
        return;
    }
    a.sema().ForceDeclarationOfImplicitMembers(r);
    for (auto c : r->ctors()) {
        if (c->isDefaulted() &&
            !c->isDeleted() &&
            !c->doesThisDeclarationHaveABody()) {
            switch (a.sema().getSpecialMember(c)) {
              case Sema::CXXDefaultConstructor:
                a.sema().DefineImplicitDefaultConstructor(c->getBeginLoc(), c);
                break;
              case Sema::CXXCopyConstructor:
                a.sema().DefineImplicitCopyConstructor(c->getBeginLoc(), c);
                break;
              case Sema::CXXMoveConstructor:
                a.sema().DefineImplicitMoveConstructor(c->getBeginLoc(), c);
                break;
              default:
                break;
            }
        }
    }
}

void report::match_class_using_allocator(const BoundNodes& nodes)
{
    auto r = nodes.getNodeAs<CXXRecordDecl>("r");
    if (!r->isDependentContext()) {
        a.sema().ForceDeclarationOfImplicitMembers(
            const_cast<CXXRecordDecl *>(r));
    }
    QualType type = r->getTypeForDecl()->getCanonicalTypeInternal();
    auto &t = d.type_takes_allocator_[type.getTypePtr()];
    t = AllocatorLocation(t | (nodes.getNodeAs<Decl>("second") ? a_Second :
                               nodes.getNodeAs<Decl>("last")   ? a_Last   :
                                                                 a_None));
}

static internal::DynTypedMatcher allocator_trait_matcher(int value)
{
    return decl(forEachDescendant(
        classTemplateSpecializationDecl(
            hasName("::BloombergLP::bslma::UsesBslmaAllocator"),
            templateArgumentCountIs(1),
            isDerivedFrom(classTemplateSpecializationDecl(
                hasName("::bsl::integral_constant"),
                templateArgumentCountIs(2),
                hasTemplateArgument(0, refersToType(asString("_Bool"))),
                hasTemplateArgument(1, equalsIntegral(value)))))
            .bind("class")));
}

void report::match_allocator_trait(data::DeclsWithAllocatorTrait* set,
                                   const BoundNodes& nodes)
{
    const ClassTemplateSpecializationDecl* td =
        nodes.getNodeAs<ClassTemplateSpecializationDecl>("class");
    QualType arg = td->getTemplateArgs()[0].getAsType();
    const NamedDecl *d = arg->getAsCXXRecordDecl();
    if (!d) {
        const TemplateSpecializationType* tst =
            arg->getAs<TemplateSpecializationType>();
        if (tst) {
            d = tst->getTemplateName().getAsTemplateDecl();
            if (d) {
                d = llvm::dyn_cast<TemplateDecl>(d)->getTemplatedDecl();
            }
        }
    }
    if (d) {
        d = llvm::dyn_cast<NamedDecl>(d->getCanonicalDecl());
        set->insert(d);
    }
}

void report::match_negative_allocator_trait(const BoundNodes& nodes)
{
    match_allocator_trait(&d.decls_with_false_allocator_trait_, nodes);
}

void report::match_positive_allocator_trait(const BoundNodes& nodes)
{
    match_allocator_trait(&d.decls_with_true_allocator_trait_, nodes);
}

static internal::DynTypedMatcher dependent_allocator_trait_matcher()
{
    return decl(forEachDescendant(
        classTemplateSpecializationDecl(
            hasName("::BloombergLP::bslma::UsesBslmaAllocator"),
            templateArgumentCountIs(1),
            unless(isDerivedFrom(classTemplateSpecializationDecl(
                hasName("::bsl::integral_constant"),
                templateArgumentCountIs(2),
                hasTemplateArgument(0, refersToType(asString("_Bool"))),
                anyOf(hasTemplateArgument(1, equalsIntegral(0)),
                      hasTemplateArgument(1, equalsIntegral(1)))))))
            .bind("class")));
}

void report::match_dependent_allocator_trait(const BoundNodes& nodes)
{
    match_allocator_trait(&d.decls_with_dependent_allocator_trait_, nodes);
}

static internal::DynTypedMatcher should_return_by_value_matcher()
{
    return decl(forEachDescendant(
        functionDecl(
            returns(asString("void")),
            hasParameter(
                0,
                parmVarDecl(
                    hasType(pointerType(unless(pointee(isConstQualified())),
                                        unless(pointee(asString("void"))),
                                        unless(pointee(functionType())),
                                        unless(pointee(memberPointerType())))
                                .bind("type")))
                    .bind("parm")),
            anyOf(parameterCountIs(1),
                  hasParameter(
                      1,
                      unless(
                          anyOf(hasType(isInteger()),
                                hasType(pointerType(
                                    unless(pointee(asString("void"))),
                                    unless(pointee(functionType())),
                                    unless(pointee(memberPointerType())))))))),
            anyOf(hasDescendant(binaryOperator(
                      hasOperatorName("="),
                      hasLHS(unaryOperator(
                          hasOperatorName("*"),
                          hasUnaryOperand(ignoringImpCasts(declRefExpr(
                              to(decl(equalsBoundNode("parm")))))))))),
                  hasDescendant(cxxOperatorCallExpr(
                      hasOverloadedOperatorName("="),
                      hasArgument(
                          0,
                          ignoringImpCasts(unaryOperator(
                              hasOperatorName("*"),
                              hasUnaryOperand(ignoringImpCasts(declRefExpr(
                                  to(decl(equalsBoundNode("parm")))))))))))))
            .bind("func")));
}

bool report::hasRVCognate(const FunctionDecl *func)
{
    const DeclContext *parent = func->getLookupParent();
    std::string name = func->getNameAsString();

    DeclContext::decl_iterator declsb = parent->decls_begin();
    DeclContext::decl_iterator declse = parent->decls_end();
    while (declsb != declse) {
        const Decl *decl = *declsb++;
        const FunctionDecl* cfunc = llvm::dyn_cast<FunctionDecl>(decl);
        const FunctionTemplateDecl* ctplt =
            llvm::dyn_cast<FunctionTemplateDecl>(decl);
        if (ctplt) {
            cfunc = ctplt->getTemplatedDecl();
        }
        if (cfunc &&
            cfunc->getNameAsString() == name &&
            cfunc->getNumParams() == func->getNumParams() - 1 &&
            cfunc->getCallResultType() ==
                func->getParamDecl(0)->getOriginalType()->getPointeeType()) {
            return true;
        }
    }
    return false;
}

void report::match_should_return_by_value(const BoundNodes& nodes)
{
    auto func = nodes.getNodeAs<FunctionDecl>("func");
    auto p1 = nodes.getNodeAs<PointerType>("type");
    if (func->getCanonicalDecl() == func &&
        func->getTemplatedKind() == FunctionDecl::TK_NonTemplate &&
        !func->getLocation().isMacroID() &&
        !llvm::dyn_cast<CXXConstructorDecl>(func) &&
        !p1->getPointeeType()->isDependentType() &&
        !p1->getPointeeType()->isInstantiationDependentType() &&
        !p1->getPointeeType()->isAnyCharacterType() &&
        !p1->getPointeeType()->isIntegralType(*a.context()) &&
        !p1->getPointeeType()->isFunctionType() &&
        !p1->getPointeeType()->isMemberPointerType() &&
        !func->getParamDecl(0)->hasDefaultArg() &&
        !is_allocator(p1->desugar()) &&
        !takes_allocator(p1->getPointeeType().getCanonicalType()) &&
        (func->getNumParams() == 1 ||
            func->getParamDecl(0)->getOriginalType().getUnqualifiedType() !=
            func->getParamDecl(1)->getOriginalType().getUnqualifiedType()) &&
        !hasRVCognate(func)) {
        auto report = a.report(func, check_name, "RV01",
                 "Consider returning '%0' by value");
        
        report << p1->getPointeeType().getCanonicalType().getAsString();
        a.report(func->getParamDecl(0), check_name, "RV01",
                 "instead of through pointer parameter",
                 false, DiagnosticIDs::Note);
    }
}

static internal::DynTypedMatcher ctor_expr_matcher()
{
    return decl(forEachDescendant(cxxConstructExpr(anything()).bind("e")));
}

void report::match_ctor_expr(const BoundNodes& nodes)
{
    auto expr = nodes.getNodeAs<CXXConstructExpr>("e");
    d.cexprs_.insert(expr);
}

static internal::DynTypedMatcher ctor_init_matcher()
{
    return decl(forEachDescendant(cxxCtorInitializer(anything()).bind("i")));
}

void report::match_ctor_init(const BoundNodes& nodes)
{
    auto init = nodes.getNodeAs<CXXCtorInitializer>("i");
    d.cinits_[init->getInit()] = init;
}

static internal::DynTypedMatcher return_stmt_matcher()
{
    return decl(forEachDescendant(
        functionDecl(forEachDescendant(returnStmt(anything()).bind("r")))
            .bind("f")));
}

void report::match_return_stmt(const BoundNodes& nodes)
{
    //if (!nodes.getNodeAs<FunctionDecl>("f")->isTemplateInstantiation())
    {
        auto stmt = nodes.getNodeAs<ReturnStmt>("r");
        d.returns_.insert(stmt);
    }
}

static internal::DynTypedMatcher var_decl_matcher()
{
    return decl(forEachDescendant(varDecl(anything()).bind("v")));
}

void report::match_var_decl(const BoundNodes& nodes)
{
    auto decl = nodes.getNodeAs<VarDecl>("v");
    if (decl->hasGlobalStorage() &&
        decl->hasInit() &&
        llvm::dyn_cast<CXXConstructExpr>(decl->getInit())) {
        d.globals_.insert(decl);
    }
}

static internal::DynTypedMatcher ctor_decl_matcher()
{
    return decl(forEachDescendant(cxxConstructorDecl(anything()).bind("c")));
}

void report::match_ctor_decl(const BoundNodes& nodes)
{
    auto decl = nodes.getNodeAs<CXXConstructorDecl>("c");
    if (!decl->isDeleted()) {
        d.ctors_.push_back(decl);
    }
}

static internal::DynTypedMatcher allocator_method_matcher()
{
    return decl(forEachDescendant(cxxRecordDecl(
        isSameOrDerivedFrom(
            cxxRecordDecl(has(
                cxxMethodDecl(isConst(),
                              isPublic(),
                              parameterCountIs(0),
                              anyOf(allOf(hasName("allocator"),
                                          returns(pointerType(isAllocator()))),
                                    allOf(hasName("get_allocator"),
                                          returns(isAllocator()))))
                    .bind("m")))))
            .bind("r")));
}

void report::match_allocator_method(const BoundNodes& nodes)
{
    const CXXRecordDecl *r = nodes.getNodeAs<CXXRecordDecl>("r");
    const CXXMethodDecl *m = nodes.getNodeAs<CXXMethodDecl>("m");
    d.allocator_methods_[r->getCanonicalDecl()] = m;
}

static internal::DynTypedMatcher allocator_member_matcher()
{
    return decl(forEachDescendant(
        cxxRecordDecl(has(fieldDecl(allOf(hasName("d_allocator_p"),
                                          hasType(pointerType(isAllocator()))))
                              .bind("p")))
            .bind("r")));
}

void report::match_allocator_member(const BoundNodes& nodes)
{
    d.allocator_members_[nodes.getNodeAs<CXXRecordDecl>("r")] =
        nodes.getNodeAs<FieldDecl>("p");
}

static internal::DynTypedMatcher force_implicit_matcher()
{
    return decl(forEachDescendant(cxxRecordDecl(anything()).bind("r")));
}

void report::match_force_implicit(const BoundNodes& nodes)
{
    const auto *r = nodes.getNodeAs<CXXRecordDecl>("r");
    if (should_transform(r)) {
        force_implicit_definitions(r);
    }
}

void report::operator()()
{
    MatchFinder mf;

    OnMatch<report, &report::match_force_implicit> m14(this);
    mf.addDynamicMatcher(force_implicit_matcher(), &m14);

    OnMatch<report, &report::match_nested_allocator_trait> m2(this);
    mf.addDynamicMatcher(nested_allocator_trait_matcher(), &m2);

    OnMatch<report, &report::match_negative_allocator_trait> m4(this);
    mf.addDynamicMatcher(allocator_trait_matcher(0), &m4);

    OnMatch<report, &report::match_positive_allocator_trait> m6(this);
    mf.addDynamicMatcher(allocator_trait_matcher(1), &m6);

    OnMatch<report, &report::match_dependent_allocator_trait> m5(this);
    mf.addDynamicMatcher(dependent_allocator_trait_matcher(), &m5);

    OnMatch<report, &report::match_class_using_allocator> m3(this);
    mf.addDynamicMatcher(class_using_allocator_matcher(), &m3);

    OnMatch<report, &report::match_should_return_by_value> m7(this);
    mf.addDynamicMatcher(should_return_by_value_matcher(), &m7);

    OnMatch<report, &report::match_ctor_expr> m1(this);
    mf.addDynamicMatcher(ctor_expr_matcher(), &m1);

    OnMatch<report, &report::match_ctor_init> m11(this);
    mf.addDynamicMatcher(ctor_init_matcher(), &m11);

    OnMatch<report, &report::match_return_stmt> m8(this);
    mf.addDynamicMatcher(return_stmt_matcher(), &m8);

    OnMatch<report, &report::match_var_decl> m9(this);
    mf.addDynamicMatcher(var_decl_matcher(), &m9);

    OnMatch<report, &report::match_ctor_decl> m10(this);
    mf.addDynamicMatcher(ctor_decl_matcher(), &m10);

    OnMatch<report, &report::match_allocator_method> m12(this);
    mf.addDynamicMatcher(allocator_method_matcher(), &m12);

    OnMatch<report, &report::match_allocator_member> m13(this);
    mf.addDynamicMatcher(allocator_member_matcher(), &m13);

    mf.match(*a.context()->getTranslationUnitDecl(), *a.context());

    check_not_forwarded(d.ctors_.begin(), d.ctors_.end());
    check_wrong_parm(d.cexprs_.begin(), d.cexprs_.end());
    check_uses_allocator(d.cexprs_.begin(), d.cexprs_.end());

    std::set<data::Returns::value_type, csabase::SortByLocation>
        sorted_returns(
            d.returns_.begin(), d.returns_.end(), csabase::SortByLocation(m));
    check_alloc_returns(sorted_returns.begin(), sorted_returns.end());

    std::set<data::Globals::value_type, csabase::SortByLocation>
        sorted_globals(
             d.globals_.begin(), d.globals_.end(), csabase::SortByLocation(m));
    check_globals_use_allocator(sorted_globals.begin(), sorted_globals.end());
}

void report::check_globals_use_allocator(data::Globals::const_iterator begin,
                                         data::Globals::const_iterator end)
{
    for (data::Globals::const_iterator itr = begin; itr != end; ++itr) {
        const VarDecl          *decl = *itr;
        const CXXConstructExpr *expr =
            llvm::dyn_cast<CXXConstructExpr>(decl->getInit());
        if (!is_allocator(expr->getType()) &&
            takes_allocator(expr->getType()) == a_Last) {
            unsigned n   = expr->getNumArgs();
            bool     bad = n == 0;
            if (n > 0) {
                const Expr *last = expr->getArg(n - 1);
                bool        result;
                if (last->isDefaultArgument() ||
                    (last->EvaluateAsBooleanCondition(result, *a.context()) &&
                     result == false)) {
                    bad = true;
                }
            }
            if (bad) {
                auto record = expr->getType()->getAsCXXRecordDecl();
                if (record && d.decls_with_false_allocator_trait_.count(
                                  record->getCanonicalDecl())) {
                    bad = false;
                }
            }
            if (bad) {
                a.report(decl, check_name, "GA01",
                         "Variable with global storage must be "
                         "initialized with non-default allocator");
            }
        }
    }
}

bool report::has_public_copy_constructor(const CXXRecordDecl *decl)
{
    if (!decl->hasDefinition() || !decl->hasUserDeclaredCopyConstructor()) {
        return true;                                                  // RETURN
    }

    for (auto c : decl->ctors()) {
        if (c->isCopyConstructor() &&
            c->getAccess() == AS_public &&
            !c->isDeleted()) {
            return true;                                              // RETURN
        }
    }

    return false;
}

bool report::should_transform(const CXXRecordDecl *record)
{
    if (a.is_component(record)) {
        llvm::StringRef to_transform =
            a.config()->value("allocator_transform", record->getLocation());
        llvm::StringRef name = record->getNameAsString();
        if (name.size()) {
            for (size_t colons = 0;; colons += 2) {
                if (contains_word(to_transform, name.drop_front(colons)) !=
                    to_transform.npos) {
                    return true;                                      // RETURN
                }
                if ((colons = name.find("::", colons)) == name.npos) {
                    break;
                }
            }
        }
    }
    return false;
}

const FieldDecl *report::has_array_member(const CXXRecordDecl *record)
{
    for (const auto *field : record->fields()) {
        if (field->getType()->isArrayType()) {
            return field;
        }
    }
    return 0;
}

void report::check_not_forwarded(data::Ctors::const_iterator begin,
                                 data::Ctors::const_iterator end)
{
    std::set<std::pair<bool, const CXXRecordDecl *>> records;
    std::set<std::pair<std::string, Range>>          processed;
    std::set<const CXXRecordDecl *>                  added_d_allocator;

    for (auto itr = begin; itr != end; ++itr) {
        const CXXConstructorDecl *decl = *itr;
        if (decl->isFunctionTemplateSpecialization()) {
            continue;
        }
        if (decl->isDeleted()) {
            continue;
        }
        const CXXRecordDecl *record = decl->getParent()->getCanonicalDecl();

        const FieldDecl *array_member = 0;
        bool do_transform = should_transform(record);
        if (do_transform) {
            if (0 != (array_member = has_array_member(record))) {
                do_transform = false;
            }
        }

        AllocatorLocation uses_allocator = takes_allocator(
            record->getTypeForDecl()->getCanonicalTypeInternal());
        if (uses_allocator == a_None && do_transform) {
            uses_allocator = a_Last;
        }

        std::pair<bool, const CXXRecordDecl *> rp = std::make_pair(
                                                               uses_allocator,
                                                               record);

        if (array_member && records.count(rp) == 0) {
            auto report = a.report(array_member, check_name, "WT01",
                     "Cannot transform %0 because it has an array member %1");
            report << record << array_member;
        }

        bool has_true_alloc_trait = d.decls_with_true_allocator_trait_.count(
                                                                        record);
        bool has_false_alloc_trait = d.decls_with_false_allocator_trait_.count(
                                                                        record);
        bool has_dependent_alloc_trait =
            !has_true_alloc_trait && !has_false_alloc_trait &&
            d.decls_with_dependent_allocator_trait_.count(record);
        auto *ts = llvm::dyn_cast<ClassTemplateSpecializationDecl>(record);
        if (ts && !ts->isExplicitSpecialization()) {
            const CXXRecordDecl *tr = ts->getSpecializedTemplate()
                                          ->getTemplatedDecl()
                                          ->getCanonicalDecl();
            if (uses_allocator && tr->hasDefinition()) {
                record = tr;
                rp.second = record;
            }
            if (d.decls_with_true_allocator_trait_.count(tr)) {
                has_true_alloc_trait = true;
            }
            if (d.decls_with_false_allocator_trait_.count(tr)) {
                has_false_alloc_trait = true;
            }
            if (d.decls_with_dependent_allocator_trait_.count(tr)) {
                has_dependent_alloc_trait = true;
            }
        }

        if (has_false_alloc_trait) {
            continue;
        }

        if (!has_true_alloc_trait &&
            !has_dependent_alloc_trait &&
            !do_transform &&
            is_allocator(QualType(record->getTypeForDecl(), 0))) {
            continue;
        }

        check_not_forwarded(decl);

        if (records.count(rp) == 0) {
            records.insert(rp);

            if (has_public_copy_constructor(record)) {
                if (!uses_allocator && has_true_alloc_trait) {
                    auto report = a.report(record, check_name, "AT01",
                             "Class %0 does not use allocators but has a "
                             "positive allocator trait");
                    report << record;
                }
                else if (uses_allocator &&
                         !has_true_alloc_trait &&
                         !has_dependent_alloc_trait) {
                    auto report = a.report(record, check_name, "AT02",
                             "Class %0 uses allocators but does not have an "
                             "allocator trait");
                    report << record;
                    if (do_transform) {
                        if (uses_allocator & a_Last) {
                            write_allocator_trait(record, true);
                        }
                        if (uses_allocator & a_Second) {
                            write_allocator_trait(record, false);
                        }
                    }
                }

                if (uses_allocator && record->hasDefinition()) {
                    const CXXBaseSpecifier *base_with_allocator = 0;
                    for (const auto& base : record->bases()) {
                        if (takes_allocator(
                                base.getType()->getCanonicalTypeInternal()) &&
                            d.allocator_methods_.count(
                                base.getType()
                                    ->getAsCXXRecordDecl()
                                    ->getCanonicalDecl())) {
                            base_with_allocator = &base;
                            break;
                        }
                    }
                    const FieldDecl *field_with_allocator = 0;
                    for (const auto *field : record->fields()) {
                        if (takes_allocator(
                                field->getType()
                                    ->getCanonicalTypeInternal()) &&
                            d.allocator_methods_.count(
                                field->getType()
                                    ->getAsCXXRecordDecl()
                                    ->getCanonicalDecl())) {
                            field_with_allocator = field;
                            break;
                        }
                    }

                    if (d.allocator_members_.count(record)) {
                        if (base_with_allocator || field_with_allocator) {
                            auto report = a.report(d.allocator_members_[record],
                                     check_name, "AP01",
                                     "Class %0 has unnecessary d_allocator_p");
                            report << record;
                            if (base_with_allocator) {
                                auto report_2 = a.report(base_with_allocator->getBeginLoc(),
                                         check_name, "AP01",
                                         "Use allocator of base class %0",
                                         false, DiagnosticIDs::Note);
                                report_2 << base_with_allocator->getType();
                            }
                            else {
                                auto report_2 = a.report(field_with_allocator,
                                         check_name, "AP01",
                                         "Use allocator of field %0",
                                         false, DiagnosticIDs::Note);
                                report_2 << field_with_allocator;
                            }
                        }
                    }
                    else if (!base_with_allocator &&
                             !field_with_allocator &&
                             !d.allocator_methods_.count(record)) {
                        auto report = a.report(record, check_name, "AP02",
                                 "Class %0 needs d_allocator_p member");
                        report << record;
                        if (do_transform) {
                            if (write_d_allocator_p_declaration(record)) {
                                added_d_allocator.insert(record);
                                if (!record->hasUserDeclaredCopyAssignment()) {
                                    //write_assignment_declaration(record);
                                    //write_assignment_definition(record);
                                    write_in_class_assignment_definition(
                                                                       record);
                                }
                            }
                        }
                    }

                    if (!d.allocator_methods_.count(record)) {
                        auto report = a.report(record, check_name, "AL01",
                                 "Class %0 needs allocator() method");
                        report << record;
                        if (do_transform) {
                            //write_allocator_method_declaration(
                            //                           record,
                            //                           uses_allocator,
                            //                           base_with_allocator,
                            //                           field_with_allocator);
                            //write_allocator_method_definition(
                            //                           record,
                            //                           uses_allocator,
                            //                           base_with_allocator,
                            //                           field_with_allocator);
                            write_in_class_allocator_method_definition(
                                                         record,
                                                         uses_allocator,
                                                         base_with_allocator,
                                                         field_with_allocator);
                        }
                    }
                }
            }
        }

        if (decl == decl->getCanonicalDecl() &&
            !decl->isMoveConstructor() &&
            uses_allocator &&
            !takes_allocator(decl)) {
            // Warn if the class does not have a constructor that matches
            // this one, but with a final allocator parameter or an initial
            // allocator_arg_t, allocator pair.

            bool found =  // Private copy constructor declarations are OK.
                decl->getAccess() == AS_private &&
                decl->isCopyOrMoveConstructor() &&
                decl->isUserProvided() && !decl->hasBody();

            unsigned num_parms = decl->getNumParams();
            for (auto ci = begin; !found && ci != end; ++ci) {
                const CXXConstructorDecl *ctor = *ci;
                if (ctor->isFunctionTemplateSpecialization()) {
                    continue;
                }
                auto r = ctor->getParent()->getCanonicalDecl();
                if (auto ts =
                        llvm::dyn_cast<ClassTemplateSpecializationDecl>(r)) {
                    r = ts->getSpecializedTemplate()
                            ->getTemplatedDecl()
                            ->getCanonicalDecl();
                }
                if (ctor == ctor->getCanonicalDecl() &&
                    ctor != decl &&
                    r == record) {
                    AllocatorLocation aloc = takes_allocator(ctor);
                    if (aloc == a_Last &&
                        ctor->getNumParams() >= num_parms + 1) {
                        found = true;
                        for (unsigned pi = 0; found && pi < num_parms; ++pi) {
                            auto d = decl->getParamDecl(pi)->getOriginalType();
                            auto c = ctor->getParamDecl(pi)->getOriginalType();
                            found = d->getCanonicalTypeUnqualified() ==
                                    c->getCanonicalTypeUnqualified();
                        }
                        for (unsigned pi = num_parms + 1;
                             found && pi < ctor->getNumParams(); ++pi) {
                            found = ctor->getParamDecl(pi)->hasDefaultArg();
                        }
                    }
                    if (aloc == a_Second &&
                        ctor->getNumParams() == num_parms + 2) {
                        found = true;
                        for (unsigned pi = 0; found && pi < num_parms; ++pi) {
                            auto d = decl->getParamDecl(pi)->getOriginalType();
                            auto c =
                                ctor->getParamDecl(pi + 2)->getOriginalType();
                            found = d->getCanonicalTypeUnqualified() ==
                                    c->getCanonicalTypeUnqualified();
                        }
                    }
                }
            }

            std::string type = decl->isDefaultConstructor() ? "default " :
                               decl->isCopyConstructor()    ? "copy "    :
                               decl->isMoveConstructor()    ? "move "    :
                                                              "";

            auto key = std::make_pair(std::string(type),
                                      Range(m, decl->getSourceRange()));
            if (!found && !processed.count(key)) {
                processed.insert(key);
                if (decl->isUserProvided()) {
                    auto report = a.report(decl, check_name, "AC01",
                             "Class %0 " + type +
                             "constructor has no allocator-aware version");
                    report << decl;
                }
                else {
                    auto report = a.report(decl, check_name, "AC02",
                             "Class %0 implicit " + type +
                             "constructor is not allocator-aware");
                    report << decl;
                }
                auto def = decl;
                bool default_allocator =
                    !decl->isUserProvided() || decl->isDefaultConstructor();
                if (!decl->isThisDeclarationADefinition()) {
                    if (do_transform) {
                        write_ctor_with_allocator_declaration(record, decl);
                    }
                    def = 0;
                    if (decl->isDefined()) {
                        def = llvm::dyn_cast<CXXConstructorDecl>(
                            decl->getDefinition());
                        default_allocator = false;
                    }
                }
                if (def) {
                    if (do_transform) {
                        write_ctor_with_allocator_definition(
                            record,
                            def,
                            added_d_allocator.count(record),
                            default_allocator);
                    }
                }
            }
        }
    }
}

void report::include(SourceLocation loc, llvm::StringRef name)
{
    for (auto &x : d.added_) {
        if (x.first == m.getFileID(loc) && x.second.count(name)) {
            return;
        }
    }

    FullSourceLoc ins_loc(loc, m);
    FileName ins(ins_loc.getFileEntry()->getName());

    for (const auto& f : a.attachment<IncludesData>().d_inclusions) {
        if (FileName(f.second.d_fe->getName()).name() == name) {
            FileName src(f.first.getFileEntry()->getName());
            if (src.name() == ins.name() ||
                a.is_component_header(src.name().str())) {
                d.added_[ins_loc.getFileID()].insert(name);
                return;
            }
        }
    }

    SourceLocation ip;
    bool insert_after = true;
    for (const auto& f : a.attachment<IncludesData>().d_inclusions) {
        if (f.first.getFileID() == m.getFileID(loc)) {
            if (insert_after) {
                ip = f.second.d_fullRange.getEnd();
                FileName fn(f.second.d_fe->getName());
                if (!a.is_component(fn.name().str()) && fn.name() > name) {
                    insert_after = false;
                    ip = f.second.d_fullRange.getBegin();
                    break;
                }
            }
        }
    }

    std::string header = "#include <" + name.str() + ">\n";
    if (!ip.isValid()) {
        SourceLocation l = m.getLocForStartOfFile(m.getFileID(loc));
        auto report = a.report(l, check_name, "AT02",
                 "Header needed for allocator trait\n%0",
                 false, DiagnosticIDs::Note);
        report << header;
        a.InsertTextBefore(l, header);
    }
    else if (insert_after) {
        SourceLocation l = a.get_line_range(ip).getEnd().getLocWithOffset(1);
        auto report = a.report(l, check_name, "AT02",
                 "Header needed for allocator trait\n%0",
                 false, DiagnosticIDs::Note);
        report << header;
        a.ReplaceText(l, 0, header);
    }
    else {
        SourceLocation l = a.get_line_range(ip).getBegin();
        auto report = a.report(l, check_name, "AT02",
                 "Header needed for allocator trait\n%0",
                 false, DiagnosticIDs::Note);
        report << header;
        a.InsertTextBefore(l, header);
    }

    d.added_[ins_loc.getFileID()].insert(name);
}

bool report::write_allocator_trait(const CXXRecordDecl *record, bool bslma)
    // Write out an allocator trait for the specified record.  If necessary
    // emit
    //    #include <bslmf_nestedtraitdeclaration.h>
    // If 'bslma' is true, then if necessary emit
    //    #include <bslma_usesbslmaallocator.h>
    // If 'bslma' is false, then if necessary emit
    //    #include <bslmf_usesallocatorargt.h>
    // If 'bslma' is true, emit within 'record'
    //    BSLMF_NESTED_TRAIT_DECLARATION(Record, bslma::UsesBslmaAllocator);
    // If 'bslma' is false, emit within 'record'
    //    BSLMF_NESTED_TRAIT_DECLARATION(Record, bslma::UsesAllocatorArgt);
{
    if (!a.is_component(record) || !record->hasDefinition())
      return false;

    record = record->getDefinition();

    if (a.config()->suppressed("AT02", record->getLocation()))
        return false;

    std::string s;
    llvm::raw_string_ostream ot(s);

    include(record->getLocation(), "bslmf_nestedtraitdeclaration.h");

    llvm::StringRef blp = "BloombergLP::";
    if (llvm::StringRef(record->getQualifiedNameAsString()).startswith(blp)) {
        blp = "";
    }

    llvm::StringRef header =
        bslma ? "bslma_usesbslmaallocator.h" : "bslmf_usesallocatorargt.h";
    llvm::StringRef trait =
        bslma ? "bslma::UsesBslmaAllocator" : "bslmf::UsesAllocatorArgT";

    include(record->getLocation(), header);

    SourceLocation ins_loc = record->getBraceRange().getEnd();
    llvm::StringRef range = a.get_source(record->getBraceRange());
    int end_spaces = range.size() - range.drop_back(1).rtrim().size() - 1;
    llvm::StringRef spaces =
        a.get_source_line(ins_loc).take_until([](char c) { return c != ' '; });

    ot << "\n"                                                << "\n" << spaces
       << "  public:"                                         << "\n" << spaces
       << "    // TRAITS"                                     << "\n" << spaces
       << "    BSLMF_NESTED_TRAIT_DECLARATION("
       <<          record->getName() << ", " << blp << trait << ");"
                                                              << "\n" << spaces
       ;

    auto report = a.report(ins_loc, check_name, "AT02",
             "Allocator trait for class %0%1",
             false, DiagnosticIDs::Note);

    report << record->getName() << ot.str();

    a.ReplaceText(ins_loc.getLocWithOffset(-end_spaces), end_spaces, ot.str());
    return true;
}

bool report::write_allocator_method_declaration(const CXXRecordDecl    *record,
                                                AllocatorLocation       kind,
                                                const CXXBaseSpecifier *base,
                                                const FieldDecl        *field)
{
    if (!a.is_component(record) || !record->hasDefinition())
        return false;

    if (!(kind & a_Last))
        return false;  // TBD: deal with get_allocator();

    record = record->getDefinition();

    if (a.config()->suppressed("AL01", record->getLocation()))
        return false;

    std::string s;
    llvm::raw_string_ostream ot(s);

    SourceLocation ins_loc = record->getBraceRange().getEnd();
    llvm::StringRef range = a.get_source(record->getBraceRange());
    int end_spaces = range.size() - range.drop_back(1).rtrim().size() - 1;
    llvm::StringRef spaces =
        a.get_source_line(ins_loc).take_until([](char c) { return c != ' '; });

    llvm::StringRef blp = "BloombergLP::";
    if (llvm::StringRef(record->getQualifiedNameAsString()).startswith(blp)) {
        blp = "";
    }

    ot << "\n"                                                << "\n" << spaces
       << "  public:"                                         << "\n" << spaces
       << "    // PUBLIC ACCESSORS"                           << "\n" << spaces
       << "    " << blp << "bslma::Allocator *allocator() const;"
                                                              << "\n" << spaces
       ;

    auto report = a.report(ins_loc, check_name, "AL01",
             "Allocator method declaration for class %0%1",
             false, DiagnosticIDs::Note);
    
    report << record->getName() << ot.str();
    a.ReplaceText(ins_loc.getLocWithOffset(-end_spaces), end_spaces, ot.str());
    return true;
}

bool report::write_allocator_method_definition(const CXXRecordDecl    *record,
                                               AllocatorLocation       kind,
                                               const CXXBaseSpecifier *base,
                                               const FieldDecl        *field)
{
    if (!a.is_component(record) || !record->hasDefinition())
        return false;

    if (!(kind & a_Last))
        return false;  // TBD: deal with get_allocator();

    record = record->getDefinition();

    if (a.config()->suppressed("AL01", record->getLocation()))
        return false;

    std::string s;
    llvm::raw_string_ostream ot(s);

    SourceLocation ins_loc = semiAfterRecordDecl(record);
    llvm::StringRef spaces =
        a.get_source_line(ins_loc).take_until([](char c) { return c != ' '; });

    llvm::StringRef blp = "BloombergLP::";
    if (llvm::StringRef(record->getQualifiedNameAsString()).startswith(blp)) {
        blp = "";
    }

    ot << "\n"                                                << "\n" << spaces
       << "// PUBLIC ACCESSORS"                               << "\n" << spaces
       << "inline"                                            << "\n" << spaces
       << blp << "bslma::Allocator *"
       << record->getName() << "::allocator() const {"        << "\n" << spaces
       << "    return "
       ;
    if (base) {
        auto f =
            d.allocator_methods_
                [base->getType()->getAsCXXRecordDecl()->getCanonicalDecl()];
        std::string name = a.get_source(base->getSourceRange()).trim().str();
        name = prune(name, "virtual");
        name = prune(name, "public");
        name = prune(name, "private");
        name = prune(name, "protected");
        name = prune(name, "class");
        name = prune(name, "struct");
        name = prune(name, "union");
        ot << name << "::"
           << (f->getName() == "allocator" ? "allocator()"
                                           : "get_allocator().mechanism()");
    }
    else if (field) {
        auto f =
            d.allocator_methods_
                [field->getType()->getAsCXXRecordDecl()->getCanonicalDecl()];
        ot << field->getName() << "."
           << (f->getName() == "allocator" ? "allocator()"
                                           : "get_allocator().mechanism()");
    }
    else {
        ot << "d_allocator_p";
    }
    ot << ";"                                                 << "\n" << spaces
       << "}"                                                 << "\n" << spaces
       ;

    auto report = a.report(ins_loc, check_name, "AL01",
             "Allocator method definition for class %0%1",
             false, DiagnosticIDs::Note);
    report << record->getName() << ot.str();
    a.ReplaceText(ins_loc.getLocWithOffset(1), 0, ot.str());
    return true;
}

bool report::write_in_class_allocator_method_definition(
                                                const CXXRecordDecl    *record,
                                                AllocatorLocation       kind,
                                                const CXXBaseSpecifier *base,
                                                const FieldDecl        *field)
{
    if (!a.is_component(record) || !record->hasDefinition())
        return false;

    if (!(kind & a_Last))
        return false;  // TBD: deal with get_allocator();

    record = record->getDefinition();

    if (a.config()->suppressed("AL01", record->getLocation()))
        return false;

    std::string s;
    llvm::raw_string_ostream ot(s);

    SourceLocation ins_loc = record->getBraceRange().getEnd();
    llvm::StringRef range = a.get_source(record->getBraceRange());
    int end_spaces = range.size() - range.drop_back(1).rtrim().size() - 1;
    llvm::StringRef spaces =
        a.get_source_line(ins_loc).take_until([](char c) { return c != ' '; });

    llvm::StringRef blp = "BloombergLP::";
    if (llvm::StringRef(record->getQualifiedNameAsString()).startswith(blp)) {
        blp = "";
    }

    ot << "\n"                                                << "\n" << spaces
       << "  public:"                                         << "\n" << spaces
       << "    // PUBLIC ACCESSORS"                           << "\n" << spaces
       << "    " << blp << "bslma::Allocator *allocator() const {"
                                                              << "\n" << spaces
       << "        return "
       ;
    if (base) {
        auto f =
            d.allocator_methods_
                [base->getType()->getAsCXXRecordDecl()->getCanonicalDecl()];
        std::string name = a.get_source(base->getSourceRange()).trim().str();
        name = prune(name, "virtual");
        name = prune(name, "public");
        name = prune(name, "private");
        name = prune(name, "protected");
        name = prune(name, "class");
        name = prune(name, "struct");
        name = prune(name, "union");
        ot << name << "::"
           << (f->getName() == "allocator" ? "allocator()"
                                           : "get_allocator().mechanism()");
    }
    else if (field) {
        auto f =
            d.allocator_methods_
                [field->getType()->getAsCXXRecordDecl()->getCanonicalDecl()];
        ot << field->getName() << "."
           << (f->getName() == "allocator" ? "allocator()"
                                           : "get_allocator().mechanism()");
    }
    else {
        ot << "d_allocator_p";
    }
    ot << ";"                                                 << "\n" << spaces
       << "    }"                                             << "\n" << spaces
       ;

    auto report = a.report(ins_loc, check_name, "AL01",
             "In-class allocator method definition for class %0%1",
             false, DiagnosticIDs::Note);
    report << record->getName() << ot.str();
    a.ReplaceText(ins_loc.getLocWithOffset(-end_spaces), end_spaces, ot.str());
    return true;
}

bool report::write_d_allocator_p_declaration(const CXXRecordDecl *record)
{
    if (!a.is_component(record) || !record->hasDefinition())
      return false;

    record = record->getDefinition();

    if (a.config()->suppressed("AP02", record->getLocation()))
        return false;

    std::string s;
    llvm::raw_string_ostream ot(s);

    SourceLocation ins_loc = record->getBraceRange().getEnd();
    llvm::StringRef range = a.get_source(record->getBraceRange());
    int end_spaces = range.size() - range.drop_back(1).rtrim().size() - 1;
    llvm::StringRef spaces =
        a.get_source_line(ins_loc).take_until([](char c) { return c != ' '; });

    llvm::StringRef blp = "BloombergLP::";
    if (llvm::StringRef(record->getQualifiedNameAsString()).startswith(blp)) {
        blp = "";
    }

    ot << "\n"                                                << "\n" << spaces
       << "  private:"                                        << "\n" << spaces
       << "    // PRIVATE DATA"                               << "\n" << spaces
       << "    " << blp << "bslma::Allocator *d_allocator_p = 0;"
                                                              << "\n" << spaces
       ;

    auto report = a.report(ins_loc, check_name, "AP02",
             "Allocator member declaration for class %0%1",
             false, DiagnosticIDs::Note);
    report << record->getName() << ot.str();
    a.ReplaceText(ins_loc.getLocWithOffset(-end_spaces), end_spaces, ot.str());
    return true;
}

bool report::write_ctor_with_allocator_definition(
                                   const CXXRecordDecl      *record,
                                   const CXXConstructorDecl *decl,
                                   bool                      init_allocator,
                                   bool                      default_allocator)
{
    if (!a.is_component(record))
        return false;

    bool up = decl->isUserProvided();

    record = record->getDefinition();

    if (a.config()->suppressed(up ? "AC01" : "AC02", record->getLocation()))
        return false;

    llvm::StringRef range = a.get_source(record->getBraceRange());

    std::string s;
    llvm::raw_string_ostream ot(s);

    SourceLocation  ins_loc;
    int             end_spaces;
    llvm::StringRef indent;

    llvm::StringRef blp = "BloombergLP::";
    if (llvm::StringRef(record->getQualifiedNameAsString()).startswith(blp)) {
        blp = "";
    }

    const FunctionTemplateDecl *ft =
        decl->getNumTemplateParameterLists() ? 0 :
        decl->getDescribedFunctionTemplate();

    if (up) {
        ins_loc    = ft ? ft->getBeginLoc() : decl->getBeginLoc();
        end_spaces = 0;
    }
    else {
        ins_loc    = record->getBraceRange().getEnd();
        end_spaces = range.size() - range.drop_back(1).rtrim().size() - 1;
        indent     = "    ";
    }
    llvm::StringRef spaces =
        a.get_source_line(ins_loc).take_until([](char c) { return c != ' '; });

    static llvm::StringRef accesses[] = {
        "public", "protected", "private", "public"};
    StringRef access = accesses[decl->getAccess()];

    if (!up) {
        ot << "\n"                                            << "\n" << spaces
           << "  " << access << ":"                           << "\n" << spaces
           << "    // " << access.upper() << " CREATORS"      << "\n" << spaces
           ;
    }

    if (ft) {
        ot << indent << a.get_source(SourceRange(
                  ft->getBeginLoc(),
                  decl->getBeginLoc().getLocWithOffset(-1))).rtrim()
                                                              << "\n" << spaces
           ;
    }
    if (up) {
        ot << indent
           << a.get_source(SourceRange(decl->getBeginLoc(),
                                       decl->getNameInfo().getEndLoc()))
           << '(';
    }
    else {
        ot << indent << record->getName() << '(';
    }
    if (decl->getNumParams() == 0) {
    }
    else if (up) {
        ot << a.get_source(SourceRange(
                  decl->parameters().front()->getSourceRange().getBegin(),
                  decl->parameters().back()->getSourceRange().getEnd()))
           << ", "
           ;
    }
    else if (decl->isCopyConstructor()) {
        ot << "const " << record->getName() << "& original, "
           ;
    }
    else if (decl->isMoveConstructor()) {
        ot << record->getName() << "&& original, "
           ;
    }
    else {
        return false;
    }
    ot << blp << "bslma::Allocator *basicAllocator";
    if (default_allocator) {
        ot << " = 0";
    }
    ot << ")"                                                 << "\n" << spaces
       ;
    char sep = ':';
    for (auto init : decl->inits()) {
        bool pass_allocator = false;
        std::string name;
        if (auto b = init->getBaseClass()) {
            name = b->getCanonicalTypeInternal().getAsString();
            name = prune(name, "class");
            name = prune(name, "struct");
            name = prune(name, "union");
            if (takes_allocator(b->getCanonicalTypeInternal())) {
                pass_allocator = true;
            }
        }
        if (auto f = init->getMember()) {
            name = f->getNameAsString();
            if (takes_allocator(f->getType())) {
                pass_allocator = true;
            }
        }
        if (init->isWritten()) {
            if (auto ce = llvm::dyn_cast<CXXConstructExpr>(init->getInit())) {
                ot << indent << sep << ' '
                   << a.get_source(SourceRange(
                          init->getSourceRange().getBegin(),
                          init->getRParenLoc().getLocWithOffset(-1)));
                if (pass_allocator) {
                    if (ce->getNumArgs() > 0) {
                        ot << ", ";
                    }
                    ot << "basicAllocator";
                }
                ot << ")"                                     << "\n" << spaces
                   ;
            }
            else {
                ot << indent << sep << ' '
                   << a.get_source(init->getSourceRange())    << "\n" << spaces
                   ;
            }
            sep = ',';
        }
        else if (pass_allocator) {
            ot << indent << sep << ' ' << name << '(';
            if (decl->isCopyOrMoveConstructor()) {
                ot << "original";
                if (init->getMember()) {
                    ot << '.' << name;
                }
                ot << ", ";
            }
            ot << "basicAllocator)"                           << "\n" << spaces
               ;
            sep = ',';
        }
        else if (decl->isCopyOrMoveConstructor()) {
            if (name == "d_allocator_p") {
                init_allocator = true;
            }
            else {
                ot << indent << sep << ' ' << name << "(original";
                if (init->getMember()) {
                    ot << '.' << name;
                }
                ot << ")"                                     << "\n" << spaces
                   ;
                sep = ',';
            }
        }
    }
    if (init_allocator) {
        ot << indent << sep
           << " d_allocator_p(basicAllocator)"                << "\n" << spaces
           ;
        sep = ',';
    }
    ot << indent
       << (up && decl->doesThisDeclarationHaveABody()
               ? a.get_source(decl->getBody()->getSourceRange())
               : "{ }") << "\n"                               << "\n" << spaces
       ;

    auto report = a.report(ins_loc, check_name, up ? "AC01" : "AC02",
             "Definition for version with allocator\n%1%0",
             false, DiagnosticIDs::Note);
    
    report << ot.str() << spaces;
    a.ReplaceText(ins_loc.getLocWithOffset(-end_spaces), end_spaces, ot.str());
    return true;
}

bool report::write_ctor_with_allocator_declaration(
                                              const CXXRecordDecl      *record,
                                              const CXXConstructorDecl *decl)
{
    if (!a.is_component(record) || !record->hasDefinition())
        return false;

    bool up = decl->isUserProvided();

    if (!up || decl->isThisDeclarationADefinition())
        return false;

    record = record->getDefinition();

    if (a.config()->suppressed(up ? "AC01" : "AC02", record->getLocation()))
        return false;

    llvm::StringRef blp = "BloombergLP::";
    if (llvm::StringRef(record->getQualifiedNameAsString()).startswith(blp)) {
        blp = "";
    }

    std::string s;
    llvm::raw_string_ostream ot(s);

    SourceLocation ins_loc = record->getBraceRange().getEnd();
    llvm::StringRef range = a.get_source(record->getBraceRange());
    int end_spaces = range.size() - range.drop_back(1).rtrim().size() - 1;
    llvm::StringRef spaces =
        a.get_source_line(ins_loc).take_until([](char c) { return c != ' '; });
    static llvm::StringRef accesses[] = {
        "public", "protected", "private", "public"};
    StringRef access = accesses[decl->getAccess()];

    ot << "\n"                                                << "\n" << spaces
       << "  " << access << ":"                               << "\n" << spaces
       << "    // " << access.upper() << " CREATORS"          << "\n" << spaces
       ;
    if (const auto *ft = decl->getDescribedFunctionTemplate()) {
        ot << "    "
           << a.get_source(SourceRange(
                  ft->getBeginLoc(),
                  decl->getBeginLoc().getLocWithOffset(-1))).rtrim()
                                                              << "\n" << spaces
           ;
    }
    if (decl->getNumParams() == 0) {
        ot << "    explicit " << record->getName()
           << "(" << blp << "bslma::Allocator *basicAllocator);"
                                                              << "\n" << spaces
           ;
    }
    else if (up) {
        ot << "    " << record->getName() << "("
           << a.get_source(SourceRange(
                  decl->parameters().front()->getSourceRange().getBegin(),
                  decl->parameters().back()->getSourceRange().getEnd()))
           << ", " << blp << "bslma::Allocator *basicAllocator";
        if (decl->isDefaultConstructor()) {
            ot << " = 0";
        }
        ot << ");"                                            << "\n" << spaces
           ;
    }
    else if (decl->isCopyConstructor()) {
        ot << "    " << record->getName()
           << "(const " << record->getName() << "& original, "
           << "" << blp << "bslma::Allocator *basicAllocator = 0);"
                                                              << "\n" << spaces
           ;
    }
    else if (decl->isMoveConstructor()) {
        ot << "    " << record->getName()
           << "(" << record->getName() << "&& original, "
           << "" << blp << "bslma::Allocator *basicAllocator = 0);"
                                                              << "\n" << spaces
           ;
    }
    else {
        return false;
    }

    auto report = a.report(ins_loc, check_name, up ? "AC01" : "AC02",
             "Declaration for version with allocator%0",
             false, DiagnosticIDs::Note);
    report << ot.str();
    a.ReplaceText(ins_loc.getLocWithOffset(-end_spaces), end_spaces, ot.str());
    return true;
}

bool report::write_assignment_declaration(const CXXRecordDecl *record)
{
    if (!a.is_component(record) || !record->hasDefinition())
        return false;

    record = record->getDefinition();

    std::string s;
    llvm::raw_string_ostream ot(s);

    SourceLocation ins_loc = record->getBraceRange().getEnd();
    llvm::StringRef range = a.get_source(record->getBraceRange());
    int end_spaces = range.size() - range.drop_back(1).rtrim().size() - 1;
    llvm::StringRef spaces =
        a.get_source_line(ins_loc).take_until([](char c) { return c != ' '; });

    ot << "\n"                                                << "\n" << spaces
       << "  public:"                                         << "\n" << spaces
       << "    // PUBLIC MANIPULATORS"                        << "\n" << spaces
       << "    " << record->getName() << "& operator=(" << "const "
                 << record->getName() << "& original);"       << "\n" << spaces
       ;

    auto report = a.report(ins_loc, check_name, "AH01",
             "Declaration for assignment operator%0",
             false, DiagnosticIDs::Note);
    report << ot.str();
    a.ReplaceText(ins_loc.getLocWithOffset(-end_spaces), end_spaces, ot.str());
    return true;
}

bool report::write_assignment_definition(const CXXRecordDecl *record)
{
    if (!a.is_component(record) || !record->hasDefinition())
        return false;

    record = record->getDefinition();

    std::string s;
    llvm::raw_string_ostream ot(s);

    SourceLocation ins_loc = semiAfterRecordDecl(record);
    llvm::StringRef spaces =
        a.get_source_line(ins_loc).take_until([](char c) { return c != ' '; });

    llvm::StringRef rn = record->getName();

    ot << "\n"                                                << "\n" << spaces
       << "// PUBLIC MANIPULATORS"                            << "\n" << spaces
       << "inline"                                            << "\n" << spaces
       << rn << "& "
       << rn << "::operator=(const "
       << rn << "& original) {"                               << "\n" << spaces
       ;
    for (const auto& base : record->bases()) {
        llvm::StringRef bn = base.getType()->getAsCXXRecordDecl()->getName();
        ot << "    static_cast<" << bn << "&>(*this) = original;"
                                                              << "\n" << spaces
        ;
    }
    for (const auto *field : record->fields()) {
        if (field->getType()->isArrayType()) {
            auto report = a.report(record, check_name, "AH01",
                     "Assignment for %0 not generated due to array member %1",
                     false, DiagnosticIDs::Note);
            report  << record << field;
            return false;
        }
        llvm::StringRef fn = field->getName();
        ot << "    this->" << fn << " = " << "original." << fn << ";"
                                                              << "\n" << spaces
        ;
    }
    ot << "    return *this;"                                 << "\n" << spaces
       << "}"                                                 << "\n" << spaces
       ;

    auto report = a.report(ins_loc, check_name, "AH01",
             "Definition for assignment operator%0",
             false, DiagnosticIDs::Note);
    report << ot.str();
    a.ReplaceText(ins_loc.getLocWithOffset(1), 0, ot.str());
    return true;
}

bool report::write_in_class_assignment_definition(const CXXRecordDecl *record)
{
    if (!a.is_component(record) || !record->hasDefinition())
        return false;

    record = record->getDefinition();

    std::string s;
    llvm::raw_string_ostream ot(s);

    SourceLocation ins_loc = record->getBraceRange().getEnd();
    llvm::StringRef range = a.get_source(record->getBraceRange());
    int end_spaces = range.size() - range.drop_back(1).rtrim().size() - 1;
    llvm::StringRef spaces =
        a.get_source_line(ins_loc).take_until([](char c) { return c != ' '; });

    ot << "\n"                                                << "\n" << spaces
       << "  public:"                                         << "\n" << spaces
       << "    // PUBLIC MANIPULATORS"                        << "\n" << spaces
       << "    " << record->getName() << "& operator=(" << "const "
                 << record->getName() << "& original) {"      << "\n" << spaces
       ;
    for (const auto& base : record->bases()) {
        llvm::StringRef bn = base.getType()->getAsCXXRecordDecl()->getName();
        ot << "        static_cast<" << bn << "&>(*this) = original;"
                                                              << "\n" << spaces
        ;
    }
    for (const auto *field : record->fields()) {
        if (field->getType()->isArrayType()) {
            auto report = a.report(record, check_name, "AH01",
                     "Assignment for %0 not generated due to array member %1",
                     false, DiagnosticIDs::Note);
            
            report << record << field;
            return false;
        }
        llvm::StringRef fn = field->getName();
        ot << "        this->" << fn << " = " << "original." << fn << ";"
                                                              << "\n" << spaces
        ;
    }
    ot << "        return *this;"                             << "\n" << spaces
       << "    }"                                             << "\n" << spaces
       ;

    auto report = a.report(ins_loc, check_name, "AH01",
             "In-class definition for assignment operator%0",
             false, DiagnosticIDs::Note);
    report << ot.str();
    a.ReplaceText(ins_loc.getLocWithOffset(-end_spaces), end_spaces, ot.str());
    return true;
}

void report::check_not_forwarded(const CXXConstructorDecl *decl)
{
    if (!decl->hasBody()) {
        return;                                                       // RETURN
    }

    AllocatorLocation aloc = takes_allocator(decl);

    if (aloc == a_None) {
        return;                                                       // RETURN
    }

    // The allocator parameter is the last one (but skip over default args) or
    // the second one.
    const ParmVarDecl *palloc = 0;
    if (aloc == a_Last) {
        for (unsigned n = decl->getNumParams(); n > 0; --n) {
            palloc = decl->getParamDecl(n - 1);
            if (is_allocator(palloc->getType())) {
                break;
            }
            if (!palloc->hasDefaultArg()) {
                palloc = decl->getParamDecl(decl->getNumParams() - 1);
                break;
            }
        }
    }
    else {
        palloc = decl->getParamDecl(1);
    }

    // Iterate through the base and member initializers and report those
    // which take an allocator parameter that we do not pass.

    check_not_forwarded(decl->init_begin(), decl->init_end(), palloc);
}

void report::check_not_forwarded(CXXConstructorDecl::init_const_iterator begin,
                                 CXXConstructorDecl::init_const_iterator end,
                                 const ParmVarDecl* palloc)
{
    while (begin != end) {
        check_not_forwarded(*begin++, palloc);
    }
}

void report::check_not_forwarded(const CXXCtorInitializer* init,
                                 const ParmVarDecl* palloc)
{
    auto ctor_expr = llvm::dyn_cast<CXXConstructExpr>(init->getInit());

    if (!ctor_expr) {
        return;                                                       // RETURN
    }

    AllocatorLocation aloc = takes_allocator(ctor_expr->getConstructor());

    if (a_Second == aloc ||
        (a_Last == aloc && last_arg_is_explicit_allocator(ctor_expr))) {
        // The allocator is explicitly passed last, or it's passed second
        // (where it would not be optional).
        return;                                                       // RETURN
    }

    // Type of object being initialized.
    const Type *type = init->isBaseInitializer() ?
                           init->getBaseClass() :
                       init->isAnyMemberInitializer() ?
                           init->getAnyMember()->getType().getTypePtr() :
                           nullptr;

    if (!type || !takes_allocator(type->getCanonicalTypeInternal())) {
        return;                                                       // RETURN
    }

    auto rd = get_record_decl(type->getCanonicalTypeInternal());
    while (rd) {
        rd = rd->getCanonicalDecl();
        if (d.decls_with_false_allocator_trait_.count(rd)) {
            return;                                                   // RETURN
        }
        rd = rd->getTemplateInstantiationPattern();
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
        auto report = a.report(loc, check_name, "MA01",
                 "Allocator not passed to base %0");
        report << init->getBaseClass()->getCanonicalTypeInternal().getAsString()
               << range;
    } else {
        auto report = a.report(loc, check_name, "MA02",
                 "Allocator not passed to member %0");
        
        report << init->getAnyMember()->getNameAsString()
               << range;
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

            if (llvm::dyn_cast<CXXTemporaryObjectExpr>(arg)) {
                // Actual temporary object requested in the code.
                break;
            }

            if (const MaterializeTemporaryExpr* mte =
                           llvm::dyn_cast<MaterializeTemporaryExpr>(arg)) {
                arg = mte->getSubExpr();
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

            if (is_allocator(arg->getType())) {
                auto report = a.report(arg->getExprLoc(), check_name, "AM01",
                         "Allocator argument initializes "
                         "non-allocator %0 of type '%1' rather than "
                         "allocator %2");
                report << parm_name(wrongp, n - 1)
                       << wrongp->getType().getAsString()
                       << parm_name(allocp, n)
                       << arg->getSourceRange();
            }

            break;  // Done.
        }
    }
}

template <typename Iter>
void report::check_uses_allocator(Iter begin, Iter end)
{
    while (begin != end) {
        check_uses_allocator(*begin++);
    }
}

void report::check_uses_allocator(const CXXConstructExpr *expr)
{
    unsigned n = expr->getNumArgs();

    if (n == 0) {
        return;
    }

    const CXXConstructorDecl *decl = expr->getConstructor();

    AllocatorLocation aloc = takes_allocator(decl);

    if (decl->getNumParams() != n || a_None == aloc) {
        return;
    }

    if (d.cinits_.find(expr) != d.cinits_.end()) {
        return;
    }

    const Expr *arg = expr->getArg(aloc == a_Last ? n - 1 : 1);

    if (arg->isDefaultArgument()) {
        return;
    }

    // Descend into the expression, looking for a conversion from an allocator.
    // We use a loop because elements of the descent can repeat.
    for (;;) {
        arg = arg->IgnoreImpCasts();

        if (llvm::dyn_cast<CXXTemporaryObjectExpr>(arg)) {
            // Actual temporary object requested in the code.
            break;
        }

        if (const MaterializeTemporaryExpr *mte =
                llvm::dyn_cast<MaterializeTemporaryExpr>(arg)) {

            arg = mte->getSubExpr();
            continue;
        }

        if (const CXXBindTemporaryExpr *bte =
                llvm::dyn_cast<CXXBindTemporaryExpr>(arg)) {
            arg = bte->getSubExpr();
            continue;
        }

        if (const CXXConstructExpr *ce =
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

            if (i > 0) {
                continue;
            }
        }

        // At this point, we should have stripped off all the outer layers of
        // the argument expression which are performing the conversion to the
        // parameter type, and have the inner expression with its actual type.
        // If that type is bslma::Allocator* (specifically, to eliminate cases
        // such as passing &testAllocator), report the use if it is not a
        // parameter, a null pointer, or a call to a non-member function.

        if (is_allocator(arg->getType(), false)) {
            if (auto dre = llvm::dyn_cast<DeclRefExpr>(arg)) {
                if (llvm::dyn_cast<ParmVarDecl>(dre->getDecl())) {
                    // E.g., X x(basicAllocator);
                    break;
                }
            }
            if (arg->IgnoreCasts()->isNullPointerConstant(
                    *a.context(), arg->NPC_ValueDependentIsNotNull)) {
                // E.g., X x((bslma::Allocator *)0);
                break;
            }
            if (llvm::dyn_cast<CallExpr>(arg) &&
                !llvm::dyn_cast<CXXMemberCallExpr>(arg)) {
                // E.g., X x(bslma::Default::allocator());
                break;
            }
            auto report = a.report(arg->getExprLoc(), check_name, "AU01",
                     "Verify whether allocator use is appropriate");

            report << arg->getSourceRange();
        }

        break;  // Done.
    }
}

template <typename Iter>
void report::check_alloc_returns(Iter begin, Iter end)
{
    SourceLocation loc;
    for (Iter itr = begin; itr != end; ++itr) {
        const ReturnStmt *stmt = *itr;
        if (loc != stmt->getReturnLoc()) {
            check_alloc_return(stmt);
            loc = stmt->getReturnLoc();
        }
    }
}

void report::check_alloc_return(const ReturnStmt *stmt)
{
    if (   stmt->getRetValue()
        && !stmt->getRetValue()->getType()->isPointerType()
        && d.decls_with_true_allocator_trait_.count(
            get_record_decl(stmt->getRetValue()->getType()))) {
        const FunctionDecl* func = a.get_parent<FunctionDecl>(stmt);
        if (!func || !func->getReturnType()->isReferenceType()) {
            a.report(stmt, check_name, "AR01",
                     "Type using allocator is returned by value");
        }
    }
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver&)
    // Create a callback within the specified 'analyser' which will be invoked
    // after a translation unit has been processed.
{
    analyser.onTranslationUnitDone += report(analyser);
}

}  // close anonymous namespace

static RegisterCheck c1(check_name, &subscribe);

// ----------------------------------------------------------------------------
// Copyright (C) 2014 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
