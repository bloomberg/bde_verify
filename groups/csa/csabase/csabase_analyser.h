// csabase_analyser.h                                                 -*-C++-*-

#ifndef INCLUDED_CSABASE_ANALYSER
#define INCLUDED_CSABASE_ANALYSER

#include <clang/AST/ASTContext.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Tooling/Refactoring.h>
#include <csabase_analyse.h>
#include <csabase_attachments.h>
#include <csabase_config.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_visitor.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Casting.h>
#include <map>
#include <memory>
#include <string>
#include <utils/event.hpp>
#include <vector>

namespace clang { class CompilerInstance; }
namespace clang { class Decl; }
namespace clang { class Expr; }
namespace clang { class NamedDecl; }
namespace clang { class Rewriter; }
namespace clang { class Sema; }
namespace clang { class SourceManager; }
namespace clang { class Stmt; }
namespace clang { class TypeDecl; }

// -----------------------------------------------------------------------------

namespace csabase
{
class Analyser : public Attachments
{
  public:
    Analyser(clang::CompilerInstance& compiler, const PluginAction& plugin);

    Config const* config() const;
    std::string const& tool_name() const;
    std::string const& diagnose() const;

    clang::ASTContext                   *context();
    clang::ASTContext const             *context() const;
    void                                 context(clang::ASTContext*);
    clang::CompilerInstance&             compiler();
    clang::Sema&                         sema();
    clang::Rewriter&                     rewriter();
    csabase::PPObserver&                 pp_observer();
    clang::tooling::Replacements const&  replacements() const;

    std::string const& toplevel() const;
    std::string const& directory() const;
    std::string const& prefix() const;
    std::string const& group() const;
    std::string const& package() const;
    std::string const& component() const;
    std::string const& rewrite_dir() const;
    std::string const& rewrite_file() const;
    std::string const& diff_file() const;
    void               toplevel(std::string const&);
    bool               is_header(std::string const&) const;
    bool               is_component_header(std::string const&) const;
    bool               is_component_header(clang::SourceLocation) const;
    bool               is_source(std::string const&) const;
    bool               is_component_source(std::string const&) const;
    bool               is_component_source(clang::SourceLocation) const;
    bool               is_component(clang::SourceLocation) const;
    bool               is_component(std::string const&) const;
    template <typename T> bool is_component(T const*);
    template <typename T> bool is_component_header(T const*);
    template <typename T> bool is_component_source(T const*);
    bool               is_system_header(llvm::StringRef);
    bool               is_system_header(clang::SourceLocation);
    template <typename T> bool is_system_header(T const*);
    bool               is_test_driver() const;
    bool               is_test_driver(const std::string &) const;
    template <typename T> bool is_test_driver(T const*);
    bool               is_main() const;
    bool               is_standard_namespace(std::string const&) const;
    bool               is_global_name(clang::NamedDecl const*);
    bool               is_global_package() const;
    bool               is_global_package(std::string const&) const;
    bool               is_ADL_candidate(clang::Decl const*);
    bool               is_generated(clang::SourceLocation) const;
    bool               is_toplevel(std::string const&) const;

    diagnostic_builder report(clang::SourceLocation       where,
                              std::string const&          check,
                              std::string const&          tag,
                              std::string const&          message,
                              bool                        always = false,
                              clang::DiagnosticIDs::Level level =
                                                clang::DiagnosticIDs::Warning);

    template <typename T>
    diagnostic_builder report(T where,
                              std::string const&          check,
                              std::string const&          tag,
                              std::string const&          message,
                              bool                        always = false,
                              clang::DiagnosticIDs::Level level =
                                                clang::DiagnosticIDs::Warning);

    clang::SourceManager& manager() const;
    llvm::StringRef         get_source(clang::SourceRange, bool exact = false);
    clang::SourceRange      get_full_range(clang::SourceRange);
    clang::SourceRange      get_line_range(clang::SourceLocation);
    clang::SourceRange      get_trim_line_range(clang::SourceLocation);
    llvm::StringRef         get_source_line(clang::SourceLocation);
    Location get_location(clang::SourceLocation) const;
    Location get_location(clang::Decl const*) const;
    Location get_location(clang::Expr const*) const;
    Location get_location(clang::Stmt const*) const;

    template <typename InIt> void process_decls(InIt, InIt);
    void process_decl(clang::Decl const*);
    void process_translation_unit_done();
    utils::event<void()> onTranslationUnitDone;

