// csabase_analyser.cpp                                               -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_config.h>
#include <csabase_checkregistry.h>
#include <csabase_debug.h>
#include <csabase_filenames.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_util.h>
#include <csabase_visitor.h>
#include <utils/array.hpp>
#include <clang/AST/AST.h>
#include <clang/AST/DeclVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Lexer.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Rewrite/Core/Rewriter.h>
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

csabase::Analyser::Analyser(clang::CompilerInstance& compiler,
                                  bool debug,
                                  std::vector<std::string> const& config,
                                  std::string const& name,
                                  std::string const& rewrite_dir)
: d_config(new csabase::Config(
               config.size() == 0 ?
                   std::vector<std::string>(1, "load .bdeverify") :
                   config,
           compiler.getSourceManager()))
, tool_name_(name)
, compiler_(compiler)
, d_source_manager(compiler.getSourceManager())
, visitor_(new csabase::Visitor())
, pp_observer_(new csabase::PPObserver(&d_source_manager,
                                                   d_config.get()))
, context_(0)
, rewriter_(new clang::Rewriter(compiler.getSourceManager(),
                                compiler.getLangOpts()))
, rewrite_dir_(rewrite_dir)
{
    csabase::CheckRegistry::attach(*this, *visitor_, *pp_observer_);
    compiler_.getPreprocessor().addCommentHandler(
        pp_observer_->get_comment_handler());
    compiler_.getPreprocessor().addPPCallbacks(pp_observer_);
}

csabase::Analyser::~Analyser()
{
    if (pp_observer_)
    {
        pp_observer_->detach();
    }
}

// -----------------------------------------------------------------------------

csabase::Config const*
csabase::Analyser::config() const
{
    return d_config.get();
}

// -----------------------------------------------------------------------------

std::string const&
csabase::Analyser::tool_name() const
{
    return tool_name_;
}

// -----------------------------------------------------------------------------

clang::ASTContext const*
csabase::Analyser::context() const
{
    return context_;
}

clang::ASTContext*
csabase::Analyser::context()
{
    return context_;
}

void
csabase::Analyser::context(clang::ASTContext* context)
{
    context_ = context;
    pp_observer_->Context();
}

clang::CompilerInstance&
csabase::Analyser::compiler()
{
    return compiler_;
}

clang::Rewriter&
csabase::Analyser::rewriter()
{
    return *rewriter_;
}

std::string const&
csabase::Analyser::rewrite_dir() const
{
    return rewrite_dir_;
}

// -----------------------------------------------------------------------------

clang::Sema&
csabase::Analyser::sema()
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
csabase::Analyser::toplevel() const
{
    return toplevel_;
}

std::string const&
csabase::Analyser::directory() const
{
    return directory_;
}

std::string const&
csabase::Analyser::prefix() const
{
    return prefix_;
}

std::string const&
csabase::Analyser::package() const
{
    return package_;
}

std::string const&
csabase::Analyser::group() const
{
    return group_;
}

std::string const&
csabase::Analyser::component() const
{
    return component_;
}

void
csabase::Analyser::toplevel(std::string const& path)
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
csabase::Analyser::is_component_header(std::string const& name) const
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
}

bool
csabase::Analyser::is_component_header(clang::SourceLocation loc) const
{
    return is_component_header(get_location(loc).file());
}

bool
csabase::Analyser::is_global_package(std::string const& pkg) const
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
csabase::Analyser::is_global_package() const
{
    return is_global_package(package());
}

bool
csabase::Analyser::is_standard_namespace(std::string const& ns) const
{
    IsGlobalPackage::iterator in = is_standard_namespace_.find(ns);
    if (in == is_standard_namespace_.end()) {
        llvm::Regex re("(" "^[[:space:]]*" "|" "[^[:alnum:]]" ")" +
                       ns.substr(0, ns.find(':')) +
                       "(" "[^[:alnum:]]" "|" "[[:space:]]*$" ")");
        in = is_standard_namespace_
                 .insert(std::make_pair(
                      ns, re.match(config()->value("standard_namespaces"))))
                 .first;
    }
    return in->second;
}

