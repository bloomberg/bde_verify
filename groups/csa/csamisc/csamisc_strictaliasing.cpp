// csamisc_strictaliasing.cpp                                         -*-C++-*-

#include <clang/AST/ExprCXX.h>
#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <string>
#include <map>

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("strict-alias");

// ----------------------------------------------------------------------------

CanQualType getType(QualType type)
{
    QualType pt = type->getPointeeType();
    if (!pt.isNull()) {
        type = pt;
    }
    if (type->isArrayType()) {
        type = QualType(type->getBaseElementTypeUnsafe(), 0);
    }
    return type->getCanonicalTypeUnqualified();
}

bool is_gen(QualType type)
{
    if (const BuiltinType *bt = type->getAs<BuiltinType>())
    {
        switch (bt->getKind()) {
          case BuiltinType::Void:
          case BuiltinType::Char_U:
          case BuiltinType::Char_S:
          case BuiltinType::UChar:
            return true;                                              // RETURN
          default:
            break;
        }
    }
    return false;
}

bool are_signed_variations(QualType q1, QualType q2)
{
#undef  KK
#define KK(a, b) ((a) * (BuiltinType::LastKind + 1) + (b))
#undef  caseKK
#define caseKK(a, b) \
    case KK(BuiltinType::a, BuiltinType::b): \
    case KK(BuiltinType::b, BuiltinType::a)

    auto bt1 = q1->getAs<BuiltinType>();
    auto bt2 = q2->getAs<BuiltinType>();
    if (bt1 && bt2) {
        if (bt1->getKind() == bt2->getKind()) {
            return true;                                              // RETURN
        }
        switch (KK(bt1->getKind(), bt2->getKind())) {
          caseKK(Char_U, UChar):
          caseKK(Char_U, Char_S):
          caseKK(Char_U, SChar):
          caseKK(UChar, Char_S):
          caseKK(UChar, SChar):
          caseKK(Char_S, SChar):
          caseKK(WChar_U, WChar_S):
          caseKK(UShort, Short):
          caseKK(UInt, Int):
          caseKK(ULong, Long):
          caseKK(ULongLong, LongLong):
          caseKK(UInt128, Int128):
            return true;                                              // RETURN
          default:
            break;
        }
    }
    return false;

#undef  KK
#undef  caseKK
}

bool is_derived(QualType qs, QualType qt)
{
    auto rs = qs->getAsCXXRecordDecl();
    auto rt = qt->getAsCXXRecordDecl();
    return rs && rs->hasDefinition() &&
           rt && rt->hasDefinition() &&
           (rt->isDerivedFrom(rs) || rs->isDerivedFrom(rt));
}

static void check(Analyser& analyser, ExplicitCastExpr const *expr)
{
    //ERRS(); expr->getExprLoc().dump(analyser.manager());
    //llvm::errs() << " "; expr->dump(); ERNL();

    if (expr->getCastKind() != CK_BitCast &&
        expr->getCastKind() != CK_LValueBitCast &&
        expr->getCastKind() != CK_NoOp) {
        return;                                                       // RETURN
    }

    if (expr->getSubExpr()->isNullPointerConstant(
            *analyser.context(), Expr::NPC_ValueDependentIsNotNull)) {
        return;                                                       // RETURN
    }

    QualType qs = expr->getSubExpr()->IgnoreParenImpCasts()->getType();
    QualType qt = expr->getTypeAsWritten();

    QualType qns(qs.getNonReferenceType()->getBaseElementTypeUnsafe(), 0);
    QualType qnt(qt.getNonReferenceType()->getBaseElementTypeUnsafe(), 0);

    QualType cqs = qns->getCanonicalTypeUnqualified();
    QualType cqt = qnt->getCanonicalTypeUnqualified();

    QualType pqs = cqs->getPointeeType();
    if (!pqs.isNull()) {
        pqs = QualType(pqs->getBaseElementTypeUnsafe(), 0);
    }
    QualType pqt = cqt->getPointeeType();
    if (!pqt.isNull()) {
        pqt = QualType(pqt->getBaseElementTypeUnsafe(), 0);
    }

    bool target_is_ref =
        qt->isReferenceType() || expr->getCastKind() == CK_LValueBitCast;

    //ERRS() << "Ref: " << target_is_ref << "\n" << "src: ";
    //cqs.dump(); llvm::errs() << "\ntgt: "; cqt.dump(); ERNL();

    bool bad = false;

    if (cqs == cqt) {
        //ERRS() << bad; ERNL();
    }
    else if (target_is_ref) {
        if (cqt->isPointerType()) {
            bad = !is_gen(pqt) &&
                  (!cqs->isPointerType() || !are_signed_variations(pqs, pqt));
            //ERRS() << bad; ERNL();
        }
        else if (cqs->isPointerType()) {
            bad = !is_gen(cqt);
            //ERRS() << bad; ERNL();
        }
        else {
            bad = !are_signed_variations(cqs, cqt) &&
                  !is_gen(cqs) && !is_gen(cqt);
            //ERRS() << bad; ERNL();
        }
    }
    else if (cqt->isPointerType() && cqs->isPointerType()) {
        bad = !are_signed_variations(pqs, pqt) &&
              !is_gen(pqs) &&
              !is_gen(pqt) &&
              !is_derived(pqs, pqt);
        //ERRS() << bad; ERNL(); pqs.dump(); ERNL(); pqt.dump(); ERNL();
    }
    else {
        //ERRS() << bad; ERNL();
    }

    if (bad) {
        analyser.report(expr, check_name, "AL01",
                        "Possible strict-aliasing violation")
            << expr->getSourceRange();
    }
}

// ----------------------------------------------------------------------------

static void checkCCast(Analyser& analyser, CStyleCastExpr const *expr)
{
    check(analyser, expr);
}

static void
checkReinterpretCast(Analyser& analyser,
                     CXXReinterpretCastExpr const *expr)
{
    check(analyser, expr);
}

static RegisterCheck register_check0(check_name, checkCCast);
static RegisterCheck register_check1(check_name, checkReinterpretCast);

// ----------------------------------------------------------------------------
// Copyright (C) 2014 Bloomberg Finance L.P.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------- END-OF-FILE ----------------------------------
