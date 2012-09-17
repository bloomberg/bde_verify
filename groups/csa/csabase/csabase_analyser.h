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
             bool debug, std::string const& config, std::string const& name);
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

    cool::diagnostic_builder report(clang::SourceLocation, std::string const&, std::string const&, bool always = false);
    template <typename T> cool::diagnostic_builder report(T, std::string const&, std::string const&, bool always = false);

    clang::SourceManager& manager();
    std::string             get_source(clang::SourceRange);
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
    bool hasContext() const { return this->context_; }

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
};

// -----------------------------------------------------------------------------

template <typename InIt>
void
cool::csabase::Analyser::process_decls(InIt it, InIt end)
{
    for (; it != end; ++it)
    {
        this->process_decl(*it);
    }
}

template <typename T>
cool::diagnostic_builder
cool::csabase::Analyser::report(T where, std::string const& check, std::string const& message, bool always)
{
    return this->report(where->getLocStart(), check, message, always);
}

// -----------------------------------------------------------------------------

template <typename T>
bool
cool::csabase::Analyser::is_component(T const* value)
{
    return this->is_component(this->get_location(value).file());
}

template <typename T>
bool
cool::csabase::Analyser::is_component_header(T const* value)
{
    return this->is_component_header(this->get_location(value).file());
}

template <typename T>
bool
cool::csabase::Analyser::is_component_source(T const* value)
{
    return this->is_component_source(this->get_location(value).file());
}

// -----------------------------------------------------------------------------

#endif /* COOL_ANALYSER_HPP */