    clang::NamedDecl* lookup_name(std::string const& name);
    clang::TypeDecl*  lookup_type(std::string const& name);
    template <typename T> T* lookup_name_as(std::string const& name);

    bool hasContext() const { return context_; }

    template <typename Parent, typename Node>
    const Parent *get_parent(const Node *node);
        // Return a pointer to the object of the specified 'Parent' type which
        // is the nearest ancestor of the specified 'node' of 'Node' type, and
        // 0 if there is no such object.

    std::string get_rewrite_file(std::string file);
        // Return the name of the file to use for rewriting the specified
        // 'file'.

    int InsertTextBefore(clang::SourceLocation l, llvm::StringRef s);
    int RemoveText(clang::SourceRange r);
    int RemoveText(clang::SourceLocation l, unsigned n);
    int ReplaceText(clang::SourceLocation l, unsigned n, llvm::StringRef s);
    int ReplaceText(clang::SourceRange r, llvm::StringRef s);
    int ReplaceText(
         llvm::StringRef file, unsigned offset, unsigned n, llvm::StringRef s);
        // Rewriting actions.

private:
    Analyser(Analyser const&);
    void operator= (Analyser const&);
        
    std::auto_ptr<Config>                 d_config;
    std::string                           tool_name_;
    std::string                           diagnose_;
    clang::CompilerInstance&              compiler_;
    clang::SourceManager const&           d_source_manager;
    std::auto_ptr<Visitor>                visitor_;
    clang::ASTContext*                    context_;
    clang::Rewriter*                      rewriter_;
    std::string                           toplevel_;
    std::string                           directory_;
    std::string                           prefix_;
    std::string                           group_;
    std::string                           package_;
    std::string                           component_;
    std::string                           rewrite_dir_;
    std::string                           rewrite_file_;
    std::string                           diff_file_;
    typedef std::map<std::string, bool>   IsComponent;
    mutable IsComponent                   is_component_;
    typedef std::map<std::string, bool>   IsComponentHeader;
    mutable IsComponentHeader             is_component_header_;
    typedef std::map<std::string, bool>   IsGlobalPackage;
    mutable IsGlobalPackage               is_global_package_;
    typedef std::map<std::string, bool>   IsStandardNamespace;
    mutable IsStandardNamespace           is_standard_namespace_;
    clang::tooling::Replacements          replacements_;
    typedef std::map<std::string, bool>   IsSystemHeader;
    mutable IsSystemHeader                is_system_header_;
    typedef std::map<std::string, bool>   IsTopLevel;
    mutable IsTopLevel                    is_top_level_;
};

// -----------------------------------------------------------------------------

template <typename InIt>
inline
void Analyser::process_decls(InIt it, InIt end)
{
    while (it != end) {
        process_decl(*it++);
    }
}

template <typename T>
inline
diagnostic_builder Analyser::report(
    T where,
    std::string const&          check,
    std::string const&          tag,
    std::string const&          message,
    bool                        always,
    clang::DiagnosticIDs::Level level)
{
    return report(
        get_location(where).location(), check, tag, message, always, level);
}

// -----------------------------------------------------------------------------

template <typename T>
inline
bool Analyser::is_component(T const* value)
{
    return is_component(get_location(value).file());
}

template <typename T>
inline
bool Analyser::is_component_header(T const* value)
{
    return is_component_header(get_location(value).file());
}

template <typename T>
inline
bool Analyser::is_component_source(T const* value)
{
    return is_component_source(get_location(value).file());
}

template <typename T>
inline
bool Analyser::is_system_header(T const* value)
{
    return is_system_header(get_location(value).file());
}

template <typename T>
inline
bool Analyser::is_test_driver(T const* value)
{
    return is_test_driver(get_location(value).file());
}

template <typename T>
inline
T* Analyser::lookup_name_as(const std::string& name)
{
    clang::NamedDecl* nd = lookup_name(name);
    return nd ? llvm::dyn_cast<T>(nd) : 0;
}

template <typename Parent, typename Node>
inline
const Parent* Analyser::get_parent(const Node* node)
{
    for (auto pv = context()->getParents(*node);
         pv.size() >= 1;
         pv = context()->getParents(pv[0])) {
        if (const Parent* p = pv[0].template get<Parent>()) {
            return p;                                                 // RETURN
        }
    }
    return 0;
}
}

#endif

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
