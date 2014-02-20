// csabase_coolyser.cpp                                               -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_coolyse.h>
#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_diagnosticfilter.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/Sema/SemaConsumer.h>
#include <clang/AST/AST.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Preprocessor.h>
#include <llvm/Support/raw_ostream.h>
#include <string>
#include <vector>
#ident "$Id: coolyse.cpp 167 2012-04-14 19:38:03Z kuehl $"

// -----------------------------------------------------------------------------

namespace
{

class PluginAction : public clang::PluginASTAction
{
  public:
    PluginAction();

    bool debug() const;
    const std::vector<std::string>& config() const;
    std::string tool_name() const;

  protected:
    clang::ASTConsumer* CreateASTConsumer(clang::CompilerInstance& compiler,
                                          llvm::StringRef source);

    bool ParseArgs(clang::CompilerInstance const& compiler,
                   std::vector<std::string> const& args);

  private:
    bool debug_;
    std::vector<std::string> config_;
    std::string tool_name_;
};

}

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
        cool::csabase::Analyser  analyser_;
        std::string const        source_;
    };
}

// -----------------------------------------------------------------------------

AnalyseConsumer::AnalyseConsumer(clang::CompilerInstance& compiler,
                                 std::string const&       source,
                                 PluginAction const&      plugin)
    : analyser_(compiler, plugin.debug(), plugin.config(), plugin.tool_name())
    , source_(source)
{
    analyser_.toplevel(source);

    compiler.getDiagnostics().setClient(new cool::csabase::DiagnosticFilter(
        analyser_, compiler.getDiagnosticOpts()));
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
}

// -----------------------------------------------------------------------------

PluginAction::PluginAction()
    : debug_()
    , config_(1, "load .bdeverify")
    , tool_name_()
{
}

// -----------------------------------------------------------------------------

clang::ASTConsumer*
PluginAction::CreateASTConsumer(clang::CompilerInstance& compiler, llvm::StringRef source)
{
    return new AnalyseConsumer(compiler, source, *this);
}

// -----------------------------------------------------------------------------

bool
PluginAction::ParseArgs(clang::CompilerInstance const& compiler, std::vector<std::string> const& args)
{
    for (std::vector<std::string>::const_iterator it(args.begin()), end(args.end()); it != end; ++it)
    {
        if (*it == "debug-on")
        {
            cool::csabase::Debug::set_debug(true);
            debug_ = true;
        }
        else if (*it == "debug-off")
        {
            cool::csabase::Debug::set_debug(false);
            debug_ = false;
        }
        else if (7 < it->size() && it->substr(0, 7) == "config=") {
            config_.push_back("load " + it->substr(7));
        }
        else if (12 < it->size() && it->substr(0, 12) == "config-line=") {
            config_.push_back(it->substr(12));
        }
        else if(5 < it->size() && it->substr(0, 5) == "tool=") {
            tool_name_ = "[" + it->substr(5) + "] ";
        }
        else
        {
            llvm::errs() << "unknown bdeverify argument = '" << *it << "'\n";
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

// -----------------------------------------------------------------------------

clang::FrontendPluginRegistry::Add<PluginAction> registerPlugin(
    "bdeverify", "analyse source");
