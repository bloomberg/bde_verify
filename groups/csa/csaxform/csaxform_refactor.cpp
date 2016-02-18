// csaxform_refactor.cpp                                              -*-C++-*-

#include "csabase_analyser.h"
#include "csabase_debug.h"
#include "csabase_ppobserver.h"
#include "csabase_registercheck.h"
#include "csabase_report.h"
#include "csabase_util.h"
#include "csabase_visitor.h"

#include <llvm/ADT/Hashing.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/ADT/StringRef.h>
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

    std::unordered_map<SourceLocation /* location */,
                       std::string    /* guard    */,
                       hash           /* hash     */> d_ifs;
        // Guarded region beginnings.

    std::unordered_map<std::string              /* file      */,
                       std::vector<SourceRange> /* locations */> d_includes;
        // Locations where file is included, including those elided by external
        // include guards.  If a file is included within a guarded region, the
        // range starts on the '#ifndef' line and ends on the '#endif' line.
        // Otherwise the range starts and ends on the '#include' line.

    std::string d_most_recent_guard;
        // The most recently seen guard in an '#ifndef' condition.

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

    const std::string guard_for_file(llvm::StringRef file);
        // Return an include guard derived from the specified 'file' name,
        // formulated as "INCLUDED_{STEM}" where "STEM" is the capitalized part
        // of 'file' after path and extension components are removed.

    const std::string file_for_guard(llvm::StringRef guard);
        // return a file name derived from the specified 'guard'.  It consists
        // of a lower-case version of 'guard' with its "INCLUDE[D]_" prefix
        // removed and ".h" appended.

    bool is_guard(llvm::StringRef macro);
        // Return whether the specified 'macro' looks like an include guard
        // (starts with "INCLUDE_" or "INCLUDED_").

    void operator()();
        // Callback for end of compilation unit.

    void operator()(SourceLocation, const Token &, const MacroDirective *);
        // Callback for ifndef.

    void operator()(SourceLocation,
                    const Token &,
                    StringRef,
                    bool,
                    CharSourceRange,
                    const FileEntry *,
                    StringRef,
                    StringRef,
                    const Module *);
        // Callback for inclusion directive.

    void operator()(Token const &,
                    MacroDirective const *,
                    SourceRange,
                    MacroArgs const *);
        // Callback for macro expansion
    
    void operator()(SourceLocation, SourceLocation);
        // Callback for endif.

    void operator()(SourceRange);
        // Callback for skipped region.

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

const std::string report::guard_for_file(llvm::StringRef file)
{
    return "INCLUDED_" + llvm::sys::path::stem(file).upper();
}

const std::string report::file_for_guard(llvm::StringRef guard)
{
    if (guard.startswith("INCLUDED_")) {
        guard = guard.drop_front(9);
    }
    else if (guard.startswith("INCLUDE_")) {
        guard = guard.drop_front(8);
    }
    if (guard.endswith("_H")) {
        guard = guard.drop_back(2);
    }
    return guard.lower() + ".h";
}

bool report::is_guard(llvm::StringRef macro)
{
    return macro.startswith("INCLUDED_") ||
           macro.startswith("INCLUDE_");
}

