// csaxform_dump_defs.cpp                                             -*-C++-*-

#include "csabase_analyser.h"
#include "csabase_debug.h"
#include "csabase_ppobserver.h"
#include "csabase_registercheck.h"
#include "csabase_report.h"
#include "csabase_util.h"
#include "csabase_visitor.h"

#include <llvm/ADT/StringExtras.h>
#include <llvm/ADT/StringRef.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Lex/Preprocessor.h>

#include <string>
#include <map>
#include <set>

using namespace clang;
using namespace clang::ast_matchers;
using namespace csabase;

// ----------------------------------------------------------------------------

static std::string const check_name("refactor-config");

// ----------------------------------------------------------------------------

namespace
{

typedef std::set<std::string> t_ss;

int s_index = 0;
std::map<std::string, t_ss> s_names[2];
std::string s_files[2];
std::string s_upper_prefix[2];

struct data
{
    std::map<std::string, t_ss> d_ns; // reported names
};

// Callback object invoked upon completion.
struct report : Report<data>
{
    INHERIT_REPORT_CTOR(report, Report, data);

    void operator()();
        // Callback for end of compilation unit.

    llvm::StringRef clean_name(llvm::StringRef name, std::string& buf);
        // Clean up the specified 'name' for use in refactor configuration
        // using the specified 'buf' as a work area if needed, and return the
        // cleaned string.

    llvm::StringRef common_suffix(llvm::StringRef a, llvm::StringRef b);
        // Return the longest common suffix of the specified 'a' and 'b'.

    llvm::StringRef best_match(llvm::StringRef s, const t_ss& sequence);
        // Return the first string in the specified 'sequence' that has the
        // longest common suffix with the specified 's'.  If the sequence
        // contains an exact match for 's', that value is ignored.

