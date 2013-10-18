// csabase_analyser.cpp                                               -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_config.h>
#include <csabase_checkregistry.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_visitor.h>
#include <cool/array.hpp>
#include <clang/AST/AST.h>
#include <clang/AST/DeclVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Lexer.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Sema/Sema.h>
#include <clang/Sema/Lookup.h>
#include <llvm/Support/raw_ostream.h>
#include <algorithm>
#include <functional>
#include <map>
#ident "$Id: analyser.cpp 167 2012-04-14 19:38:03Z kuehl $"

// -----------------------------------------------------------------------------

cool::csabase::Analyser::Analyser(clang::CompilerInstance& compiler,
                                  bool                     debug,
                                  std::string const&       config,
                                  std::string const&       name)
    : d_config(new cool::csabase::Config(config.empty()? ".coolyser": config))
    , tool_name_(name)
    , compiler_(compiler)
    , d_source_manager(this->compiler_.getSourceManager())
    , visitor_(new cool::csabase::Visitor())
    , pp_observer_(0)
    , context_(0)
{
    std::auto_ptr<cool::csabase::PPObserver> observer(new cool::csabase::PPObserver(&this->d_source_manager));
    this->pp_observer_ = observer.get();
    cool::csabase::CheckRegistry::attach(*this, *this->visitor_, *observer);
    this->compiler_.getPreprocessor().addCommentHandler(observer->get_comment_handler());
    this->compiler_.getPreprocessor().addPPCallbacks(observer.release());
}

cool::csabase::Analyser::~Analyser()
{
    if (this->pp_observer_)
    {
        this->pp_observer_->detach();
    }
}

// -----------------------------------------------------------------------------

cool::csabase::Config const*
cool::csabase::Analyser::config() const
{
    return this->d_config.get();
}

// -----------------------------------------------------------------------------

std::string const&
cool::csabase::Analyser::tool_name() const
{
    return this->tool_name_;
}

// -----------------------------------------------------------------------------

clang::ASTContext const*
cool::csabase::Analyser::context() const
{
    return this->context_;
}

clang::ASTContext*
cool::csabase::Analyser::context()
{
    return this->context_;
}

void
cool::csabase::Analyser::context(clang::ASTContext* context)
{
    this->context_ = context;
    this->pp_observer_->Context();
}

clang::CompilerInstance&
cool::csabase::Analyser::compiler()
{
    return this->compiler_;
}

// -----------------------------------------------------------------------------

clang::Sema&
cool::csabase::Analyser::sema()
{
    return this->compiler_.getSema();
}

// -----------------------------------------------------------------------------

namespace
{
    static std::string const source_suffixes[] =
    {
        ".cpp", ".cxx", ".cc", ".C", ".CC", ".c++", ".c"
    };
    static std::string const header_suffixes[] =
    {
        ".hpp", ".hxx", ".hh", ".H", ".HH", ".h++", ".h"
    };

    bool
    compare_tail(std::string full, std::string tail)
    {
        return tail.size() < full.size()
            && std::mismatch(full.end() - tail.size(), full.end(), tail.begin()).first == full.end();
    }
}

std::string const&
cool::csabase::Analyser::toplevel() const
{
    return this->toplevel_;
}

std::string const&
cool::csabase::Analyser::directory() const
{
    return this->directory_;
}


std::string const&
cool::csabase::Analyser::prefix() const
{
    return this->prefix_;
}

std::string const&
cool::csabase::Analyser::package() const
{
    return this->package_;
}

std::string const&
cool::csabase::Analyser::group() const
{
    return this->group_;
}

std::string const&
cool::csabase::Analyser::component() const
{
    return this->component_;
}

void
cool::csabase::Analyser::toplevel(std::string const& path)
{
    this->toplevel_ = path;
    std::string const* it(std::find_if(cool::begin(::source_suffixes), cool::end(::source_suffixes),
                                       std::bind1st(std::ptr_fun(&::compare_tail), this->toplevel_)));
    this->prefix_ = it == cool::end(::source_suffixes)? path: path.substr(0, path.size() - it->size());
    std::string const& prefix(this->prefix_);
    std::string::size_type slash(prefix.rfind('/'));
    this->directory_ = prefix.substr(0, slash == prefix.npos? 0: slash + 1);
    std::string            fileroot(slash == prefix.npos? prefix: prefix.substr(slash + 1));
    std::string::size_type under(fileroot.find('_'));
    under = under != fileroot.npos && under == 1? fileroot.find('_', 2): under;
    this->package_   = fileroot.substr(0, under);
    under = under == fileroot.npos? 0: under + 1;
    if (1u == under || fileroot[1] != '_') {
        this->group_ = fileroot.substr(0, std::min<std::string::size_type>(3u, fileroot.size()));
    }
    std::string::size_type period(fileroot.find('.', under));
    this->component_ = fileroot.substr(under,
                                       period == fileroot.npos
                                       ? fileroot.npos
                                       : (period - under));
}

bool
cool::csabase::Analyser::is_component_header(std::string const& name) const
{
    std::string::size_type separator(name.rfind('.'));
    std::string            suffix(name.substr(name.npos == separator? name.size(): separator));
    std::string            base(name.substr(0, name.npos == separator? name.size(): separator));
    std::string::size_type slash(base.rfind('/'));
    if (slash != base.npos) {
        base = base.substr(slash + 1);
    }
    slash = this->prefix().rfind('/');
    std::string            pre(slash != std::string::npos? this->prefix().substr(slash + 1): this->prefix());
    return (base == pre || (base + ".t") == pre)
        && cool::end(::header_suffixes) != std::find(cool::begin(::header_suffixes), cool::end(::header_suffixes), suffix);
}

