// csaaq_transitiveincludes.cpp                                       -*-C++-*-

#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/PrettyPrinter.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Type.h>
#include <clang/Basic/IdentifierTable.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/MacroInfo.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Token.h>
#include <clang/Tooling/Refactoring.h>
#include <csabase_analyser.h>
#include <csabase_config.h>
#include <csabase_debug.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_filenames.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_util.h>
#include <csabase_visitor.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Regex.h>
#include <llvm/Support/raw_ostream.h>
#include <stddef.h>
#include <cctype>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <vector>
#include <tuple>
namespace clang { class FileEntry; }
namespace clang { class MacroArgs; }
namespace clang { class Module; }
namespace csabase { class Visitor; }

using namespace csabase;
using namespace clang::tooling;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("transitive-includes");

// ----------------------------------------------------------------------------

namespace
{

struct data
    // Data attached to analyzer for this check.
{
    data();
        // Create an object of this type.

    std::vector<std::string>                             d_file_stack;
    std::vector<FileID>                                  d_fileid_stack;
    llvm::StringRef                                      d_guard;
    SourceLocation                                       d_guard_pos;
    std::set<std::pair<std::string, SourceLocation>>     d_std_names;
    std::map<FileID, std::map<std::string, SourceLocation>>
                                                         d_once;
    std::map<FileID,
             std::vector<std::tuple<std::string, SourceLocation, bool> > >
                                                         d_includes;
    std::map<FileID, llvm::StringRef>                    d_guards;
    std::map<std::tuple<FileID, FileID, SourceLocation>, SourceLocation>
                                                         d_fid_map;
};

data::data()
{
}

struct report : public RecursiveASTVisitor<report>
{
    report(Analyser& analyser,
           PPObserver::CallbackType type = PPObserver::e_None);
        // Create an object of this type, that will use the specified
        // 'analyser'.  Optionally specify a 'type' to identify the callback
        // that will be invoked, for preprocessor callbacks that have the same
        // signature.

    void clear_guard();
        // Clear the current include guard info.

    void set_guard(llvm::StringRef guard, SourceLocation where);
        // Set the guard info to the specified 'guard' and 'where'.

    void push_include(FileID fid, llvm::StringRef name, SourceLocation sl);
        // Mark that the specified 'name' is included at the specified 'sl'
        // in the specified 'fid', and in files which include it if not
        // dependent on BSL_OVERRIDES_STD.

    void change_include(FileID fid, llvm::StringRef name);
        // Change the file name added most recently by push_include for the
        // specified 'fid', and in files which include it if not dependent on
        // BSL_OVERRIDES_STD, to the specified 'name'.

    void operator()(SourceLocation   where,
                    const Token&     inc,
                    llvm::StringRef  name,
                    bool             angled,
                    CharSourceRange  namerange,
                    const FileEntry *entry,
                    llvm::StringRef  path,
                    llvm::StringRef  relpath,
                    const Module    *imported);
        // Preprocessor callback for included file.

    void map_file(std::string name);
        // Find a possibly different file to include that includes the
        // specified 'name'.

    void operator()(SourceLocation                now,
                    PPCallbacks::FileChangeReason reason,
                    SrcMgr::CharacteristicKind    type,
                    FileID                        prev);
        // Preprocessor callback for file changed.

    bool is_named(Token const& token, llvm::StringRef name);
        // Return 'true' iff the specified 'token' is the specified 'name'.

    void operator()(Token const&          token,
                    MacroDirective const *md);
        // Preprocessor callback for macro definition.

    void operator()(Token const&          token,
                    MacroDirective const *md,
                    SourceRange           range,
                    MacroArgs const      *);
        // Preprocessor callback for macro expanding.

    void operator()(SourceLocation        where,
                    const Token&          token,
                    const MacroDirective *md);
        // Preprocessor callback for 'ifdef'/'ifndef'.