bool
csabase::Analyser::is_component_source(std::string const& file) const
{
    std::string::size_type pos(file.find(toplevel()));
    return pos != file.npos && pos + toplevel().size() == file.size();
}

bool
csabase::Analyser::is_component_source(clang::SourceLocation loc) const
{
    return is_component_source(get_location(loc).file());
}

bool
csabase::Analyser::is_component(std::string const& file) const
{
    return is_component_source(file)
        || is_component_header(file);
}

bool
csabase::Analyser::is_component(clang::SourceLocation loc) const
{
    return is_component(get_location(loc).file());
}

bool
csabase::Analyser::is_test_driver() const
{
    //-dk:TODO this should be configurable, e.g. using regexp
    return 6 < toplevel().size()
        && toplevel().substr(toplevel().size() - 6) == ".t.cpp";
}

bool
csabase::Analyser::is_main() const
{
    std::string::size_type size(toplevel().size());
    std::string suffix(toplevel().substr(size - std::min(size, std::string::size_type(6))));
    return suffix == ".m.cpp" || suffix == ".t.cpp";
}

// -----------------------------------------------------------------------------

csabase::diagnostic_builder
csabase::Analyser::report(clang::SourceLocation where,
                                std::string const& check,
                                std::string const& tag,
                                std::string const& message,
                                bool always,
                                clang::DiagnosticsEngine::Level level)
{
    csabase::Location location(get_location(where));
    if (   (always || is_component(location.file()))
        && !config()->suppressed(tag, where))
    {
        unsigned int id(compiler_.getDiagnostics().
            getCustomDiagID(level, tool_name() + tag + ": " + message));
        return csabase::diagnostic_builder(
            compiler_.getDiagnostics().Report(where, id));
    }
    return csabase::diagnostic_builder();
}

// -----------------------------------------------------------------------------

clang::SourceManager&
csabase::Analyser::manager() const
{
    return compiler_.getSourceManager();
}

