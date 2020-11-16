// csaxform_refactor.cpp                                              -*-C++-*-

#include "csabase_analyser.h"
#include "csabase_debug.h"
#include "csabase_filenames.h"
#include "csabase_location.h"
#include "csabase_ppobserver.h"
#include "csabase_registercheck.h"
#include "csabase_report.h"
#include "csabase_util.h"
#include "csabase_visitor.h"

#include "csaglb_includes.h"

#include <llvm/ADT/Hashing.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Regex.h>

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>

#include <string>
#include <unordered_map>
#include <unordered_set>

using namespace csabase;
using namespace clang;

#ifdef DEBUG_RAV
#include "csabase_debug_rav.h"
#define RecursiveASTVisitor DebugRecursiveASTVisitor
#else
namespace { struct Guard { Guard(...) { } }; }
#endif

// ----------------------------------------------------------------------------

static std::string const check_name("refactor");

// ----------------------------------------------------------------------------

namespace
{

struct data
{
    struct hash {
        unsigned operator()(llvm::StringRef s) const
        { return llvm::hash_value(s); }
        unsigned operator()(SourceLocation sl) const
        { return sl.getRawEncoding(); }
        unsigned operator()(FileID fid) const
        { return fid.getHashValue(); }
    };

    std::unordered_map<std::string              /* original     */,
                       std::vector<std::string> /* replacements */> d_files;
        // Include files to be replaced; each time one of the key files is
        // found to be included, it is removed and replaced by all of the files
        // in the value vector (which may be empty).

    std::unordered_map<std::string              /* file      */,
                       std::vector<SourceRange> /* locations */> d_includes;
        // Locations where file is included, including those elided by external
        // include guards.  If a file is included within a guarded region, the
        // range starts on the '#ifndef' line and ends on the '#endif' line.
        // Otherwise the range starts and ends on the '#include' line.

    std::unordered_map<llvm::StringRef /* old  */,
                       llvm::StringRef /* new  */,
                       hash            /* hash */> d_replacements;
        // Mapping of names to their replacements.
};

// Callback object invoked upon completion.
struct report : public RecursiveASTVisitor<report>, Report<data>
{
    INHERIT_REPORT_CTOR(report, Report, data);

    typedef RecursiveASTVisitor<report> base;

    void operator()();
        // Callback for end of compilation unit.

    void operator()(Token const &,
                    const MacroDefinition&,
                    SourceRange,
                    MacroArgs const *);
        // Callback for macro expansion

    void operator()(SourceLocation,
                    PPCallbacks::FileChangeReason,
                    SrcMgr::CharacteristicKind,
                    FileID);
        // Callback for file changed.

    bool TraverseDeclRefExpr(DeclRefExpr *);
    bool TraverseElaboratedTypeLoc(ElaboratedTypeLoc);
    bool TraverseNestedNameSpecifierLoc(NestedNameSpecifierLoc);
    bool TraverseQualifiedTypeLoc(QualifiedTypeLoc);
    bool TraverseTypeLoc(TypeLoc);
    bool TraverseTemplateSpecializationTypeLoc(TemplateSpecializationTypeLoc);
    bool VisitCXXRecordDecl(CXXRecordDecl *);

    llvm::StringRef clean_for_replace(llvm::StringRef s, std::string& buf);
        // Clean and return the specified 's' by removing a leading enterprise
        // name and/or extra '::'s.  The returned reference may point into the
        // specified 'buf' if sufficient manipulation is needed.

    bool replace_class(SourceRange sr, QualType t);
        // If the type represented by the specified 't' is replaceable, change
        // the specified range 'sr' to its replacement.  Return true if a
        // substitution is made.