    void operator()(const Token&          token,
                    const MacroDirective *md,
                    SourceRange           range);
        // Preprocessor callback for 'defined(.)'.

    void operator()(SourceRange range);
        // Preprocessor callback for skipped ranges.

    void operator()(SourceLocation where,
                    SourceRange    condition,
                    bool           value);
        // Preprocessor callback for 'if'.

    void operator()(SourceLocation where,
                    SourceRange    condition,
                    bool           value,
                    SourceLocation ifloc);
        // Preprocessor callback for 'elif'.

    void operator()(SourceLocation     where,
                    SourceLocation     ifloc);
        // Preprocessor callback for 'else'.

    void require_file(std::string     name,
                      SourceLocation  sl,
                      llvm::StringRef symbol);
        // Indicate that the specified file 'name' is needed at the specified
        // 'sl' in order to obtain the specified 'symbol'.

    void inc_for_std_decl(llvm::StringRef  r,
                          SourceLocation   sl,
                          const Decl      *ds);
        // For the specified name 'r' at location 'sl' referenced by the
        // specified declaration 'ds', determine which header file, if any, is
        // needed.

    const NamedDecl *look_through_typedef(const Decl *ds);
        // If the specified 'ds' is a typedef for a record, return the
        // definition for the record if it exists.  Return null otherwise.

    void operator()();
        // Callback for end of main file.

    bool is_guard(llvm::StringRef guard);
    bool is_guard(const Token& token);
        // Return true if the specified 'guard' or 'token' looks like a header
        // guard ("INCLUDED_...").

    bool is_guard_for(llvm::StringRef guard, llvm::StringRef file);
    bool is_guard_for(const Token& token, llvm::StringRef file);
    bool is_guard_for(llvm::StringRef guard, SourceLocation sl);
    bool is_guard_for(const Token& token, SourceLocation sl);
        // Return true if the specified 'guard' or 'token' is a header guard
        // for the specified 'file' or 'sl'.

    bool VisitDeclRefExpr(DeclRefExpr *expr);
    bool VisitCXXConstructExpr(CXXConstructExpr *expr);
    bool VisitTypeLoc(TypeLoc tl);
    bool VisitUnresolvedLookupExpr(UnresolvedLookupExpr *expr);
        // Return true after processing the specified 'tl' and 'expr'.

