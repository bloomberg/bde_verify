// csabase_utilsyse.h                                                  -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(INCLUDED_CSABASE_UTILSYSE)
#define INCLUDED_CSABASE_UTILSYSE 1
#ident "$Id$"

#include <clang/Frontend/FrontendPluginRegistry.h>

namespace csabase
{

class PluginAction : public clang::PluginASTAction
{
  public:
    PluginAction();

    bool debug() const;
    const std::vector<std::string>& config() const;
    std::string tool_name() const;
    bool toplevel_only() const;
    std::string rewrite_dir() const;

  protected:
    clang::ASTConsumer* CreateASTConsumer(clang::CompilerInstance& compiler,
                                          llvm::StringRef source);

    bool ParseArgs(clang::CompilerInstance const& compiler,
                   std::vector<std::string> const& args);

  private:
    bool debug_;
    std::vector<std::string> config_;
    std::string tool_name_;
    bool toplevel_only_;
    std::string rewrite_dir_;
};

}

#endif
