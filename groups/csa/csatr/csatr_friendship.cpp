// csatr_friendship.cpp                                               -*-C++-*-

#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclFriend.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/TemplateName.h>
#include <clang/AST/Type.h>
#include <clang/AST/TypeLoc.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchersInternal.h>
#include <clang/Basic/SourceLocation.h>
#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_filenames.h>
#include <csabase_format.h>
#include <csabase_location.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_util.h>
#include <llvm/Support/Casting.h>
#include <string>

using namespace clang;
using namespace clang::ast_matchers;
using namespace csabase;

// ----------------------------------------------------------------------------

static std::string const check_name("local-friendship-only");

// ----------------------------------------------------------------------------

namespace
{

struct data
{
};

struct report : Report<data>
{
    INHERIT_REPORT_CTOR(report, Report, data);

    std::string context(const Decl *decl);
    bool is_extern_type(Type const* type);
    const Decl *get_definition(const Decl *decl);
    const Decl *other(const Decl *frend, const Decl *decl);
    bool is_good_friend(const FriendDecl *frend, const NamedDecl *def);
    void local_friendship_only(FriendDecl const* decl);
    void operator()();
};

std::string report::context(const Decl *decl)
{
    return "";
}

bool report::is_extern_type(Type const* type)
{
    CXXRecordDecl const* record(type->getAsCXXRecordDecl());
    Location cloc(a.get_location(record));
    return (type->isIncompleteType()
            && record->getLexicalDeclContext()->isFileContext())
        || !a.is_component(cloc.file());
}

const Decl *report::get_definition(const Decl *decl)
{
    if (auto d = llvm::dyn_cast<RecordDecl>(decl)) {
        return d->getDefinition();
    }
    if (auto d = llvm::dyn_cast<VarDecl>(decl)) {
        return d->getDefinition();
    }
    if (auto d = llvm::dyn_cast<TagDecl>(decl)) {
        return d->getDefinition();
    }
    if (auto d = llvm::dyn_cast<ClassTemplateDecl>(decl)) {
        if (d->isThisDeclarationADefinition()) {
            return d;
        }
    }
    if (auto d = llvm::dyn_cast<FunctionTemplateDecl>(decl)) {
        if (d->isThisDeclarationADefinition()) {
            return d;
        }
    }
    if (auto d = llvm::dyn_cast<VarTemplateDecl>(decl)) {
        if (d->isThisDeclarationADefinition()) {
            return d;
        }
    }
    return 0;
}

const Decl *report::other(const Decl *frend, const Decl *decl)
{
    for (auto d = decl->redecls_begin(); d != decl->redecls_end(); ++d) {
        auto def = get_definition(*d);
        if (def) {
            return def;
        }
    }
    FileID fid = m.getFileID(frend->getLocation());
    for (auto d = decl->redecls_begin(); d != decl->redecls_end(); ++d) {
        if (*d != decl && m.getFileID((*d)->getLocation()) == fid) {
            return *d;
        }
    }
    for (auto d = decl->redecls_begin(); d != decl->redecls_end(); ++d) {
        if (*d != decl) {
            return *d;
        }
    }
    return 0;
}

bool report::is_good_friend(const FriendDecl *frend, const NamedDecl *def)
{
    std::string fn = llvm::dyn_cast<NamedDecl>(frend->getDeclContext())
                         ->getQualifiedNameAsString();
    std::string dn = def->getQualifiedNameAsString();

    FileName ff(m.getFilename(frend->getLocation()));
    FileName df(m.getFilename(def->getLocation()));

    if (llvm::StringRef(dn).startswith_lower(fn)) {
        return true;
    }

    if (auto rd = llvm::dyn_cast<RecordDecl>(def)) {
        if (rd != rd->getDefinition()) {
            if (rd->getLexicalDeclContext()->isFileContext()) {
                return false;
            }
        }
    }

    if (ff.component() == df.component()) {
        return true;
    }

    if (a.is_standard_namespace(fn) || a.is_standard_namespace(dn)) {
        return true;
    }

    return false;
}

void report::local_friendship_only(FriendDecl const* frend)
{
    const Decl *def = frend->getFriendDecl();
    const TypeSourceInfo *tsi = frend->getFriendType();
    const Type *type = tsi ? tsi->getTypeLoc().getTypePtr() : 0;
    if (type && type->isElaboratedTypeSpecifier()) {
        type = type->getAs<ElaboratedType>()->getNamedType().getTypePtr();
        if (auto spec = type->getAs<TemplateSpecializationType>()) {
            def = spec->getTemplateName().getAsTemplateDecl();
        }
    }

    const char *tag;

    if (def) {
        if (auto method = llvm::dyn_cast<CXXMethodDecl>(def)) {
            tag = "Friendship to a method";
            def = method->getParent();
        }
        else if (llvm::dyn_cast<FunctionDecl>(def)) {
            tag = "Friendship to a function";
        }
        else if (llvm::dyn_cast<FunctionTemplateDecl>(def)) {
            tag = "Friendship to a function template";
        }
        else if (llvm::dyn_cast<ClassTemplateDecl>(def)) {
            tag = "Friendship to a class template";
        }
        else {
            tag = "Friendship";
        }
        if (auto d = other(frend, def)) {
            def = d;
        }
    }
    else {
        tag = "Friendship to a class";
        auto rd = type->getAsCXXRecordDecl();
        while (rd && !(def = other(frend, rd))) {
            rd = llvm::dyn_cast<CXXRecordDecl>(rd->getParent());
        }
    }

    if (!def) {
        def = frend->getFriendDecl();
        if (!def && type) {
            def = type->getAsCXXRecordDecl();
        }
    }

    if (!def || !is_good_friend(frend, llvm::dyn_cast<NamedDecl>(def))) {
        auto r = a.report(frend->getFriendLoc(), check_name, "TR19",
                 "%0 can only be granted within a component");
        r << frend->getSourceRange()
          << tag;
    }
}

void report::operator()()
{
    MatchFinder mf;
    OnMatch<> m1([&](const BoundNodes &nodes) {
        local_friendship_only(nodes.getNodeAs<FriendDecl>("friend"));
    });
    mf.addDynamicMatcher(
        decl(forEachDescendant(cxxRecordDecl(
            unless(classTemplateSpecializationDecl(anything())),
            forEach(friendDecl(anything()).bind("friend"))
        ))), &m1);
    mf.addDynamicMatcher(
        decl(forEachDescendant(cxxRecordDecl(
            isExplicitTemplateSpecialization(),
            forEach(friendDecl(anything()).bind("friend"))
        ))), &m1);
    mf.addDynamicMatcher(
        decl(forEachDescendant(classTemplateDecl(
            forEach(friendDecl(anything()).bind("friend"))
        ))), &m1);
    mf.match(*a.context()->getTranslationUnitDecl(), *a.context());
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    analyser.onTranslationUnitDone += report(analyser);
}

}

// ----------------------------------------------------------------------------

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
