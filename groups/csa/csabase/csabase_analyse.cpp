// csabase_analyse.cpp                                                -*-C++-*-

#include <csabase_analyse.h>
#include <csabase_filenames.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclGroup.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_diagnosticfilter.h>
#include <csabase_filenames.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>
#include <limits.h>
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600  // For 'realpath'
#endif
#include <stdlib.h>
#include <stddef.h>
#include <fstream>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace clang { class ASTContext; }

// -----------------------------------------------------------------------------

using namespace clang;
using namespace clang::tooling;
using namespace csabase;

// -----------------------------------------------------------------------------

namespace
{

class AnalyseConsumer : public ASTConsumer
{
  public:
    AnalyseConsumer(CompilerInstance&   compiler,
                    std::string const&  source,
                    PluginAction const& plugin);
    void Initialize(ASTContext& context);
    bool HandleTopLevelDecl(DeclGroupRef DG);
    void ReadReplacements(std::string file);
    void HandleTranslationUnit(ASTContext&);

  private:
    Analyser analyser_;
    std::string const source_;
};
}

// -----------------------------------------------------------------------------

AnalyseConsumer::AnalyseConsumer(CompilerInstance&   compiler,
                                 std::string const&  source,
                                 PluginAction const& plugin)
: analyser_(compiler, plugin)
, source_(source)
{
    analyser_.toplevel(source);

    compiler.getDiagnostics().setClient(new DiagnosticFilter(
        analyser_, plugin.diagnose(), compiler.getDiagnosticOpts()));
    compiler.getDiagnostics().getClient()->BeginSourceFile(
        compiler.getLangOpts(),
        compiler.hasPreprocessor() ? &compiler.getPreprocessor() : 0);
}

// -----------------------------------------------------------------------------

void
AnalyseConsumer::Initialize(ASTContext& context)
{
    analyser_.context(&context);
}

// -----------------------------------------------------------------------------

bool
AnalyseConsumer::HandleTopLevelDecl(DeclGroupRef DG)
{
    analyser_.process_decls(DG.begin(), DG.end());
    return true;
}

// -----------------------------------------------------------------------------

void
AnalyseConsumer::ReadReplacements(std::string file)
{
    if (!file.empty()) {
        std::ifstream f(file);
        if (!f) {
            llvm::errs() << analyser_.toplevel()
                         << ":1:1: error: cannot open " << file
                         << " for reading\n";
        }
        else {
            int length;
            std::string mod_file;
            int rep_offset;
            int rep_length;
            std::string data;
            while (f >> length) {
                mod_file.resize(length);
                f.ignore(1).read(&mod_file[0], mod_file.size()).ignore(1);
                f >> rep_offset >> rep_length >> length;
                data.resize(length);
                f.ignore(1).read(&data[0], data.size()).ignore(1);
                analyser_.ReplaceText(mod_file, rep_offset, rep_length, data);
            }
        }
    }
}

