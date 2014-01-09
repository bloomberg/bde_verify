// csabase_analyser.cpp                                               -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_config.h>
#include <csabase_checkregistry.h>
#include <csabase_filenames.h>
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
#include <llvm/Support/Regex.h>
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
    : d_config(new cool::csabase::Config(config.empty()? ".bdeverify": config))
    , tool_name_(name)
    , compiler_(compiler)
    , d_source_manager(compiler_.getSourceManager())
    , visitor_(new cool::csabase::Visitor())
    , pp_observer_(0)
    , context_(0)
{
    std::auto_ptr<cool::csabase::PPObserver> observer(new cool::csabase::PPObserver(&d_source_manager));
    pp_observer_ = observer.get();
    cool::csabase::CheckRegistry::attach(*this, *visitor_, *observer);
    compiler_.getPreprocessor().addCommentHandler(observer->get_comment_handler());
    compiler_.getPreprocessor().addPPCallbacks(observer.release());
}

cool::csabase::Analyser::~Analyser()
{
    if (pp_observer_)
    {
        pp_observer_->detach();
    }
}

// -----------------------------------------------------------------------------

cool::csabase::Config const*
cool::csabase::Analyser::config() const
{
    return d_config.get();
}

// -----------------------------------------------------------------------------

std::string const&
cool::csabase::Analyser::tool_name() const
{
    return tool_name_;
}

// -----------------------------------------------------------------------------

clang::ASTContext const*
cool::csabase::Analyser::context() const
{
    return context_;
}

clang::ASTContext*
cool::csabase::Analyser::context()
{
    return context_;
}

void
cool::csabase::Analyser::context(clang::ASTContext* context)
{
    context_ = context;
    pp_observer_->Context();
}

clang::CompilerInstance&
cool::csabase::Analyser::compiler()
{
    return compiler_;
}

// -----------------------------------------------------------------------------

clang::Sema&
cool::csabase::Analyser::sema()
{
    return compiler_.getSema();
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
    return toplevel_;
}

std::string const&
cool::csabase::Analyser::directory() const
{
    return directory_;
}


std::string const&
cool::csabase::Analyser::prefix() const
{
    return prefix_;
}

std::string const&
cool::csabase::Analyser::package() const
{
    return package_;
}

std::string const&
cool::csabase::Analyser::group() const
{
    return group_;
}

std::string const&
cool::csabase::Analyser::component() const
{
    return component_;
}

void
cool::csabase::Analyser::toplevel(std::string const& path)
{
    FileName fn(path);
    toplevel_ = fn.full();
    prefix_ = fn.full();
    for (int i = 0; i < NSS; ++i) {
        if (fn.extension().equals(source_suffixes[i])) {
            prefix_ = fn.prefix();
            break;
        }
    }
    directory_ = fn.directory();
    package_ = fn.package();
    group_ = fn.group();
    component_ = fn.component();
}

bool
cool::csabase::Analyser::is_component_header(std::string const& name) const
{
    IsComponentHeader::iterator in = is_component_header_.find(name);
    if (in != is_component_header_.end()) {
        return in->second;
    }

    FileName fn(name);

    for (int i = 0; i < NHS; ++i) {
        if (fn.extension().equals(header_suffixes[i])) {
            if (fn.component() == component_) {
                return is_component_header_[name] = true;             // RETURN
            }
            break;
        }
    }
    return is_component_header_[name] = false;

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
            llvm::StringRef srpre(prefix_);
            llvm::StringRef srdir(prefix_);
            while (srdir.size() > 0 && 
                   !llvm::sys::path::is_separator(srdir.back())) {
                srdir = srdir.drop_back(1);
            }
            srpre = srpre.drop_front(srdir.size());
            if (srbase.equals(srpre) ||
                (srpre.endswith(".t") && srbase.equals(srpre.drop_back(2)))) {
                return is_component_header_[name] = true;             // RETURN
            }
            break;
        }
    }
    return is_component_header_[name] = false;
}

bool
cool::csabase::Analyser::is_component_header(clang::SourceLocation loc) const
{
    return is_component_header(get_location(loc).file());
}

bool
cool::csabase::Analyser::is_global_package(std::string const& pkg) const
{
    IsGlobalPackage::iterator in = is_global_package_.find(pkg);
    if (in == is_global_package_.end()) {
        llvm::Regex re("(" "^[[:space:]]*" "|" "[^[:alnum:]]" ")" +
                       pkg +
                       "(" "[^[:alnum:]]" "|" "[[:space:]]*$" ")");
        in = is_global_package_.insert(
              std::make_pair(pkg, re.match(config()->value("global_packages")))
             ).first;
    }
    return in->second;
}