void report::operator()()
{
    auto tu = a.context()->getTranslationUnitDecl();

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
                FileID ifid = m.getFileID(replace_range.getBegin());
                std::string rep_text = "";
                std::string sep = "";
                for (const auto& replacement : replace_iter->second) {
                    bool already_included = by_id[ifid].count(replacement);
                    if (!already_included) {
                        if (replace_range.getBegin() ==
                            replace_range.getEnd()) {
                            rep_text += sep + "#include <" + replacement + ">";
                        }
                        else {
                            rep_text += sep + "#ifndef " +
                                        guard_for_file(replacement) + "\n" +
                                        "#include <" + replacement + ">\n" +
                                        "#endif";
                        }
                        sep = "\n";
                        by_id[ifid].insert(replacement);
                    }
                }
                SourceRange extended_range(
                    a.get_line_range(replace_range.getBegin()).getBegin(),
                    a.get_line_range(replace_range.getEnd()).getEnd());
                a.report(extended_range.getBegin(), check_name, "RF01",
                         "Replacing " +
                         on_one_line(a.get_source(extended_range), true) +
                         " with " +
                         on_one_line(rep_text, true));
                a.ReplaceText(extended_range, rep_text);
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
                auto &v = d.d_files[fs[0].trim()];
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

void report::operator()(SourceLocation        loc,
                        const Token&          token,
                        const MacroDirective *)
{
    std::string may_be_guard = p.getSpelling(token);
    if (is_guard(may_be_guard)) {
        d.d_ifs.emplace(loc, may_be_guard);
        d.d_most_recent_guard = may_be_guard;
    }
}

void report::operator()(SourceLocation loc,
                        const Token &token,
                        StringRef file,
                        bool,
                        CharSourceRange,
                        const FileEntry *,
                        StringRef,
                        StringRef,
                        const Module *)
{
    if (d.d_most_recent_guard != guard_for_file(file)) {
        d.d_includes[file].emplace_back(SourceRange(loc, loc));
    }

}

void report::operator()(SourceLocation loc, SourceLocation ifloc)
{
    d.d_most_recent_guard = std::string();

    if (d.d_ifs.count(ifloc)) {
        llvm::StringRef src = a.get_source(SourceRange(ifloc, loc));
        std::string guard = d.d_ifs[ifloc];
        std::string file = file_for_guard(guard);
        std::string match = "^ifndef *" + guard + ".*$\r*\n"
                            "^ *# *include *[<\"]" + file + "[\">].*$\r*\n"
                            "(^ *# *define *" + guard + ".*$\r*\n)?"
                            "^ *#endif";
        llvm::Regex re(match, llvm::Regex::Newline);
        if (re.match(src)) {
            d.d_includes[file].emplace_back(SourceRange(ifloc, loc));
        }
    }
}

void report::operator()(Token const& token,
                        MacroDirective const *,
                        SourceRange,
                        MacroArgs const *)
{
    std::string name = p.getSpelling(token);
    replace(SourceRange(token.getLocation(), token.getLastLoc()), name);
}

bool report::TraverseDeclRefExpr(DeclRefExpr *arg)
{
    Guard guard(this, __FUNCTION__, arg);

    if (replace(arg->getSourceRange(),
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
    if (replace_class(arg.getSourceRange(), arg.getInnerType())) {
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
    TemplateName tn = arg.getTypePtr()->getTemplateName();
    std::string s;
    llvm::raw_string_ostream o(s);
    PrintingPolicy pp(a.context()->getLangOpts());
    tn.print(o, pp);
    replace(SourceRange(arg.getTemplateNameLoc(), arg.getTemplateNameLoc()),
            o.str());

    return base::TraverseTemplateSpecializationTypeLoc(arg);
}

bool report::TraverseTypeLoc(TypeLoc arg)
{
    Guard guard(this, __FUNCTION__, arg);
    if (replace_class(arg.getSourceRange(), arg.getType())) {
        return true;                                                  // RETURN
    }
    return base::TraverseTypeLoc(arg);
}

bool report::TraverseNestedNameSpecifierLoc(NestedNameSpecifierLoc arg)
{
    if (arg) {
        Guard guard(this, __FUNCTION__, arg);
        auto k = arg.getNestedNameSpecifier()->getKind();
        if (k == NestedNameSpecifier::TypeSpec ||
            k == NestedNameSpecifier::TypeSpecWithTemplate) {
            if (replace_class(
                    arg.getSourceRange(), arg.getTypeLoc().getType())) {
                return true;                                          // RETURN
            }
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
                SourceRange(m.getSpellingLoc(arg->getLocStart()),
                            m.getSpellingLoc(arg->getLocEnd())));
            a.report(sr.getBegin(), check_name, "RD01",
                     "Replacing forward declaration " + e.str() +
                     " with " + rep)
                << sr;
            a.ReplaceText(sr, rep);
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
        std::string rep = i->second;
        if (src.endswith("::")) {
            rep += "::";
        }
        a.report(sr.getBegin(), check_name, "RC01",
                 "Replacing " + src.str() + " with " + rep)
            << sr;
        a.ReplaceText(sr, rep);
        return true;
    }
    return false;
}

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
{
    analyser.onTranslationUnitDone += report(analyser);
    observer.onPPIfndef += report(analyser);
    observer.onPPInclusionDirective += report(analyser);
    observer.onPPEndif += report(analyser);
    observer.onPPMacroExpands += report(analyser);
    observer.onPPFileChanged += report(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static RegisterCheck c1(check_name, &subscribe);

// ----------------------------------------------------------------------------
// Copyright (C) 2015 Bloomberg Finance L.P.
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