    bool replace(SourceRange sr, llvm::StringRef e);
        // If the name represented by the specified 'e' is replaceable, change
        // the specified range 'sr' to its replacement.  Return true if a
        // substitution is made.
};

void report::operator()()
{
    auto tu = a.context()->getTranslationUnitDecl();

    for (auto &id : a.attachment<IncludesData>().d_inclusions) {
        d.d_includes[a.get_source(id.second.d_file).str()].emplace_back(
            id.second.d_fullRange);
    }

    std::unordered_map<FileID,
                       std::unordered_set<llvm::StringRef, data::hash>,
                       data::hash> by_id;
    for (auto &includes : d.d_includes) {
        for (auto range : includes.second) {
            by_id[m.getFileID(range.getBegin())].insert(includes.first);
        }
    }
    for (auto &include_locations : d.d_includes) {
        auto replace_iter = d.d_files.find(include_locations.first);
        if (replace_iter != d.d_files.end()) {
            for (auto &replace_range : include_locations.second) {
                if (!a.is_component(replace_range.getBegin())) {
                    continue;
                }
                FileID ifid = m.getFileID(replace_range.getBegin());
                std::string rep_text = "";
                std::string sep = "";
                for (const auto& replacement : replace_iter->second) {
                    bool already_included = by_id[ifid].count(replacement);
                    if (!already_included) {
                        rep_text += sep + "#include <" + replacement + ">";
                        sep = "\n";
                        by_id[ifid].insert(replacement);
                    }
                }
                SourceRange extended_range(
                    a.get_line_range(replace_range.getBegin()).getBegin(),
                    a.get_line_range(replace_range.getEnd()).getEnd());
                if (a.ReplaceText(extended_range, rep_text)) {
                    a.report(extended_range.getBegin(), check_name, "RF01",
                             "Replacing " +
                             on_one_line(a.get_source(extended_range), true) +
                             " with " + on_one_line(rep_text, true));
                }
            }
        }
    }

    TraverseDecl(tu);
}

void report::operator()(SourceLocation loc,
                        PPCallbacks::FileChangeReason reason,
                        SrcMgr::CharacteristicKind kind,
                        FileID prev)
{
    if (loc == m.getLocForStartOfFile(m.getMainFileID()) &&
        reason == PPCallbacks::EnterFile &&
        kind == SrcMgr::C_User &&
        prev.isInvalid()) {
        // First time through.  Configure refactor data.
        llvm::StringRef cfg = a.config()->value("refactor");
        SmallVector<llvm::StringRef, 5> rs;
        llvm::Regex rre("^(file|name)[(][^()]+(,[^()]+)*[)]");
        while (rre.match(cfg, &rs)) {
            llvm::StringRef r = rs[0].trim();
            llvm::StringRef o = r;
            cfg = cfg.drop_front(rs[0].size()).trim();
            bool bad = true;
            r = r.trim();
            if (r.startswith("file")) {
                r = r.drop_front(5).drop_back(1).trim();
                SmallVector<llvm::StringRef, 5> fs;
                r.split(fs, ",", -1, false);
                auto &v = d.d_files[fs[0].trim().str()];
                for (unsigned i = 1; i < fs.size(); ++i) {
                    v.emplace_back(fs[i].trim());
                }
                bad = false;
            } else if (r.startswith("name")) {
                r = r.drop_front(5).drop_back(1).trim();
                SmallVector<llvm::StringRef, 2> ns;
                r.split(ns, ",", -1, false);
                if (ns.size() == 2) {
                    bad = false;
                    d.d_replacements[ns[0].trim()] = ns[1].trim();
                }
            }
            if (bad) {
                a.report(m.getLocForStartOfFile(m.getMainFileID()),
                         check_name, "RX01",
                         "Bad refactor option '" + o.str() + "' - "
                         "Use file(old[,new]*) or name(old,new)");
            }
        }
        if (cfg.size()) {
            a.report(m.getLocForStartOfFile(m.getMainFileID()),
            check_name, "RX01",
            "Bad refactor option '" + cfg.str() + "' - "
            "Use file(old[,new]*) or name(old,new)");
        }
    }
}

void report::operator()(Token const& token,
                        const MacroDefinition&,
                        SourceRange,
                        MacroArgs const *)
{
    if (a.is_component(token.getLocation())) {
        std::string name = p.getSpelling(token);
        replace(SourceRange(token.getLocation(), token.getLastLoc()), name);
    }
}

bool report::TraverseDeclRefExpr(DeclRefExpr *arg)
{
    Guard guard(this, __FUNCTION__, arg);

    if (a.is_component(arg->getLocation()) &&
        replace(arg->getSourceRange(),
                arg->getDecl()->getQualifiedNameAsString())) {
        // Skip calling TraverseNestedNameSpecifierLoc(arg->getQualifierLoc());
        // Skip calling TraverseDeclarationNameInfo(arg->getNameInfo());
        // This is TraverseTemplateArgumentLocsHelper, which is private.
        unsigned tan = arg->getNumTemplateArgs();
        const TemplateArgumentLoc *tal = arg->getTemplateArgs();
        for (unsigned i = 0; i < tan; ++i) {
            if (!TraverseTemplateArgumentLoc(tal[i])) {
                return false;                                         // RETURN
            }
        }
        return true;                                                  // RETURN
    }
    return base::TraverseDeclRefExpr(arg);
}

bool report::TraverseElaboratedTypeLoc(ElaboratedTypeLoc arg)
{
    Guard guard(this, __FUNCTION__, arg);
    if (a.is_component(arg.getBeginLoc()) &&
        replace_class(arg.getSourceRange(), arg.getInnerType())) {
        return true;                                                  // RETURN
    }
    return base::TraverseElaboratedTypeLoc(arg);
}

bool report::TraverseQualifiedTypeLoc(QualifiedTypeLoc arg)
{
    Guard guard(this, __FUNCTION__, arg);
    // Base TraverseQualifiedTypeLoc does not call derived TraverseTypeLoc.
    return TraverseTypeLoc(arg.getUnqualifiedLoc());
}

bool report::TraverseTemplateSpecializationTypeLoc(
                                             TemplateSpecializationTypeLoc arg)
{
    if (a.is_component(arg.getBeginLoc())) {
        TemplateName tn = arg.getTypePtr()->getTemplateName();
        std::string s;
        llvm::raw_string_ostream o(s);
        PrintingPolicy pp(a.context()->getLangOpts());
        tn.print(o, pp);
        replace(
            SourceRange(arg.getTemplateNameLoc(), arg.getTemplateNameLoc()),
            o.str());
    }

    return base::TraverseTemplateSpecializationTypeLoc(arg);
}

bool report::TraverseTypeLoc(TypeLoc arg)
{
    Guard guard(this, __FUNCTION__, arg);
    if (a.is_component(arg.getBeginLoc()) &&
        replace_class(arg.getSourceRange(), arg.getType())) {
        return true;                                                  // RETURN
    }
    return base::TraverseTypeLoc(arg);
}

bool report::TraverseNestedNameSpecifierLoc(NestedNameSpecifierLoc arg)
{
    if (arg && a.is_component(arg.getBeginLoc())) {
        Guard guard(this, __FUNCTION__, arg);
        auto k = arg.getNestedNameSpecifier()->getKind();
        switch (k) {
          case NestedNameSpecifier::TypeSpec:
          case NestedNameSpecifier::TypeSpecWithTemplate:
            if (replace_class(
                    arg.getSourceRange(), arg.getTypeLoc().getType())) {
                return true;                                          // RETURN
            }
            break;
          case NestedNameSpecifier::Namespace:
            if (replace(arg.getSourceRange(),
                        arg.getNestedNameSpecifier()
                            ->getAsNamespace()
                            ->getName())) {
                return true;                                          // RETURN
            }
            break;
          case NestedNameSpecifier::NamespaceAlias:
            if (replace(arg.getSourceRange(),
                        arg.getNestedNameSpecifier()
                            ->getAsNamespaceAlias()
                            ->getName())) {
                return true;                                          // RETURN
            }
            break;
        }
    }
    return base::TraverseNestedNameSpecifierLoc(arg);
}

bool report::VisitCXXRecordDecl(CXXRecordDecl *arg)
{
    if (!arg->hasDefinition() &&
        arg->getLexicalDeclContext()->isNamespace() &&
        llvm::dyn_cast<NamespaceDecl>(arg->getLexicalDeclContext())
                             ->getName() == a.config()->toplevel_namespace()) {
        QualType r(arg->getTypeForDecl(), 0);
        PrintingPolicy pp(a.context()->getLangOpts());
        pp.SuppressTagKeyword = 1;
        pp.SuppressUnwrittenScope = 1;
        pp.TerseOutput = 1;
        std::string s = r.getAsString(pp);
        llvm::StringRef e = clean_for_replace(s, s);
        auto i = d.d_replacements.find(e);
        if (i != d.d_replacements.end()) {
            SmallVector<llvm::StringRef, 5> ns;
            i->second.split(ns, "::", -1, false);
            std::string rep = "class " + ns.back().str() + ";";
            for (unsigned i = 0; i < ns.size() - 1; ++i) {
                rep = "namespace " + ns[i].str() + " { " + rep + " }";
            }
            SourceRange sr = a.get_full_range(
                SourceRange(m.getSpellingLoc(arg->getBeginLoc()),
                            m.getSpellingLoc(arg->getEndLoc())));
            if (a.ReplaceText(sr, rep)) {
                auto report = a.report(sr.getBegin(), check_name, "RD01",
                         "Replacing forward declaration " + e.str() +
                         " with " + rep);
                report << sr;
            }
        }
    }

    return true;
}

llvm::StringRef report::clean_for_replace(llvm::StringRef s, std::string& buf)
{
    if (s.find("::::") != s.npos) {
        bool bec = s.endswith("::");
        SmallVector<llvm::StringRef, 5> ns;
        s.split(ns, "::", -1, false);
        buf = llvm::join(ns.begin(), ns.end(), "::");
        if (bec) {
            buf += "::";
        }
        s = buf;
    }

    if (s.startswith("::")) {
        s = s.drop_front(2);
    }

    llvm::StringRef blp(a.config()->toplevel_namespace());
    if (s.startswith(blp) && s.drop_front(blp.size()).startswith("::")) {
        s = s.drop_front(blp.size() + 2);
    }

    return s;
}

bool report::replace_class(SourceRange sr, QualType t)
{
    PrintingPolicy pp(a.context()->getLangOpts());
    pp.SuppressTagKeyword = 1;
    pp.SuppressUnwrittenScope = 1;
    pp.TerseOutput = 1;
    return replace(sr, t.getAsString(pp));
}

bool report::replace(SourceRange sr, llvm::StringRef e)
{
    std::string s;
    e = clean_for_replace(e, s);
    auto i = d.d_replacements.find(e);
    if (i != d.d_replacements.end()) {
        sr = a.get_full_range(SourceRange(
            m.getSpellingLoc(sr.getBegin()), m.getSpellingLoc(sr.getEnd())));
        llvm::StringRef src = a.get_source(sr, true);
        std::string rep = i->second.str();
        if (src.endswith("::")) {
            rep += "::";
        }
        if (a.ReplaceText(sr, rep)) {
            auto report = a.report(sr.getBegin(), check_name, "RC01",
                     "Replacing " + src.str() + " with " + rep);
            report << sr;
        }
        return true;
    }
    return false;
}

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
{
    analyser.onTranslationUnitDone += report(analyser);
    observer.onPPMacroExpands += report(analyser);
    observer.onPPFileChanged += report(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static RegisterCheck c1(check_name, &subscribe);

// ----------------------------------------------------------------------------
// Copyright (C) 2015 Bloomberg Finance L.P.
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