bool
cool::csabase::Analyser::is_global_package() const
{
    return is_global_package(package());
}

bool
cool::csabase::Analyser::is_component_source(std::string const& file) const
{
    std::string::size_type pos(file.find(toplevel()));
    return pos != file.npos && pos + toplevel().size() == file.size();
}

bool
cool::csabase::Analyser::is_component_source(clang::SourceLocation loc) const
{
    return is_component_source(get_location(loc).file());
}

bool
cool::csabase::Analyser::is_component(std::string const& file) const
{
    return is_component_source(file)
        || is_component_header(file);
}

bool
cool::csabase::Analyser::is_component(clang::SourceLocation loc) const
{
    return is_component(get_location(loc).file());
}

bool
cool::csabase::Analyser::is_test_driver() const
{
    //-dk:TODO this should be configurable, e.g. using regexp
    return 6 < toplevel().size()
        && toplevel().substr(toplevel().size() - 6) == ".t.cpp";
}

bool
cool::csabase::Analyser::is_main() const
{
    std::string::size_type size(toplevel().size());
    std::string suffix(toplevel().substr(size - std::min(size, std::string::size_type(6))));
    return suffix == ".m.cpp" || suffix == ".t.cpp";
}

// -----------------------------------------------------------------------------

cool::diagnostic_builder
cool::csabase::Analyser::report(clang::SourceLocation where,
                                std::string const& check,
                                std::string const& message,
                                bool always,
                                clang::DiagnosticsEngine::Level level)
{
    cool::csabase::Location location(get_location(where));
    if (always || is_component(location.file()))
    {
        unsigned int id(compiler_.getDiagnostics().
            getCustomDiagID(level, tool_name() + message));
        return cool::diagnostic_builder(
            compiler_.getDiagnostics().Report(where, id));
    }
    return cool::diagnostic_builder();
}

// -----------------------------------------------------------------------------

clang::SourceManager&
cool::csabase::Analyser::manager()
{
    return compiler_.getSourceManager();
}

llvm::StringRef
cool::csabase::Analyser::get_source(clang::SourceRange range, bool exact)
{
    const char *pb = "";
    const char *pe = pb + 1;

    if (range.isValid()) {
        clang::SourceManager& sm(manager());
        clang::SourceLocation b = sm.getSpellingLoc(range.getBegin());
        clang::SourceLocation e = sm.getSpellingLoc(range.getEnd());
        clang::SourceLocation t = exact ? e : sm.getSpellingLoc(
                clang::Lexer::getLocForEndOfToken(
                    e.getLocWithOffset(-1), 0, sm, context()->getLangOpts()));
        if (!t.isValid()) {
            t = e;
        }
        if (   b.isValid()
            && t.isValid()
            && sm.getFileID(b) == sm.getFileID(t)
            && sm.getFileOffset(b) <= sm.getFileOffset(t)) {
            pb = sm.getCharacterData(b);
            pe = sm.getCharacterData(t.isValid() ? t : e);
        }
    }
    return llvm::StringRef(pb, pe - pb);
}

// -----------------------------------------------------------------------------

cool::csabase::Location
cool::csabase::Analyser::get_location(clang::SourceLocation sl) const
{
    return cool::csabase::Location(d_source_manager, sl);
}

cool::csabase::Location
cool::csabase::Analyser::get_location(clang::Decl const* decl) const
{
    return decl
        ? get_location(decl->getLocStart())
        : cool::csabase::Location();
}

cool::csabase::Location
cool::csabase::Analyser::get_location(clang::Expr const* expr) const
{
    return expr
        ? get_location(expr->getLocStart())
        : cool::csabase::Location();
}

cool::csabase::Location
cool::csabase::Analyser::get_location(clang::Stmt const* stmt) const
{
    return stmt
        ? get_location(stmt->getLocStart())
        : cool::csabase::Location();
}

// -----------------------------------------------------------------------------

void
cool::csabase::Analyser::process_decl(clang::Decl const* decl)
{
    visitor_->visit(decl);
}

void
cool::csabase::Analyser::process_translation_unit_done()
{
    onTranslationUnitDone();
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
    return ::lookup_name(sema(), name);
}

clang::TypeDecl*
cool::csabase::Analyser::lookup_type(std::string const& name)
{
    clang::NamedDecl* decl(lookup_name(name));
    return decl? llvm::dyn_cast<clang::TypeDecl>(decl): 0;
}
