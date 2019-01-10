// csabbg_functioncontract.cpp                                        -*-C++-*-

#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/DeclarationName.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/Specifiers.h>
#include <csabase_analyser.h>
#include <csabase_config.h>
#include <csabase_debug.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_util.h>
#include <ctype.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Regex.h>
#include <stddef.h>
#include <stdlib.h>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <cctype>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace csabase { class Visitor; }

using namespace clang;
using namespace csabase;

// ----------------------------------------------------------------------------

static std::string const check_name("move-contract");

// ----------------------------------------------------------------------------

namespace
{

struct data
    // Data attached to analyzer for this check.
{
    typedef std::vector<SourceRange> Ranges;
    typedef std::map<std::string, Ranges> Comments;
    Comments d_comments;  // Comment blocks per file.

    typedef std::vector<std::pair<const FunctionDecl*, SourceRange> > FunDecls;
    FunDecls d_fundecls;  // FunDecl, comment

    typedef std::multimap<unsigned, FunDecls::iterator> FMap;
    FMap d_fmap;
};

struct report : Report<data>
{
    INHERIT_REPORT_CTOR(report, Report, data);

    void operator()();
        // Invoked on end of translation unit.

    void operator()(const FunctionDecl *);
        // Invoked to process function declarations.

    void operator()(const FunctionTemplateDecl *);
        // Invoked to process function template declarations.

    SourceRange getContract(const FunctionDecl *func,
                            data::Ranges::iterator comments_begin,
                            data::Ranges::iterator comments_end);
        // Return the 'SourceRange' of the function contract of the specified
        // 'func' if it is present in the specified range of 'comments_begin'
        // up to 'comments_end', and return an invalid 'SourceRange' otherwise.

    bool isValidContract(const FunctionDecl *func, SourceRange comment);
        // Check whether the specified 'comment' is a valid (indented, etc.)
        // contract for the specified 'func'.

    const NamedDecl *moveBefore(const FunctionDecl *func);
        // Return the declartion before which to move the contract of the
        // specified 'func'.  This can be different from 'func' itself if
        // 'func' is preceded by consecutive functions, for which the contract
        // of 'func' applies.

    enum Status { e_Empty, e_Found, e_NotFound };

    bool areConsecutive(const SourceRange& r1, const SourceRange& r2) const;
        // Return true iff the specified 'r1' is immediately followed by the
        // specified 'r2', i.e., 'r2' begins in the same file in which 'r1'
        // ends, either on the same line or the line after.  Note that there
        // might be code (not comments) between the ranges, but this check is
        // good enough for the purpose of considering multi-line "//" comments
        // as single comment blocks.

    static bool isDirective(llvm::StringRef comment);
        // Return wehether the specified 'comment' is a "// = default/delete"
        // comment.

    void operator()(SourceRange range);
        // The specified 'range', representing a comment, is either appended to
        // the previous comment or added separately to the comments list.