bool
cool::csabase::Analyser::is_component_header(clang::SourceLocation loc) const
{
    return this->is_component_header(this->get_location(loc).file());
}

bool
cool::csabase::Analyser::is_component_source(std::string const& file) const
{
    std::string::size_type pos(file.find(this->toplevel()));
    return pos != file.npos && pos + this->toplevel().size() == file.size();
}

bool
cool::csabase::Analyser::is_component_source(clang::SourceLocation loc) const
{
    return this->is_component_source(this->get_location(loc).file());
}

bool
cool::csabase::Analyser::is_component(std::string const& file) const
{
    return this->is_component_source(file)
        || this->is_component_header(file);
}

bool
cool::csabase::Analyser::is_component(clang::SourceLocation loc) const
{
    return this->is_component(this->get_location(loc).file());
}

bool
cool::csabase::Analyser::is_test_driver() const
{
    //-dk:TODO this should be configurable, e.g. using regexp
    return 6 < this->toplevel().size()
        && this->toplevel().substr(this->toplevel().size() - 6) == ".t.cpp";
}

bool
cool::csabase::Analyser::is_main() const
{
    std::string::size_type size(this->toplevel().size());
    std::string suffix(this->toplevel().substr(size - std::min(size, std::string::size_type(6))));
    return suffix == ".m.cpp" || suffix == ".t.cpp";
}

// -----------------------------------------------------------------------------

cool::diagnostic_builder
cool::csabase::Analyser::report(clang::SourceLocation where, std::string const& check, std::string const& message, bool always)
{
    cool::csabase::Location location(this->get_location(where));
    // if (always || location.file().find(this->prefix_) == 0)
    if (always || this->is_component(location.file()))
    {
        clang::FullSourceLoc location(where, this->d_source_manager);
        unsigned int id(this->compiler_.getDiagnostics().getCustomDiagID(clang::DiagnosticsEngine::Warning,
                                                                         this->tool_name() + message));
        return cool::diagnostic_builder(this->compiler_.getDiagnostics().Report(where, id));
    }
    return cool::diagnostic_builder();
}

// -----------------------------------------------------------------------------

clang::SourceManager&
cool::csabase::Analyser::manager()
{
    return this->compiler_.getSourceManager();
}

std::string
cool::csabase::Analyser::get_source(clang::SourceRange range)
{
    clang::SourceManager& sm(this->manager());
    clang::SourceLocation b = sm.getExpansionLoc(range.getBegin());
    clang::SourceLocation e = sm.getExpansionLoc(range.getEnd());
    clang::SourceLocation t = sm.getExpansionLoc(
               clang::Lexer::getLocForEndOfToken(e.getLocWithOffset(-1), 0, sm,
                                                    context()->getLangOpts()));
    return std::string(sm.getCharacterData(b),
                       sm.getCharacterData(t.isValid() ? t : e));
}

// -----------------------------------------------------------------------------

cool::csabase::Location
cool::csabase::Analyser::get_location(clang::SourceLocation sl) const
{
    return cool::csabase::Location(this->d_source_manager, sl);
}

cool::csabase::Location
cool::csabase::Analyser::get_location(clang::Decl const* decl) const
{
    return decl
        ? this->get_location(decl->getLocStart())
        : cool::csabase::Location();
}

cool::csabase::Location
cool::csabase::Analyser::get_location(clang::Expr const* expr) const
{
    return expr
        ? this->get_location(expr->getLocStart())
        : cool::csabase::Location();
}

cool::csabase::Location
cool::csabase::Analyser::get_location(clang::Stmt const* stmt) const
{
    return stmt
        ? this->get_location(stmt->getLocStart())
        : cool::csabase::Location();
}

// -----------------------------------------------------------------------------

void
cool::csabase::Analyser::process_decl(clang::Decl const* decl)
{
    this->visitor_->visit(decl);
}

void
cool::csabase::Analyser::process_translation_unit_done()
{
    this->onTranslationUnitDone();
}

// -----------------------------------------------------------------------------

namespace
{
    clang::NamedDecl*
    lookup_name(clang::Sema& sema, clang::DeclContext* context, std::string const& name)
    {
        std::string::size_type colons(name.find("::"));
        clang::IdentifierInfo const* info(&sema.getASTContext().Idents.get(name.substr(0, colons)));
        clang::DeclarationName declName(info);
        clang::DeclarationNameInfo const nameInfo(declName, clang::SourceLocation());
        clang::LookupResult result(sema, nameInfo,
                                   colons == name.npos
                                   ? clang::Sema::LookupTagName
                                   : clang::Sema::LookupNestedNameSpecifierName);

        if (sema.LookupQualifiedName(result, context) && result.begin() != result.end())
        {
            if (colons == name.npos)
            {
                return *result.begin();
            }
            if (clang::NamespaceDecl* decl = llvm::dyn_cast<clang::NamespaceDecl>(*result.begin()))
            {
                return ::lookup_name(sema, decl, name.substr(colons + 2));
            }
        }
        return 0;
    }

    clang::NamedDecl*
    lookup_name(clang::Sema& sema, std::string const& name)
    {
        std::string::size_type colons(name.find("::"));
        return 0 == colons
            ? ::lookup_name(sema, name.substr(2))
            : ::lookup_name(sema, sema.getASTContext().getTranslationUnitDecl(), name);
    }
}

clang::NamedDecl*
cool::csabase::Analyser::lookup_name(std::string const& name)
{
    return ::lookup_name(this->sema(), name);
}

clang::TypeDecl*
cool::csabase::Analyser::lookup_type(std::string const& name)
{
    clang::NamedDecl* decl(this->lookup_name(name));
    return decl? llvm::dyn_cast<clang::TypeDecl>(decl): 0;
}
