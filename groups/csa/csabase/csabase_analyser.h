// csabase_analyser.h                                                 -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(INCLUDED_CSABASE_ANALYSER)
#define INCLUDED_CSABASE_ANALYSER 1
#ident "$Id$"

#include <csabase_location.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_attachments.h>
#include <cool/function.hpp>
#include <cool/event.hpp>
#include <clang/AST/ASTContext.h>
#include <clang/AST/AST.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Expr.h>
#include <clang/AST/Stmt.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <string>
#include <map>

// -----------------------------------------------------------------------------

namespace clang
{
    class CompilerInstance;
    class Sema;
    class SourceManager;
}

namespace cool
{
    namespace csabase
    {
        class Analyser;
        class Config;
        class Location;
        class PPObserver;
        class Visitor;
    }
}

// -----------------------------------------------------------------------------

class cool::csabase::Analyser:
    public cool::csabase::Attachments
{
public:
    Analyser(clang::CompilerInstance& compiler,
             bool debug, std::vector<std::string> const& config, std::string const& name);
    ~Analyser();

    Config const* config() const;
    std::string const& tool_name() const;

    clang::ASTContext*       context();
    clang::ASTContext const* context() const;
    void                     context(clang::ASTContext*);
    clang::CompilerInstance& compiler();
    clang::Sema&             sema();

    std::string const& toplevel() const;
    std::string const& directory() const;
    std::string const& prefix() const;
    std::string const& group() const;
    std::string const& package() const;
    std::string const& component() const;
    void               toplevel(std::string const&);
    bool               is_component_header(std::string const&) const;
    bool               is_component_header(clang::SourceLocation) const;
    bool               is_component_source(std::string const&) const;
    bool               is_component_source(clang::SourceLocation) const;
    bool               is_component(clang::SourceLocation) const;
    bool               is_component(std::string const&) const;
    template <typename T> bool is_component(T const*);
    template <typename T> bool is_component_header(T const*);
    template <typename T> bool is_component_source(T const*);
    bool               is_test_driver() const;
    bool               is_main() const;
    bool               is_standard_namespace(std::string const&) const;
    bool               is_global_package() const;
    bool               is_global_package(std::string const&) const;
    bool               is_ADL_candidate(clang::Decl const*);

    cool::diagnostic_builder report(clang::SourceLocation where,
                                    std::string const& check,
                                    std::string const& tag,
                                    std::string const& message,
                                    bool always = false,
                                    clang::DiagnosticsEngine::Level level =
                                        clang::DiagnosticsEngine::Warning);

    template <typename T>
    cool::diagnostic_builder report(T where,
                                    std::string const& check,
                                    std::string const& tag,
                                    std::string const& message,
                                    bool always = false,
                                    clang::DiagnosticsEngine::Level level =
                                        clang::DiagnosticsEngine::Warning);

    clang::SourceManager& manager();
    llvm::StringRef         get_source(clang::SourceRange, bool exact = false);
    cool::csabase::Location get_location(clang::SourceLocation) const;
    cool::csabase::Location get_location(clang::Decl const*) const;
    cool::csabase::Location get_location(clang::Expr const*) const;
    cool::csabase::Location get_location(clang::Stmt const*) const;

    template <typename InIt> void process_decls(InIt, InIt);
    void process_decl(clang::Decl const*);
    void process_translation_unit_done();
    cool::event<void()> onTranslationUnitDone;

    clang::NamedDecl* lookup_name(std::string const& name);
    clang::TypeDecl*  lookup_type(std::string const& name);
    template <typename T> T* lookup_name_as(std::string const& name);

    bool hasContext() const { return context_; }

    template <typename Parent, typename Node>
    const Parent *get_parent(const Node *node);
        // Return a pointer to the object of the specified 'Parent' type which
        // is the nearest ancestor of the specified 'node' of 'Node' type, and
        // 0 if there is no such object.

private:
    Analyser(cool::csabase::Analyser const&);
    void operator= (cool::csabase::Analyser const&);
        
    std::auto_ptr<cool::csabase::Config>  d_config;
    std::string                           tool_name_;
    clang::CompilerInstance&              compiler_;
    clang::SourceManager const&           d_source_manager;
    std::auto_ptr<cool::csabase::Visitor> visitor_;
    cool::csabase::PPObserver*            pp_observer_;
    clang::ASTContext*                    context_;
    std::string                           toplevel_;
    std::string                           directory_;
    std::string                           prefix_;
    std::string                           group_;
    std::string                           package_;
    std::string                           component_;
    typedef std::map<std::string, bool>   IsComponentHeader;
    mutable IsComponentHeader             is_component_header_;
    typedef std::map<std::string, bool>   IsGlobalPackage;
    mutable IsGlobalPackage               is_global_package_;
    typedef std::map<std::string, bool>   IsStandardNamespace;
    mutable IsStandardNamespace           is_standard_namespace_;
};

// -----------------------------------------------------------------------------

template <typename InIt>
void
cool::csabase::Analyser::process_decls(InIt it, InIt end)
{
    for (; it != end; ++it)
    {
        process_decl(*it);
    }
}

template <typename T>
cool::diagnostic_builder
cool::csabase::Analyser::report(T where,
                                std::string const& check,
                                std::string const& tag,
                                std::string const& message,
                                bool always,
                                clang::DiagnosticsEngine::Level level)
{
    return report(get_location(where).location(),
                  check, tag, message, always, level);
}

// -----------------------------------------------------------------------------

template <typename T>
bool
cool::csabase::Analyser::is_component(T const* value)
{
    return is_component(get_location(value).file());
}

template <typename T>
bool
cool::csabase::Analyser::is_component_header(T const* value)
{
    return is_component_header(get_location(value).file());
}

template <typename T>
bool
cool::csabase::Analyser::is_component_source(T const* value)
{
    return is_component_source(get_location(value).file());
}

template <typename T>
T*
cool::csabase::Analyser::lookup_name_as(const std::string& name)
{
    clang::NamedDecl* nd = lookup_name(name);
    return nd ? llvm::dyn_cast<T>(nd) : 0;
}

template <typename Parent, typename Node>
const Parent *cool::csabase::Analyser::get_parent(const Node *node)
{
    for (clang::ASTContext::ParentVector pv = context()->getParents(*node);
         pv.size() >= 1;
         pv = context()->getParents(pv[0])) {
        if (const Parent *p = pv[0].get<Parent>()) {
            return p;                                                 // RETURN
        }
    }
    return 0;
}

// -----------------------------------------------------------------------------

#endif /* COOL_ANALYSER_HPP */