llvm::StringRef
csabase::Analyser::get_source(clang::SourceRange range, bool exact)
{
    const char *pb = "";
    const char *pe = pb + 1;

    if (range.isValid()) {
        clang::SourceManager& sm(manager());
        clang::SourceLocation b = sm.getFileLoc(range.getBegin());
        clang::SourceLocation e = sm.getFileLoc(range.getEnd());
        clang::SourceLocation t = exact ? e : sm.getFileLoc(
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

clang::SourceRange
csabase::Analyser::get_line_range(clang::SourceLocation loc)
{
    clang::SourceRange range(loc, loc);

    if (loc.isValid()) {
        clang::SourceManager& sm(manager());
        loc = sm.getFileLoc(loc);
        clang::FileID fid = sm.getFileID(loc);
        size_t line_num = sm.getPresumedLineNumber(loc);
        range.setBegin(sm.translateLineCol(fid, line_num, 1u));
        range.setEnd(sm.translateLineCol(fid, line_num, ~0u));
    } 

    return range;
}

llvm::StringRef
csabase::Analyser::get_source_line(clang::SourceLocation loc)
{
    return get_source(get_line_range(loc), true);
}

// -----------------------------------------------------------------------------

csabase::Location
csabase::Analyser::get_location(clang::SourceLocation sl) const
{
    return csabase::Location(d_source_manager, sl);
}

csabase::Location
csabase::Analyser::get_location(clang::Decl const* decl) const
{
    return decl
        ? get_location(decl->getLocation())
        : csabase::Location();
}

csabase::Location
csabase::Analyser::get_location(clang::Expr const* expr) const
{
    return expr
        ? get_location(expr->getLocStart())
        : csabase::Location();
}

csabase::Location
csabase::Analyser::get_location(clang::Stmt const* stmt) const
{
    return stmt
        ? get_location(stmt->getLocStart())
        : csabase::Location();
}

// -----------------------------------------------------------------------------

void
csabase::Analyser::process_decl(clang::Decl const* decl)
{
    visitor_->visit(decl);
}

void
csabase::Analyser::process_translation_unit_done()
{
    config()->check_bv_stack(*this);
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
csabase::Analyser::lookup_name(std::string const& name)
{
    return ::lookup_name(sema(), name);
}

clang::TypeDecl*
csabase::Analyser::lookup_type(std::string const& name)
{
    clang::NamedDecl* decl(lookup_name(name));
    return decl? llvm::dyn_cast<clang::TypeDecl>(decl): 0;
}

// ----------------------------------------------------------------------------

bool
csabase::Analyser::is_ADL_candidate(clang::Decl const* decl)
    // Return true iff the specified 'decl' is a function or function template
    // with a parameter whose type is declared inside the package namespace.
    // This check is used to determine whether to complain about a "mis-scoped"
    // function declaration.  We wish to allow other top-level namespaces (than
    // the package namespace) to contain function declarations if those
    // functions take arguments whose types are defined in the package
    // namespace, on the suspicion that they are declaring specializations or
    // overloads which will then be found by ADL.  As an additional
    // complication, we also wish to allow legacy 'package_name' constructs, so
    // we check whether the parameter type associated classes are prefixed by
    // the component name.
{
    bool adl = false;
    clang::NamespaceDecl *pspace = lookup_name_as<clang::NamespaceDecl>(
        "::" + config()->toplevel_namespace() + "::" + package()
    );
    const clang::FunctionDecl *fd =
        llvm::dyn_cast<clang::FunctionDecl>(decl);
    if (const clang::FunctionTemplateDecl *ftd =
            llvm::dyn_cast<clang::FunctionTemplateDecl>(decl)) {
        fd = ftd->getTemplatedDecl();
    }
    if (fd) {
        unsigned n = fd->getNumParams();
        for (unsigned i = 0; !adl && i < n; ++i) {
            const clang::ParmVarDecl *pd = fd->getParamDecl(i);
            // Gain access to the protected constructors of Expr.
            struct MyExpr : public clang::Expr {
                MyExpr(clang::QualType t) :
                    clang::Expr(clang::Stmt::NoStmtClass,
                            clang::Stmt::EmptyShell()) {
                        setType(t);
                    }
            } e(pd->getOriginalType().getNonReferenceType());
            llvm::ArrayRef<clang::Expr *> ar(&e);
            clang::Sema::AssociatedNamespaceSet ns;
            clang::Sema::AssociatedClassSet cs;
            sema().FindAssociatedClassesAndNamespaces(
                    fd->getLocation(), ar, ns, cs);
            adl = ns.count(const_cast<clang::NamespaceDecl *>(pspace));

            clang::Sema::AssociatedClassSet::iterator csb = cs.begin();
            clang::Sema::AssociatedClassSet::iterator cse = cs.end();
            while (!adl && csb != cse) {
                std::string s =
                    csabase::to_lower((*csb++)->getNameAsString());
                adl = s == component() || 0 == s.find(component() + "_");
            }
        }
    }
    return adl;
}

// ----------------------------------------------------------------------------

static llvm::Regex generated("GENERATED FILE -+ DO NOT EDIT");

bool
csabase::Analyser::is_generated(clang::SourceLocation loc) const
    // Return true if this is an automatically generated file.  The criterion
    // is a first line containing "GENERATED FILE -- DO NOT EDIT".
{
    loc = d_source_manager.getFileLoc(loc);
    clang::FileID fid = d_source_manager.getFileID(loc);
    llvm::StringRef buf = d_source_manager.getBufferData(fid);
    if (generated.match(buf.split('\n').first)) {
        return true;                                                  // RETURN
    }

    // Note that the bg/eg tags below do not respect preprocessor sections.
    // They can look like
    //..
    //  #if !BSLS_COMPILERFEATURES_SIMULATE_CPP11_FEATURES
    //  #elif BSLS_COMPILERFEATURES_SIMULATE_VARIADIC_TEMPLATES
    //  // {{{ BEGIN GENERATED CODE
    //  #else
    //  // }}} END GENERATED CODE
    //  #endif
    //..
    unsigned pos = d_source_manager.getFileOffset(loc);
    static const char bg[] = "\n// {{{ BEGIN GENERATED CODE";
    static const char eg[] = "\n// }}} END GENERATED CODE";
    size_t bpos = buf.npos;
    for (size_t p = 0; (p = buf.find(bg, p)) < pos; ++p) {
        bpos = p;
    }
    return bpos != buf.npos && pos < buf.find(eg, bpos);
}