    llvm::StringRef prefer_e(llvm::StringRef s, const t_ss& sequence);
        // Return the first string in the specified 'sequence' that has a
        // preferred 'e_...' spelling variant of the specified 's' if such
        // exists, and 's' otherwise.
};

llvm::StringRef report::clean_name(llvm::StringRef name, std::string& buf)
{
    llvm::StringRef blp(a.config()->toplevel_namespace());
    if (name.startswith(blp) && name.drop_front(blp.size()).startswith("::")) {
        name = name.drop_front(blp.size() + 2);
    }
    if (name.endswith("::")) {
        name = name.drop_back(2);
    }
    if (name.find("::::") != name.npos) {
        SmallVector<llvm::StringRef, 5> ns;
        name.split(ns, "::", -1, false);
        buf = llvm::join(ns.begin(), ns.end(), "::");
        name = buf;
    }
    return name;
}

llvm::StringRef report::common_suffix(llvm::StringRef a, llvm::StringRef b)
{
    llvm::StringRef cs;
    size_t na = a.size();
    size_t nb = b.size();
    bool saw_sep = false;

    for (size_t i = 1; i <= na && i <= nb; ++i) {
        if (!isalnum(a[na - i]) || !isalnum(b[nb - i])) {
            saw_sep = true;
        }
        if (a[na - i] != b[nb - i]) {
            break;
        }
        cs = a.drop_front(na - i);
    }
    return saw_sep ? cs : llvm::StringRef();
}

llvm::StringRef report::best_match(llvm::StringRef s, const t_ss& sequence)
{
    llvm::StringRef lcs;
    llvm::StringRef bm;

    for (llvm::StringRef m : sequence) {
        if (!s.equals(m)) {
            llvm::StringRef cs = common_suffix(s, m);
            if (cs.size() > lcs.size()) {
                lcs = cs;
                bm = m;
            }
        }
    }
    return bm;
}

llvm::StringRef report::prefer_e(llvm::StringRef s, const t_ss& sequence)
{
    size_t last_colons = s.rfind("::");
    if (last_colons != s.npos) {
        llvm::StringRef lit = s.drop_front(last_colons + 2);
        for (int i = 0; i <= 1; ++i) {
            if (lit.startswith(s_upper_prefix[i])) {
                lit = lit.drop_front(s_upper_prefix[i].size());
                if (lit.startswith("_")) {
                    lit = lit.drop_front(1);
                }
                break;
            }
        }
        std::string k = s.slice(0, last_colons + 2).str() + "k_" + lit.str();
        std::string e = s.slice(0, last_colons + 2).str() + "e_" + lit.str();
        auto ik = sequence.find(k);
        auto ie = sequence.find(e);
        if (ik != sequence.end()) {
            s = *ik;
        }
        if (ie != sequence.end()) {
            s = *ie;
        }
    }
    return s;
}

void report::operator()()
{
    auto tu = a.context()->getTranslationUnitDecl();

    s_files[s_index] = llvm::sys::path::filename(
        Location(m, m.getLocForStartOfFile(m.getMainFileID())).file()).str();
    llvm::StringRef f(s_files[s_index]);
    size_t under = f.find_first_of("_.");
    if (under == f.npos) {
        under = f.size();
    }
    s_upper_prefix[s_index] = f.slice(0, under).upper();

    MatchFinder mf;

    static const std::string Tags[] = {
          "class", "enum", "literal", "template", "typedef", "macro"
    };
    enum { Class,   Enum,   Literal,   Template,   Typedef,   Macro, NTags };

    auto &macros = d.d_ns[Tags[Macro]];
    for (auto i = p.macro_begin(); i != p.macro_end(); ++i) {
        IdentifierInfo *ii = const_cast<IdentifierInfo *>(i->first);
        if (MacroDirective *md = p.getMacroDirective(ii)) {
            if (m.isWrittenInMainFile(md->getLocation())) {
                std::string s = ii->getName().str();
                if (!llvm::StringRef(s).startswith("INCLUDE") &&
                    macros.emplace(s).second) {
                    a.report(md->getLocation(), check_name, "DD01",
                             "Found " + Tags[Macro] + " " + s);
                }
            }
        }
    }

    OnMatch<> m1([&](const BoundNodes &nodes) {
        if (!nodes.getNodeAs<NamespaceDecl>("n")->isAnonymousNamespace()) {
            auto r = nodes.getNodeAs<CXXRecordDecl>(Tags[Class]);
            if (m.isWrittenInMainFile(r->getLocation()) &&
                !r->getTypedefNameForAnonDecl()) {
                std::string s = r->getQualifiedNameAsString();
                s = clean_name(s, s);
                if (d.d_ns[Tags[Class]].emplace(s).second) {
                    a.report(r, check_name, "DD01",
                             "Found " + Tags[Class] + " " + s);
                }
            }
        }
    });
    mf.addDynamicMatcher(
        decl(forEachDescendant(
            namespaceDecl(
                forEach(recordDecl(isDefinition()
                        ).bind(Tags[Class]))
            ).bind("n"))),
        &m1);

    OnMatch<> m2([&](const BoundNodes &nodes) {
        if (!nodes.getNodeAs<NamespaceDecl>("n")->isAnonymousNamespace()) {
            auto r = nodes.getNodeAs<CXXRecordDecl>(Tags[Class]);
            auto e = nodes.getNodeAs<EnumDecl>(Tags[Enum]);
            if (m.isWrittenInMainFile(e->getLocation()) &&
                !r->getTypedefNameForAnonDecl() &&
                e->hasNameForLinkage()) {
                std::string s = e->getQualifiedNameAsString();
                s = clean_name(s, s);
                if (d.d_ns[Tags[Enum]].emplace(s).second) {
                    a.report(e, check_name, "DD01",
                             "Found " + Tags[Enum] + " " + s);
                }
            }
        }
    });
    mf.addDynamicMatcher(
        decl(forEachDescendant(
            namespaceDecl(
                forEach(recordDecl(isDefinition(),
                    forEach(enumDecl(
                            ).bind(Tags[Enum]))
                        ).bind(Tags[Class]))
            ).bind("n"))),
        &m2);

    OnMatch<> m3([&](const BoundNodes &nodes) {
        if (!nodes.getNodeAs<NamespaceDecl>("n")->isAnonymousNamespace()) {
            auto r = nodes.getNodeAs<CXXRecordDecl>(Tags[Class]);
            auto l = nodes.getNodeAs<EnumConstantDecl>(Tags[Literal]);
            if (m.isWrittenInMainFile(l->getLocation()) &&
                !r->getTypedefNameForAnonDecl()) {
                std::string s;
                if (s_index == 0) {
                    // "From" side gets fully qualified name.
                    s = l->getQualifiedNameAsString();
                }
                else {
                    // "To" side gets "classname::literal".
                    s = r->getQualifiedNameAsString() + "::" +
                        l->getName().str();
                }
                s = clean_name(s, s);
                if (d.d_ns[Tags[Literal]].emplace(s).second) {
                    a.report(l, check_name, "DD01",
                             "Found " + Tags[Literal] + " " + s);
                }
            }
        }
    });
    mf.addDynamicMatcher(
        decl(forEachDescendant(
            namespaceDecl(
                forEach(recordDecl(isDefinition(),
                    forEach(enumDecl(
                        forEach(enumConstantDecl(
                                ).bind(Tags[Literal]))
                            ).bind(Tags[Enum]))
                        ).bind(Tags[Class]))
            ).bind("n"))),
        &m3);

    OnMatch<> m4([&](const BoundNodes &nodes) {
        if (!nodes.getNodeAs<NamespaceDecl>("n")->isAnonymousNamespace()) {
            auto t = nodes.getNodeAs<ClassTemplateDecl>(Tags[Template]);
            if (m.isWrittenInMainFile(t->getLocation()) &&
                t->isThisDeclarationADefinition()) {
                std::string s = t->getQualifiedNameAsString();
                s = clean_name(s, s);
                if (d.d_ns[Tags[Template]].emplace(s).second) {
                    a.report(t, check_name, "DD01",
                             "Found " + Tags[Template] + " " + s);
                }
            }
        }
    });
    mf.addDynamicMatcher(
        decl(forEachDescendant(
            namespaceDecl(
                forEach(classTemplateDecl(
                        ).bind(Tags[Template]))
            ).bind("n"))),
        &m4);

    OnMatch<> m5([&](const BoundNodes &nodes) {
        if (!nodes.getNodeAs<NamespaceDecl>("n")->isAnonymousNamespace()) {
            auto t = nodes.getNodeAs<TypedefNameDecl>(Tags[Typedef]);
            if (m.isWrittenInMainFile(t->getLocation())) {
                std::string s = t->getQualifiedNameAsString();
                s = clean_name(s, s);
                if (d.d_ns[Tags[Typedef]].emplace(s).second) {
                    a.report(t, check_name, "DD01",
                             "Found " + Tags[Typedef] + " " + s);
                }
            }
        }
    });
    mf.addDynamicMatcher(
        decl(forEachDescendant(
            namespaceDecl(
                forEach(typedefDecl(
                        ).bind(Tags[Typedef]))
            ).bind("n"))),
        &m5);

    mf.match(*tu, *a.context());

    s_names[s_index] = d.d_ns;
    s_index = 1 - s_index;

    if (s_index == 0) {
        llvm::StringRef file = a.config()->value("refactorfile");
        if (file == "" || file == "-") {
            file = "refactor.cfg";
        }
        std::error_code ec;
        llvm::raw_fd_ostream f(file, ec, llvm::sys::fs::F_Append);
        if (ec) {
            ERRS() << "File error " << file << " " << ec.message() << "\n";
            return;
        }

        f << "append refactor file(" << s_files[0] << "," << s_files[1] << ")";
        for (int t = 0; t < NTags; ++t) {
            for (llvm::StringRef i : s_names[0][Tags[t]]) {
                std::string bm = best_match(i, s_names[1][Tags[t]]);
                if (bm.size() == 0) {
                    if (t == Typedef) {
                        // Likely an existing forward declaration.
                        continue;
                    }
                    bm = "@@@/* Need replacement for " + i.str() + " */";
                }
                else if (t == Literal) {
                    bm = prefer_e(bm, s_names[1][Tags[t]]);
                }
                f << " \\\n                name(" << i << "," << bm << ")";
            }
        }
        f << "\n";
    }
}

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
{
    analyser.onTranslationUnitDone += report(analyser);
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
