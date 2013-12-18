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
#include <llvm/Support/Path.h>
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
        ".cpp", ".cc", ".cxx", ".C", ".CC", ".c++", ".c"
    };
    const int NSS = sizeof source_suffixes / sizeof *source_suffixes;

    static std::string const header_suffixes[] =
    {
        ".h", ".hpp", ".hxx", ".hh", ".H", ".HH", ".h++"
    };
    const int NHS = sizeof header_suffixes / sizeof *header_suffixes;
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
    llvm::StringRef srpath(path);
    llvm::StringRef srext = llvm::sys::path::extension(srpath);

    this->toplevel_ = path;
    this->prefix_ = path;
    for (int i = 0; i < NSS; ++i) {
        if (srext.equals(source_suffixes[i])) {
            this->prefix_ = srpath.drop_back(srext.size());
            break;
        }
    }
    llvm::StringRef srdir(this->prefix_);
    while (srdir.size() > 0 && !llvm::sys::path::is_separator(srdir.back())) {
        srdir = srdir.drop_back(1);
    }
    this->directory_ = srdir;

    llvm::StringRef srroot = srpath.drop_front(srdir.size());
    size_t under = srroot.find('_');
    if (under == 1) {
        under = srroot.find('_', 2);
    }
    this->package_ = srroot.slice(0, under);
    ++under;
    if (under == 1 || srroot[1] != '_') {
        this->group_ = srroot.substr(0, 3);
    }
    this->component_ = srroot.slice(under, srroot.find('.'));
}

bool
cool::csabase::Analyser::is_component_header(std::string const& name) const
{
    IsComponentHeader::iterator in = this->is_component_header_.find(name);
    if (in != this->is_component_header_.end()) {
        return in->second;
    }

    llvm::StringRef srname(name);
    llvm::StringRef srsuffix = srname.substr(srname.rfind('.'));
    llvm::StringRef srbase = srname.drop_back(srsuffix.size());
    llvm::StringRef srdir = srbase;
    while (srdir.size() > 0 && !llvm::sys::path::is_separator(srdir.back())) {
        srdir = srdir.drop_back(1);
    }
    srbase = srbase.drop_front(srdir.size());

    for (int i = 0; i < NHS; ++i) {
        if (srsuffix.equals(header_suffixes[i])) {
            llvm::StringRef srpre(this->prefix_);
            llvm::StringRef srdir(this->prefix_);
            while (srdir.size() > 0 && 
                   !llvm::sys::path::is_separator(srdir.back())) {
                srdir = srdir.drop_back(1);
            }
            srpre = srpre.drop_front(srdir.size());
            if (srbase.equals(srpre) ||
                (srpre.endswith(".t") && srbase.equals(srpre.drop_back(2)))) {
                return this->is_component_header_[name] = true;       // RETURN
            }
            break;
        }
    }
    return this->is_component_header_[name] = false;
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
                return lookup_name(sema, decl, name.substr(colons + 2));
            }
        }
        return 0;
    }

    clang::NamedDecl*
    lookup_name(clang::Sema& sema, std::string const& name)
    {
        std::string::size_type colons(name.find("::"));
        return 0 == colons
            ? lookup_name(sema, name.substr(2))
            : lookup_name(sema, sema.getASTContext().getTranslationUnitDecl(), name);
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