    Analyser&                d_analyser;
    data&                    d_data;
    PPObserver::CallbackType d_type;
};

report::report(Analyser& analyser, PPObserver::CallbackType type)
: d_analyser(analyser)
, d_data(analyser.attachment<data>())
, d_type(type)
{
}

void report::clear_guard()
{
    d_data.d_guard = "";
    d_data.d_guard_pos = SourceLocation();
}

void report::set_guard(llvm::StringRef guard, SourceLocation where)
{
    d_data.d_guard = guard;
    d_data.d_guard_pos = d_analyser.get_line_range(where).getBegin();
}

void report::push_include(FileID fid, llvm::StringRef name, SourceLocation sl)
{
    SourceManager& m = d_analyser.manager();
    bool in_header = d_analyser.is_component_header(m.getFilename(sl));
    for (FileID f : d_data.d_fileid_stack) {
        if (f == fid) {
            d_data.d_includes[f].push_back(std::make_tuple(
                name, d_analyser.get_line_range(sl).getBegin(), true));
        }
        else if (in_header && f == m.getMainFileID()) {
            SourceLocation sfl = sl;
            auto t = std::make_tuple(fid, f, sl);
            auto i = d_data.d_fid_map.find(t);
            if (i != d_data.d_fid_map.end()) {
                sfl = i->second;
            }
            else {
                FileID flid;
                while (sfl.isValid()) {
                    flid = m.getFileID(sfl);
                    if (flid == f) {
                        break;
                    }
                    sfl = m.getIncludeLoc(flid);
                }
                if (sfl.isValid()) {
                    unsigned offset = m.getFileOffset(sfl);
                    unsigned line = m.getLineNumber(flid, offset);
                    if (line > 1) {
                        SourceLocation prev =
                            m.translateLineCol(flid, line - 1, 0);
                        llvm::StringRef p = d_analyser.get_source_line(prev);
                        static llvm::Regex guard("^ *# *ifn?def  *INCLUDED_");
                        if (guard.match(p)) {
                            sfl = prev;
                        }
                    }
                }
                d_data.d_fid_map[t] = sfl;
            }
            d_data.d_includes[f].push_back(std::make_tuple(
                name, d_analyser.get_line_range(sfl).getBegin(), false));
        }
    }
}

// InclusionDirective
void report::operator()(SourceLocation   where,
                        const Token&     inc,
                        llvm::StringRef  name,
                        bool             angled,
                        CharSourceRange  namerange,
                        const FileEntry *entry,
                        llvm::StringRef  path,
                        llvm::StringRef  relpath,
                        const Module    *imported)
{
    SourceManager& m = d_analyser.manager();
    FileID fid = m.getFileID(where);
    if (d_data.d_guard_pos.isValid() &&
        fid != m.getFileID(d_data.d_guard_pos)) {
        clear_guard();
    }

    push_include(
        fid, name, d_data.d_guard_pos.isValid() ? d_data.d_guard_pos : where);

    clear_guard();
}

// FileChanged
void report::operator()(SourceLocation                now,
                        PPCallbacks::FileChangeReason reason,
                        SrcMgr::CharacteristicKind    type,
                        FileID                        prev)
{
    std::string name = d_analyser.manager().getPresumedLoc(now).getFilename();
    if (reason == PPCallbacks::EnterFile) {
        d_data.d_file_stack.push_back(name);
        d_data.d_fileid_stack.push_back(d_analyser.manager().getFileID(now));
    } else if (reason == PPCallbacks::ExitFile) {
        if (d_data.d_file_stack.size() > 0) {
            d_data.d_file_stack.pop_back();
        }
        if (d_data.d_fileid_stack.size() > 0) {
            d_data.d_fileid_stack.pop_back();
        }
    }
}

bool report::is_named(Token const& token, llvm::StringRef name)
{
    return token.isAnyIdentifier() &&
           token.getIdentifierInfo()->getName() == name;
}

// MacroDefined
// MacroUndefined
void report::operator()(Token const&          token,
                        MacroDirective const *md)
{
}

// MacroExpands
void report::operator()(Token const&          token,
                        MacroDirective const *md,
                        SourceRange           range,
                        MacroArgs const      *)
{
    llvm::StringRef macro = token.getIdentifierInfo()->getName();
    const MacroInfo *mi = md->getMacroInfo();
    SourceManager& m = d_analyser.manager();
    Location loc(m, mi->getDefinitionLoc());
    if (loc && !range.getBegin().isMacroID()) {
        require_file(loc.file(), range.getBegin(), macro);
    }
}

bool report::is_guard(llvm::StringRef guard)
{
    return guard.startswith("INCLUDED_");
}

bool report::is_guard(const Token& token)
{
    return token.isAnyIdentifier() &&
           is_guard(token.getIdentifierInfo()->getName());
}

bool report::is_guard_for(llvm::StringRef guard, llvm::StringRef file)
{
    if (!is_guard(guard)) {
        return false;                                                 // RETURN
    }

    FileName fn(file);
    std::string s = "INCLUDED_" + fn.component().upper();
    for (char& c : s) {
        if (!std::isalnum(c)) {
            c = '_';
        }
    }
    return s == guard || s + "_" + fn.extension().substr(1).upper() == guard;
}

bool report::is_guard_for(const Token& token, llvm::StringRef file)
{
    return token.isAnyIdentifier() &&
           is_guard_for(token.getIdentifierInfo()->getName(), file);
}

bool report::is_guard_for(llvm::StringRef guard, SourceLocation sl)
{
    return is_guard_for(guard, d_analyser.manager().getFilename(sl));
}

bool report::is_guard_for(const Token& token, SourceLocation sl)
{
    return is_guard_for(token, d_analyser.manager().getFilename(sl));
}

// Ifdef
// Ifndef
void report::operator()(SourceLocation        where,
                        const Token&          token,
                        const MacroDirective *)
{
    SourceManager& m = d_analyser.manager();
    llvm::StringRef tn = token.getIdentifierInfo()->getName();

    clear_guard();

    if (!m.isInSystemHeader(where) && is_guard(token)) {
        set_guard(tn, where);
    }
}

// Defined
void report::operator()(const Token&          token,
                        const MacroDirective *,
                        SourceRange           range)
{
    clear_guard();

    llvm::StringRef tn = token.getIdentifierInfo()->getName();

    if (!d_analyser.manager().isInSystemHeader(range.getBegin()) &&
        is_guard(token)) {
        set_guard(token.getIdentifierInfo()->getName(), range.getBegin());
    }
}

// SourceRangeSkipped
void report::operator()(SourceRange range)
{
    SourceManager& m = d_analyser.manager();
    Location loc(m, range.getBegin());
    if (d_data.d_guard.size() > 0 &&
        !m.isInSystemHeader(range.getBegin())) {
        std::string gs = d_data.d_guard.str();
        llvm::StringRef g = gs;
        std::string rs("def +(" + g.str() + ")[[:space:]]+" +
                       "# *include +<(" + g.drop_front(9).lower() +
                       "[.]?h?)>[[:space:]]+(# *define +" + g.str() +
                       "[[:space:]]*)?");
        llvm::Regex r(rs);
        llvm::StringRef source = d_analyser.get_source(range);
        llvm::SmallVector<llvm::StringRef, 7> matches;
        if (r.match(source, &matches)) {
            FileID fid = m.getFileID(range.getBegin());
            push_include(fid,
                         matches[2],
                         d_data.d_guard_pos.isValid() ? d_data.d_guard_pos :
                                                        range.getBegin());
        }
        clear_guard();
    }
}

// If
void report::operator()(SourceLocation where,
                        SourceRange    condition,
                        bool           value)
{
    clear_guard();
}

// Elif
void report::operator()(SourceLocation where,
                        SourceRange    condition,
                        bool           value,
                        SourceLocation ifloc)
{
    clear_guard();
}

// Else/Endif
void report::operator()(SourceLocation where, SourceLocation ifloc)
{
    if (d_type == PPObserver::e_Else) {
        clear_guard();
    }
    else {
    }
}

const NamedDecl *report::look_through_typedef(const Decl *ds)
{
    const TypedefDecl *td;
    const CXXRecordDecl *rd;
    if ((td = llvm::dyn_cast<TypedefDecl>(ds)) &&
        (rd = td->getUnderlyingType().getTypePtr()->getAsCXXRecordDecl()) &&
        rd->hasDefinition()) {
        return rd->getDefinition();
    }
    return 0;
}

void report::require_file(std::string     name,
                          SourceLocation  sl,
                          llvm::StringRef symbol)
{
    SourceManager& m = d_analyser.manager();
    SourceLocation orig_sl = sl;

    sl = m.getExpansionLoc(sl);

    FileID fid = m.getFileID(sl);
    while (m.isInSystemHeader(sl)) {
        sl = m.getIncludeLoc(fid);
        fid = m.getDecomposedIncludedLoc(fid).first;
    }

    FileName ff(m.getFilename(sl));
    FileName fn(name);
    name = fn.name();

    if (name == ff.name()) {
        return;
    }

    for (const auto& p : d_data.d_includes[fid]) {
        if (std::get<0>(p) == name &&
            (!std::get<1>(p).isValid() ||
             !m.isBeforeInTranslationUnit(sl, std::get<1>(p)))) {
            return;
        }
    }

    if (!d_data.d_once[fid].count(name) ||
        m.isBeforeInTranslationUnit(sl, d_data.d_once[fid][name])) {
        d_data.d_once[fid][name] = sl;
        d_analyser.report(sl, check_name, "AQK01",
                          "Need #include <%0> for symbol %1")
            << name
            << symbol;
    }
}

void report::inc_for_std_decl(llvm::StringRef  r,
                              SourceLocation   sl,
                              const Decl      *ds)
{
    sl = d_analyser.manager().getExpansionLoc(sl);
    FileID fid = d_analyser.manager().getFileID(sl);

    for (const Decl *d = ds; d; d = look_through_typedef(d)) {
        bool skip = false;
        for (const Decl *p = d; !skip && p; p = p->getPreviousDecl()) {
            Location loc(d_analyser.manager(), p->getLocation());
            FileName fn(loc.file());
            Decl::redecl_iterator rb = p->redecls_begin();
            Decl::redecl_iterator re = p->redecls_end();
            for (; !skip && rb != re; ++rb) {
                if (rb->getLocation().isValid() &&
                    d_analyser.manager().isBeforeInTranslationUnit(
                        rb->getLocation(), sl)) {
                    Location loc(d_analyser.manager(), rb->getLocation());
                    if (!skip && loc) {
                        require_file(loc.file(), sl, r);
                        skip = true;
                    }
                }
            }
            const UsingDecl *ud = llvm::dyn_cast<UsingDecl>(p);
            if (!skip && ud) {
                auto sb = ud->shadow_begin();
                auto se = ud->shadow_end();
                for (; !skip && sb != se; ++sb) {
                    const UsingShadowDecl *usd = *sb;
                    for (auto u = usd; !skip && u;
                         u = u->getPreviousDecl()) {
                        inc_for_std_decl(r, sl, u);
                    }
                }
            }
        }
    }
}

bool isNamespace(const DeclContext *dc, llvm::StringRef ns)
{
    for (;;) {
        if (!dc->isNamespace()) {
            return false;
        }
        const NamespaceDecl *nd = llvm::cast<NamespaceDecl>(dc);
        if (nd->isInline()) {
            dc = nd->getParent();
        } else if (!dc->getParent()->getRedeclContext()->isTranslationUnit()) {
            return false;
        } else {
            const IdentifierInfo *ii = nd->getIdentifier();
            return ii && ii->getName() == ns;
        }
    }
}

bool report::VisitDeclRefExpr(DeclRefExpr *expr)
{
    SourceLocation sl = expr->getExprLoc();
    if (sl.isValid() && !d_analyser.manager().isInSystemHeader(sl)) {
        const NamedDecl *ds = expr->getFoundDecl();
        const DeclContext *dc = ds->getDeclContext();
        std::string name = expr->getNameInfo().getName().getAsString();
        while (dc->isRecord()) {
            name = llvm::dyn_cast<NamedDecl>(dc)->getNameAsString();
            dc = dc->getParent();
        }
        if (dc->isTranslationUnit() ||
            dc->isExternCContext() ||
            dc->isExternCXXContext() ||
            isNamespace(dc, "std")) {
            d_data.d_std_names.insert(std::make_pair(name, sl));
        }
        else if (isNamespace(dc, "bsl")) {
            inc_for_std_decl(name, sl, ds);
        }
    }
    return true;
}

bool report::VisitCXXConstructExpr(CXXConstructExpr *expr)
{
    SourceLocation sl = expr->getExprLoc();
    if (sl.isValid() && !d_analyser.manager().isInSystemHeader(sl)) {
        const NamedDecl *ds = expr->getConstructor()->getParent();
        const DeclContext *dc = ds->getDeclContext();
        std::string name = ds->getNameAsString();
        while (dc->isRecord()) {
            name = llvm::dyn_cast<NamedDecl>(dc)->getNameAsString();
            dc = dc->getParent();
        }
        if (dc->isTranslationUnit() ||
            dc->isExternCContext() ||
            dc->isExternCXXContext() ||
            isNamespace(dc, "std")) {
            d_data.d_std_names.insert(std::make_pair(name, sl));
        }
        else if (isNamespace(dc, "bsl")) {
            inc_for_std_decl(name, sl, ds);
        }
    }
    return true;
}

bool report::VisitTypeLoc(TypeLoc tl)
{
    const Type *type = tl.getTypePtr();
    if (type->getAs<TypedefType>() || !type->isBuiltinType()) {
        SourceLocation sl =
            d_analyser.manager().getExpansionLoc(tl.getBeginLoc());
        PrintingPolicy pp(d_analyser.context()->getLangOpts());
        pp.SuppressTagKeyword = true;
        pp.SuppressInitializers = true;
        pp.TerseOutput = true;
        std::string r = QualType(type, 0).getAsString(pp);
        NamedDecl *ds = d_analyser.lookup_name(r);
        if (!ds) {
            if (const TypedefType *tt = type->getAs<TypedefType>()) {
                ds = tt->getDecl();
            }
            else {
                tl.getTypePtr()->isIncompleteType(&ds);
            }
        }
        if (ds &&
            sl.isValid() &&
            !d_analyser.manager().isInSystemHeader(sl)) {
            inc_for_std_decl(r, sl, ds);
        }
    }
    return true;
}

bool report::VisitUnresolvedLookupExpr(UnresolvedLookupExpr *expr)
{
    SourceLocation sl = expr->getExprLoc();
    NestedNameSpecifier *nns = expr->getQualifier();
    if (sl.isValid() &&
        !d_analyser.manager().isInSystemHeader(sl) &&
        nns &&
        nns->getKind() == NestedNameSpecifier::Namespace &&
        nns->getAsNamespace()->getNameAsString() == "bsl") {
        std::string r = "bsl::" + expr->getName().getAsString();
        if (const Decl *ds = d_analyser.lookup_name(r)) {
            inc_for_std_decl(r, sl, ds);
        }
    }
    return true;
}

// TranslationUnitDone
void report::operator()()
{
    TraverseDecl(d_analyser.context()->getTranslationUnitDecl());

    for (const auto& rp : d_data.d_std_names) {
        llvm::StringRef r = rp.first;
        SourceLocation sl = rp.second;
        Location loc(d_analyser.manager(), sl);

        if (d_analyser.manager().isInSystemHeader(sl)) {
            continue;
        }

        if (const Decl *ds = d_analyser.lookup_name(("std::" + r).str())) {
            inc_for_std_decl(r, sl, ds);
        }
    }
}

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
    // Hook up the callback functions.
{
    observer.onPPInclusionDirective += report(analyser,
                                                observer.e_InclusionDirective);
    observer.onPPFileChanged        += report(analyser,
                                                       observer.e_FileChanged);
    observer.onPPMacroDefined       += report(analyser,
                                                      observer.e_MacroDefined);
    observer.onPPMacroUndefined     += report(analyser,
                                                    observer.e_MacroUndefined);
    observer.onPPMacroExpands       += report(analyser,
                                                      observer.e_MacroExpands);
    observer.onPPIfdef              += report(analyser, observer.e_Ifdef);
    observer.onPPIfndef             += report(analyser, observer.e_Ifndef);
    observer.onPPDefined            += report(analyser, observer.e_Defined);
    observer.onPPSourceRangeSkipped += report(analyser,
                                                observer.e_SourceRangeSkipped);
    observer.onPPIf                 += report(analyser,  observer.e_If);
    observer.onPPElif               += report(analyser, observer.e_Elif);
    observer.onPPElse               += report(analyser, observer.e_Else);
    observer.onPPEndif              += report(analyser, observer.e_Endif);
    analyser.onTranslationUnitDone  += report(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static RegisterCheck c1(check_name, &subscribe);

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