void
AnalyseConsumer::HandleTranslationUnit(ASTContext&)
{
    analyser_.process_translation_unit_done();

    std::string rf = analyser_.rewrite_file();
    if (!rf.empty()) {
        int fd;
        std::error_code file_error = llvm::sys::fs::openFileForWrite(
            rf, fd, llvm::sys::fs::CD_OpenAlways, llvm::sys::fs::OF_Append);
        if (file_error) {
            llvm::errs() << analyser_.toplevel()
                         << ":1:1: error: " << file_error.message()
                         << ": cannot open " << rf
                         << " for writing\n";
        }
        else {
            llvm::raw_fd_ostream rfdo(fd, true);
            rfdo.SetUnbuffered();
            for (const auto &r : analyser_.replacements()) {
                std::string buf;
                llvm::raw_string_ostream os(buf);
                os << r.getFilePath().size() << " "
                   << r.getFilePath() << " "
                   << r.getOffset() << " "
                   << r.getLength() << " "
                   << r.getReplacementText().size() << " "
                   << r.getReplacementText() << "\n";
                rfdo << os.str();
            }
            rfdo.close();
            if (rfdo.has_error()) {
                rfdo.clear_error();
                llvm::errs() << analyser_.toplevel() << ":1:1: error: "
                             << "IO error closing " << rf << "\n";
            }
        }
    }

    std::string rd = analyser_.rewrite_dir();
    if (!rd.empty()) {
        if (!rf.empty()) {
            ReadReplacements(rf);
        }
        Rewriter& rw = analyser_.rewriter();
        SourceManager& m = analyser_.manager();
        std::map<std::string, clang::tooling::Replacements> mr;
        for (const auto &r : analyser_.replacements()) {
            auto &rs = mr[r.getFilePath().str()];
            clang::tooling::Replacement shifted(
                r.getFilePath(),
                rs.getShiftedCodePosition(r.getOffset()),
                r.getLength(),
                r.getReplacementText());
            rs = rs.merge(clang::tooling::Replacements(shifted));
        }
        for (const auto &m : mr) {
            clang::tooling::applyAllReplacements(m.second, rw);
        }
        Rewriter::buffer_iterator b = rw.buffer_begin();
        Rewriter::buffer_iterator e = rw.buffer_end();
        for (; b != e; b++) {
            const FileEntry *fe = m.getFileEntryForID(b->first);
            if (!fe) {
                continue;
            }
            SourceLocation loc = m.translateFileLineCol(fe, 1, 1);
            if (analyser_.diagnose() == "component") {
                if (!analyser_.is_component(loc)) {
                    continue;
                }
            }
            else if (analyser_.diagnose() == "main") {
                if (m.getMainFileID() != m.getFileID(loc)) {
                    continue;
                }
            }
            else if (analyser_.diagnose() == "nogen") {
                if (analyser_.is_generated(loc)) {
                    continue;
                }
            }
            std::string rewritten_file =
                analyser_.get_rewrite_file(fe->getName().str());
            const int MAX_TRIES = 10;
            int tries;
            llvm::SmallVector<char, 256> path;
            for (tries = 0; ++tries <= MAX_TRIES;
                 llvm::sys::fs::remove(path.data())) {
                int fd;
                std::error_code file_error =
                    llvm::sys::fs::createUniqueFile(
                        rewritten_file + "-%%%%%%%%", fd, path);
                if (file_error) {
                    llvm::errs() << analyser_.toplevel()
                                 << ":1:1: error: " << file_error.message()
                                 << ": cannot open " << path.data()
                                 << " for writing -- attempt " << tries
                                 << "\n";
                    continue;
                }
                llvm::raw_fd_ostream rfdo(fd, true);
                b->second.write(rfdo);
                rfdo.close();
                if (rfdo.has_error()) {
                    rfdo.clear_error();
                    llvm::errs() << analyser_.toplevel() << ":1:1: error: "
                                 << "IO error closing " << path.data()
                                 << " -- attempt " << tries << "\n";
                    continue;
                }
                file_error =
                    llvm::sys::fs::rename(path.data(), rewritten_file);
                if (file_error) {
                    llvm::errs() << analyser_.toplevel() << ":1:1: error: "
                                 << "cannot rename " << path.data()
                                 << " to " << rewritten_file
                                 << " -- attempt " << tries << "\n";
                    continue;
                }
                break;
            }
            if (tries == MAX_TRIES) {
                llvm::errs() << analyser_.toplevel() << ":1:1: error: "
                             << "utterly failed to produce "
                             << rewritten_file << "\n";
            }
            else {
                llvm::errs() << analyser_.toplevel() << ":1:1: note: "
                << "wrote " << rewritten_file << "\n";
            }
        }
    }
}

// -----------------------------------------------------------------------------

PluginAction::PluginAction()
: debug_()
, config_(1, "load .bdeverify")
, tool_name_()
, diagnose_("component")
{
}

// -----------------------------------------------------------------------------

std::unique_ptr<ASTConsumer>
PluginAction::CreateASTConsumer(CompilerInstance& compiler,
                                llvm::StringRef source)
{
    return std::make_unique<AnalyseConsumer>(compiler, source.str(), *this);
}

// -----------------------------------------------------------------------------

bool PluginAction::ParseArgs(CompilerInstance const& compiler,
                             std::vector<std::string> const& args)
{
    for (size_t i = 0; i < args.size(); ++i) {
        llvm::StringRef arg = args[i];
        if (arg == "debug-on")
        {
            Debug::set_debug(true);
            debug_ = true;
        }
        else if (arg == "debug-off")
        {
            Debug::set_debug(false);
            debug_ = false;
        }
        else if (arg == "toplevel-only-on")
        {
            diagnose_ = "main";
        }
        else if (arg.startswith("diagnose=")) {
            diagnose_ = arg.substr(9).str();
        }
        else if (arg.startswith("config=")) {
            config_.push_back("load " + arg.substr(7).str());
        }
        else if (arg.startswith("config-line=")) {
            config_.push_back(arg.substr(12).str());
        }
        else if (arg.startswith("tool=")) {
            tool_name_ = "[" + arg.substr(5).str() + "] ";
        }
        else if (arg.startswith("rewrite-dir=")) {
            rewrite_dir_ = arg.substr(12).str();
        }
        else if (arg.startswith("rewrite-file=")) {
            rewrite_file_ = arg.substr(13).str();
        }
        else if (arg.startswith("diff=")) {
            diff_file_ = arg.substr(5).str();
        }
        else
        {
            llvm::errs() << "unknown csabase argument = '" << arg << "'\n";
        }
    }
    return true;
}

bool PluginAction::BeginInvocation(clang::CompilerInstance& compiler)
{
    compiler.getDiagnosticClient().clear();
    compiler.getDiagnostics().Reset();
    ProcessWarningOptions(compiler.getDiagnostics(),
                          compiler.getDiagnosticOpts());
    return PluginASTAction::BeginInvocation(compiler);
}

bool PluginAction::debug() const
{
    return debug_;
}

const std::vector<std::string>& PluginAction::config() const
{
    return config_;
}

std::string PluginAction::tool_name() const
{
    return tool_name_;
}

std::string PluginAction::diagnose() const
{
    return diagnose_;
}

std::string PluginAction::rewrite_dir() const
{
    return rewrite_dir_;
}

std::string PluginAction::rewrite_file() const
{
    return rewrite_file_;
}

std::string PluginAction::diff_file() const
{
    return diff_file_;
}

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