    const NamedDecl *top(const NamedDecl *decl);
        // If the specified 'decl' is the function declaration of a function
        // template, return that template, otherwise return 'decl'.
};

const NamedDecl *report::top(const NamedDecl *decl)
{
    auto func = llvm::dyn_cast<FunctionDecl>(decl);
    if (func) {
        auto tplt = func->getDescribedFunctionTemplate();
        if (tplt) {
            decl = tplt;
        }
    }
    return decl;
}

void report::operator()()
{
    for (auto& decl : d.d_fundecls) {
        Location location(a.get_location(decl.first->getLocStart()));
        data::Ranges& c = d.d_comments[location.file()];
        decl.second = getContract(decl.first, c.begin(), c.end());
    }

    for (auto& decl : d.d_fundecls) {
        if (decl.second.isValid() &&
            isValidContract(decl.first, decl.second)) {
            // Move the contract
            auto range = SourceRange(
                a.get_line_range(decl.second.getBegin()).getBegin(),
                a.get_line_range(decl.second.getEnd())
                    .getEnd()
                    .getLocWithOffset(1));
            llvm::StringRef contract = a.get_source(range, true);
            auto before = moveBefore(decl.first);
            llvm::StringRef previous = a.get_source_line(
                a.get_line_range(before->getSourceRange().getBegin())
                    .getBegin()
                    .getLocWithOffset(-1));
            std::string shifted = previous.size() ? "\n" : "";
            if (contract.startswith("    ")) {
                contract = contract.drop_front(4);
            }
            size_t sep;
            while (contract.npos != (sep = contract.find("\n    "))) {
                shifted += contract.take_front(sep + 1).str();
                contract = contract.drop_front(sep + 5);
            }
            shifted += contract.str();
            a.report(decl.second.getBegin(),
                     check_name, "CM01",
                     "Function contract to be moved");
            if (before != top(decl.first)) {
                a.report(before, check_name, "CM01",
                         "Contract moves to different declaration",
                         false, DiagnosticIDs::Note);
            }
            a.RemoveText(range);
            a.InsertTextBefore(
                a.get_line_range(before->getSourceRange().getBegin())
                    .getBegin(),
                shifted);
        }
    }
}

void report::operator()(const FunctionDecl* func)
    // Callback function for inspecting function declarations.
{
    // Don't process compiler-defaulted methods, main, template instantiations,
    // or macro expansions
    auto rd = llvm::dyn_cast<CXXRecordDecl>(func->getDeclContext());
    if (   !func->isDefaulted()
        && !func->isMain()
        && !func->getLocation().isMacroID()
        && (   func->getTemplatedKind() == func->TK_NonTemplate
            || func->getTemplatedKind() == func->TK_FunctionTemplate)
        && a.is_component(func)
        && (!rd || !rd->getTemplateInstantiationPattern())
            ) {
        d.d_fundecls.push_back(std::make_pair(func, SourceRange()));
    }
}

void report::operator()(const FunctionTemplateDecl* func)
    // Callback function for inspecting function template declarations.
{
    (*this)(func->getTemplatedDecl());
}
 
const NamedDecl *report::moveBefore(const FunctionDecl *func)
{
    const DeclContext *parent = func->getLookupParent();
    std::string name = func->getNameAsString();

    DeclContext::decl_iterator declsb = parent->decls_begin();
    DeclContext::decl_iterator declse = parent->decls_end();
    while (declsb != declse) {
        const Decl *decl = *declsb++;
        const FunctionDecl* cfunc =
            llvm::dyn_cast<FunctionDecl>(decl);
        const FunctionTemplateDecl* ctplt =
            llvm::dyn_cast<FunctionTemplateDecl>(decl);
        if (ctplt) {
            cfunc = ctplt->getTemplatedDecl();
        }
        data::FunDecls::iterator itr = d.d_fundecls.begin();
        while (itr != d.d_fundecls.end() && itr->first != cfunc) {
            ++itr;
        }
        if (itr != d.d_fundecls.end()) {
            if (cfunc != func) {
                decl = top(cfunc);
                int from =
                    m.getPresumedLineNumber(decl->getSourceRange().getBegin());
                int to =
                    m.getPresumedLineNumber(decl->getSourceRange().getEnd());
                while (from <= to) {
                    d.d_fmap.insert(std::make_pair(from++, itr));
                }
            }
        }
    }

    const NamedDecl *orig = top(func);
    const NamedDecl *before = orig;
    unsigned bl = m.getPresumedLineNumber(before->getSourceRange().getBegin());
    for (;;) {
        auto itrs = d.d_fmap.equal_range(--bl);
        if (itrs.first == itrs.second) {
            break;
        }
        for (; itrs.first != itrs.second; itrs.first++) {
            if (!itrs.first->second->second.isValid()) {
                before = top(itrs.first->second->first);
            }
            else {
                before = orig;
            }
        }
    }

    return before;
}

SourceRange report::getContract(const FunctionDecl     *func,
                                data::Ranges::iterator  comments_begin,
                                data::Ranges::iterator  comments_end)
{
    SourceRange declarator = func->getSourceRange();
    declarator.setEnd(declarator.getEnd().getLocWithOffset(1));
    SourceRange contract;
    bool with_body = func->doesThisDeclarationHaveABody() && func->getBody();
    bool one_liner =
        with_body &&
        m.getPresumedLineNumber(declarator.getBegin()) ==
            m.getPresumedLineNumber(func->getBody()->getLocEnd());

    const CXXConstructorDecl *ctor = llvm::dyn_cast<CXXConstructorDecl>(func);

    if (ctor && with_body && ctor->getNumCtorInitializers() > 0) {
        // Constructor with body and initializers - look for a contract that
        // starts no earlier than the first initializer and has only whitespace
        // and a colon between itself and that initializer.
        SourceLocation initloc = (*ctor->init_begin())->getSourceLocation();
        if (initloc.isValid()) {
            data::Ranges::iterator it;
            for (it = comments_begin; it != comments_end; ++it) {
                if (m.isBeforeInTranslationUnit(initloc, it->getBegin())) {
                    break;
                }
                if (m.isBeforeInTranslationUnit(it->getEnd(),
                                                declarator.getBegin())) {
                    continue;
                }
                llvm::StringRef s = a.get_source(
                        SourceRange(it->getEnd(), initloc), true);
                if (s.find_first_not_of(": \r\n") == llvm::StringRef::npos) {
                    contract = *it;
                    break;
                }
            }
        }
    }

    if (with_body && !contract.isValid()) {
        // Function with body - look for a comment that starts no earlier than
        // the function declarator and has only whitespace between itself and 
        // the open brace of the function.
        SourceLocation bodyloc = func->getBody()->getLocStart();
        data::Ranges::iterator it;
        for (it = comments_begin; it != comments_end; ++it) {
            if (m.isBeforeInTranslationUnit(bodyloc, it->getBegin())) {
                break;
            }
            if (m.isBeforeInTranslationUnit(it->getEnd(),
                                            declarator.getBegin())) {
                continue;
            }
            llvm::StringRef s = a.get_source(
                SourceRange(it->getEnd(), bodyloc), true);
            if (s.find_first_not_of(" \r\n") == llvm::StringRef::npos) {
                contract = *it;
                break;
            }
        }
    }

    if (!with_body || (one_liner && !contract.isValid())) {
        // Function without body or one-liner - look for a comment following
        // the declaration separated from it by only whitespace and semicolon.
        SourceLocation endloc = declarator.getEnd();
        data::Ranges::iterator it;
        for (it = comments_begin; it != comments_end; ++it) {
            if (m.isBeforeInTranslationUnit(it->getEnd(), endloc)) {
                continue;
            }
            llvm::StringRef s = a.get_source(
                SourceRange(endloc, it->getBegin()), true);
            if (!with_body) {
                s = s.split(';').second;
            }
            if (s.find_first_not_of(" \r\n") == llvm::StringRef::npos &&
                s.count("\n") <= 1) {
                contract = *it;
            }
            break;
        }
    }
    return contract;
}

bool report::isValidContract(const FunctionDecl* func, SourceRange comment)
{
    llvm::StringRef contract = a.get_source(comment);

    // Ignore "= default" and "= delete" comments and deprecated functions.
    if (isDirective(contract)) {
        return false;                                                 // RETURN
    }

    const SourceLocation cloc = comment.getBegin();

    // Check for bad indentation.
    llvm::StringRef f = a.get_source_line(func->getLocStart());
    llvm::StringRef c = a.get_source_line(cloc);
    int fline = m.getPresumedLineNumber(func->getLocStart());
    int fcolm = int(f.find_first_not_of(' '));
    int cline = m.getPresumedLineNumber(cloc);
    int ccolm = int(c.find_first_not_of(' '));
    if (fline != cline && ccolm != fcolm + 4) {
        return false;                                                 // RETURN
    }
    return true;
}

bool report::isDirective(llvm::StringRef comment)
{
    // Look for a variety of directives in comments.
    size_t last_line_pos = comment.rfind('\n');
    if (last_line_pos != comment.npos) {
        comment = comment.substr(last_line_pos + 1);
    }
    static llvm::Regex re("^(//|/[*])" "[[:space:]]*"
                          "("
                             "=" "[[:space:]]*" "delete"  "|"
                        ".*" "=" "[[:space:]]*" "default" "|"
                             "[@!]?DEPRECATED!?"          "|"
                             "IMPLICIT"
                          ")"
                          "[;.[:space:]]*" "([*]/)?" "[[:space:]]*" "$",
                          llvm::Regex::IgnoreCase);
    return re.match(comment);
}

bool
report::areConsecutive(const SourceRange& r1, const SourceRange& r2) const
{
    return csabase::areConsecutive(m, r1, r2) &&
           !isDirective(a.get_source(r1)) &&
           !isDirective(a.get_source(r2));
}

void report::operator()(SourceRange range)
{
    Location location(a.get_location(range.getBegin()));
    if (a.is_component(location.file())) {
        data::Ranges& c = d.d_comments[location.file()];
        if (c.size() == 0 || !areConsecutive(c.back(), range)) {
            c.push_back(range);
        } else {
            c.back().setEnd(range.getEnd());
        }
    }
}


void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    analyser.onTranslationUnitDone += report(analyser);
    observer.onComment += report(analyser);
    visitor.onFunctionDecl += report(analyser);
    visitor.onFunctionTemplateDecl += report(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static RegisterCheck c3(check_name, &subscribe);

// ----------------------------------------------------------------------------
// Copyright (C) 2019 Bloomberg Finance L.P.
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
