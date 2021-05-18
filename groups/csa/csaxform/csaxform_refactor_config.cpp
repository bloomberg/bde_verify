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
#include <llvm/Support/Path.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Lex/Preprocessor.h>

#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace clang;
using namespace clang::ast_matchers;
using namespace csabase;

// ----------------------------------------------------------------------------

static std::string const check_name("refactor-config");

// ----------------------------------------------------------------------------

namespace
{

typedef std::set<std::string> t_ss;
typedef std::map<std::string, std::string> t_matches;

int s_index = 0;
std::map<std::string, t_ss> s_names[2];
std::map<std::string, t_matches> s_matched;
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

std::fstream &read_string(std::fstream &f, std::string &s)
{
    size_t length;
    if (f >> length) {
        s.resize(length);
        f.ignore(1).read(&s[0], length).ignore(1);
    }
    return f;
}

std::fstream &read_string_set(std::fstream &f, t_ss &ss)
{
    size_t length;
    if (f >> length) {
        while (length-- > 0) {
            std::string s;
            read_string(f, s);
            ss.insert(s);
        }
    }
    return f;
}

std::fstream &read_strings_map(std::fstream &f, t_matches &ss)
{
    size_t length;
    if (f >> length) {
        while (length-- > 0) {
            std::string s1, s2;
            read_string(f, s1);
            read_string(f, s2);
            ss[s1] = s2;
        }
    }
    return f;
}

std::fstream& write_string(std::fstream&      f,
                           const std::string& s,
                           char               c = '\n')
{
    f << s.length() << ' ' << s << c;
    return f;
}

std::fstream& write_string_set(std::fstream& f, const t_ss& ss)
{
    f << ss.size() << "\n";
    for (auto &s : ss) {
        write_string(f, s);
    }
    return f;
}

std::fstream& write_strings_map(std::fstream& f, const t_matches& ss)
{
    f << ss.size() << "\n";
    for (auto &s : ss) {
        write_string(f, s.first);
        write_string(f, s.second);
    }
    return f;
}

void report::operator()()
{
    std::string inter = a.config()->value("refactorintermediatefile");
    if (inter == "" || inter == "-") {
        inter = "refactorintermediatefile.cfg";
    }
    std::fstream interf(inter, std::ios_base::in | std::ios_base::binary);
    if (interf) {
        read_string(interf, s_files[0]);
        read_string(interf, s_upper_prefix[0]);
        std::string n1, n2;
        while (read_string(interf, n1)) {
            read_string_set(interf, s_names[0][n1]);
            read_strings_map(interf, s_matched[n1]);
        }
        interf.close();
        remove(inter.c_str());
        s_index = 1;
    }

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
        if (auto md = p.getMacroDefinition(ii).getLocalDirective()) {
            if (a.is_component(md->getLocation())) {
                std::string s = ii->getName().str();
                if (!llvm::StringRef(s).startswith("INCLUDE") &&
                    macros.emplace(s).second) {
                    // Replace macros with their replacement text if simple.
                    auto mi = md->getInfo();
                    if (mi->isObjectLike() && mi->getNumTokens()) {
                        std::string r = a.get_source(SourceRange(
                            mi->getReplacementToken(0).getLocation(),
                            mi->getReplacementToken(mi->getNumTokens() - 1)
                                .getEndLoc())).str();
                        if (r.npos == r.find('\n')) {
                            s_matched[Tags[Macro]][s] = r;
                        }
                    }
                    a.report(md->getLocation(), check_name, "DD01",
                             "Found " + Tags[Macro] + " " + s);
                }
            }
        }
    }

    OnMatch<> m1([&](const BoundNodes &nodes) {
        if (!nodes.getNodeAs<NamespaceDecl>("n")->isAnonymousNamespace()) {
            auto r = nodes.getNodeAs<CXXRecordDecl>(Tags[Class]);
            if (a.is_component(r->getLocation()) &&
                clang::AS_private != r->getAccess() &&
                !r->getTemplateSpecializationKind() &&
                !r->getTypedefNameForAnonDecl()) {
                std::string s = r->getQualifiedNameAsString();
                s = clean_name(s, s).str();
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
                forEach(cxxRecordDecl(isDefinition(),
                                      unless(isTemplateInstantiation())
                        ).bind(Tags[Class]))
            ).bind("n"))),
        &m1);

    OnMatch<> m2([&](const BoundNodes &nodes) {
        if (!nodes.getNodeAs<NamespaceDecl>("n")->isAnonymousNamespace()) {
            auto r = nodes.getNodeAs<CXXRecordDecl>(Tags[Class]);
            auto e = nodes.getNodeAs<EnumDecl>(Tags[Enum]);
            if (a.is_component(e->getLocation()) &&
                !r->getTemplateSpecializationKind() &&
                !r->getTypedefNameForAnonDecl() &&
                clang::AS_private != r->getAccess() &&
                clang::AS_private != e->getAccess() &&
                e->hasNameForLinkage()) {
                std::string s = e->getQualifiedNameAsString();
                s = clean_name(s, s).str();
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
                forEach(cxxRecordDecl(isDefinition(),
                                      unless(isTemplateInstantiation()),
                                      forEach(enumDecl(
                                              ).bind(Tags[Enum]))
                        ).bind(Tags[Class]))
                ).bind("n"))),
        &m2);

    OnMatch<> m3([&](const BoundNodes &nodes) {
        if (!nodes.getNodeAs<NamespaceDecl>("n")->isAnonymousNamespace()) {
            auto r = nodes.getNodeAs<CXXRecordDecl>(Tags[Class]);
            auto l = nodes.getNodeAs<EnumConstantDecl>(Tags[Literal]);
            if (a.is_component(l->getLocation()) &&
                clang::AS_private != r->getAccess() &&
                clang::AS_private != l->getAccess() &&
                !r->getTemplateSpecializationKind() &&
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
                s = clean_name(s, s).str();
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
                forEach(cxxRecordDecl(isDefinition(),
                                      unless(isTemplateInstantiation()),
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
            if (a.is_component(t->getLocation()) &&
                t->isThisDeclarationADefinition()) {
                std::string s = t->getQualifiedNameAsString();
                s = clean_name(s, s).str();
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
            if (a.is_component(t->getLocation())) {
                std::string s = t->getQualifiedNameAsString();
                s = clean_name(s, s).str();
                if (d.d_ns[Tags[Typedef]].emplace(s).second) {
                    std::string y = t->getUnderlyingType().getAsString();
                    y = clean_name(y, y).str();
                    s_matched[Tags[Typedef]][s] = y;
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
                std::string bm =
                    s_matched[Tags[t]].count(i.str())
                        ? s_matched[Tags[t]][i.str()]
                        : best_match(i, s_names[1][Tags[t]]).str();
                if (bm.size() == 0) {
                    if (t == Typedef) {
                        // Likely an existing forward declaration.
                        // continue;
                    }
                    bm = "@@@/* Need replacement for " + i.str() + " */";
                }
                else if (t == Literal) {
                    bm = prefer_e(bm, s_names[1][Tags[t]]).str();
                }
                f << " \\\n                name(" << i << "," << bm << ")";
            }
        }
        f << "\n";
    }
    else {
        interf.open(inter, std::ios_base::out | std::ios_base::binary);
        write_string(interf, s_files[0]);
        write_string(interf, s_upper_prefix[0]);
        for (auto &p : s_names[0]) {
            write_string(interf, p.first);
            write_string_set(interf, p.second);
            write_strings_map(interf, s_matched[p.first]);
        }
        interf.close();
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
