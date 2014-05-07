// csabase_analyser.cpp                                               -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyse.h>
#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_diagnosticfilter.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Sema/SemaConsumer.h>
#include <clang/AST/AST.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Preprocessor.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>
#include <string>
#include <vector>
#ident "$Id: csabase_analyse.cpp 167 2012-04-14 19:38:03Z kuehl $"

// -----------------------------------------------------------------------------

using csabase::PluginAction;

// -----------------------------------------------------------------------------

namespace
{
    class AnalyseConsumer:
        public clang::ASTConsumer
    {
    public:
        AnalyseConsumer(clang::CompilerInstance& compiler, std::string const& source, PluginAction const& plugin);
        void Initialize(clang::ASTContext& context);
        bool HandleTopLevelDecl(clang::DeclGroupRef DG);
        void HandleTranslationUnit(clang::ASTContext&);

    private:
        csabase::Analyser  analyser_;
        std::string const        source_;
    };
}

// -----------------------------------------------------------------------------

AnalyseConsumer::AnalyseConsumer(clang::CompilerInstance& compiler,
                                 std::string const& source,
                                 PluginAction const& plugin)
: analyser_(compiler,
            plugin.debug(),
            plugin.config(),
            plugin.tool_name(),
            plugin.rewrite_dir())
, source_(source)
{
    analyser_.toplevel(source);

    compiler.getDiagnostics().setClient(new csabase::DiagnosticFilter(
        analyser_, plugin.toplevel_only(), compiler.getDiagnosticOpts()));
    compiler.getDiagnostics().getClient()->BeginSourceFile(
        compiler.getLangOpts(),
        compiler.hasPreprocessor() ? &compiler.getPreprocessor() : 0);
}

// -----------------------------------------------------------------------------

void
AnalyseConsumer::Initialize(clang::ASTContext& context)
{
    analyser_.context(&context);
}

// -----------------------------------------------------------------------------

bool
AnalyseConsumer::HandleTopLevelDecl(clang::DeclGroupRef DG)
{
    analyser_.process_decls(DG.begin(), DG.end());
    return true;
}

// -----------------------------------------------------------------------------

void
AnalyseConsumer::HandleTranslationUnit(clang::ASTContext&)
{
    analyser_.process_translation_unit_done();

    std::string rd = analyser_.rewrite_dir();
    if (!rd.empty()) {
        for (clang::Rewriter::buffer_iterator
                 b = analyser_.rewriter().buffer_begin(),
                 e = analyser_.rewriter().buffer_end();
             b != e;
             b++) {
            const clang::FileEntry *fe =
                analyser_.manager().getFileEntryForID(b->first);
            llvm::SmallVector<char, 512> path(rd.begin(), rd.end());
            llvm::sys::path::append(
                path, llvm::sys::path::filename(fe->getName()));
            std::string rewritten_file =
                std::string(path.begin(), path.end()) + "-rewritten";
            std::string file_error;
            llvm::raw_fd_ostream rfdo(
                    rewritten_file.c_str(), file_error, llvm::sys::fs::F_None);
            if (file_error.empty()) {
                b->second.write(rfdo);
            } else {
                ERRS() << file_error << ": cannot open " << rewritten_file
                       << " for rewriting\n";
            }
        }
    }
}

// -----------------------------------------------------------------------------

PluginAction::PluginAction()
    : debug_()
    , config_(1, "load .bdeverify")
    , tool_name_()
    , toplevel_only_(false)
{
}

// -----------------------------------------------------------------------------

clang::ASTConsumer*
PluginAction::CreateASTConsumer(clang::CompilerInstance& compiler, llvm::StringRef source)
{
    return new AnalyseConsumer(compiler, source, *this);
}

// -----------------------------------------------------------------------------

bool PluginAction::ParseArgs(clang::CompilerInstance const& compiler,
                             std::vector<std::string> const& args)
{
    for (size_t i = 0; i < args.size(); ++i) {
        llvm::StringRef arg = args[i];
        if (arg == "debug-on")
        {
            csabase::Debug::set_debug(true);
            debug_ = true;
        }
        else if (arg == "debug-off")
        {
            csabase::Debug::set_debug(false);
            debug_ = false;
        }
        else if (arg == "toplevel-only-on")
        {
            toplevel_only_ = true;
        }
        else if (arg == "toplevel-only-off")
        {
            toplevel_only_ = false;
        }
        else if (arg.startswith("config=")) {
            config_.push_back("load " + arg.substr(7).str());
        }
        else if (arg.startswith("config-line=")) {
            config_.push_back(arg.substr(12));
        }
        else if (arg.startswith("tool=")) {
            tool_name_ = "[" + arg.substr(5).str() + "] ";
        }
        else if (arg.startswith("rewrite-dir=")) {
            rewrite_dir_ = arg.substr(12).str();
        }
        else
        {
            llvm::errs() << "unknown csabase argument = '" << arg << "'\n";
        }
    }
    return true;
}

bool
PluginAction::debug() const
{
    return debug_;
}

const std::vector<std::string>&
PluginAction::config() const
{
    return config_;
}

std::string
PluginAction::tool_name() const
{
    return tool_name_;
}

bool
PluginAction::toplevel_only() const
{
    return toplevel_only_;
}

std::string
PluginAction::rewrite_dir() const
{
    return rewrite_dir_;
}
