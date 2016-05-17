// csabase_analyse.h                                                  -*-C++-*-

#ifndef INCLUDED_CSABASE_UTILSYSE
#define INCLUDED_CSABASE_UTILSYSE

#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/FrontendAction.h>
#include <llvm/ADT/StringRef.h>
#include <string>
#include <vector>

namespace clang { class CompilerInstance; }

namespace csabase
{
class PluginAction : public clang::PluginASTAction
{
  public:
    PluginAction();

    bool debug() const;
    const std::vector<std::string>& config() const;
    std::string tool_name() const;
    std::string diagnose() const;
    std::string rewrite_dir() const;
    std::string rewrite_file() const;
    std::string diff_file() const;

  protected:
    std::unique_ptr<clang::ASTConsumer>
    CreateASTConsumer(clang::CompilerInstance& compiler,
                      llvm::StringRef source) override;

    bool ParseArgs(clang::CompilerInstance const& compiler,
                   std::vector<std::string> const& args) override;

    bool BeginInvocation(clang::CompilerInstance& compiler) override;

  private:
    bool debug_;
    std::vector<std::string> config_;
    std::string tool_name_;
    std::string diagnose_;
    std::string rewrite_dir_;
    std::string rewrite_file_;
    std::string diff_file_;
};
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
