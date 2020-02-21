// csabase_tool.cpp                                                   -*-C++-*-

#include <csabase_tool.h>
#include <csabase_debug.h>
#include <csabase_diagnostic_builder.h>
#include <llvm/Option/ArgList.h>
#include <llvm/Support/Allocator.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Process.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/Signals.h>
#include <llvm/Support/StringSaver.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Timer.h>
#include <clang/Driver/Compilation.h>
#include <clang/Driver/Driver.h>
#include <clang/Driver/Options.h>
#include <clang/Frontend/ChainedDiagnosticConsumer.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendDiagnostic.h>
#include <clang/Frontend/SerializedDiagnosticPrinter.h>
#include <clang/Frontend/TextDiagnosticBuffer.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/FrontendTool/Utils.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>
#include <deque>
#include <memory>
#include <set>
#include <string>
#include <system_error>

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;
using namespace llvm::opt;

// Most of this code is cribbed from clang's driver.cpp.

namespace {
void
LLVMErrorHandler(void *UserData, const std::string& Message, bool GenCrashDiag)
{
    DiagnosticsEngine &Diags = *static_cast<DiagnosticsEngine*>(UserData);
    Diags.Report(diag::err_fe_error_backend) << Message;
    sys::RunInterruptHandlers();
    exit(GenCrashDiag ? 70 : 1);
}
}

std::string GetExecutablePath(const char *Argv0, bool CanonicalPrefixes)
{
    if (!CanonicalPrefixes)
        return Argv0;

    void *P = (void *)(intptr_t)GetExecutablePath;
    return sys::fs::getMainExecutable(Argv0, P);
}

int cc1_main(ArrayRef<const char *> Argv, const char *Argv0, void *MainAddr)
{
    std::unique_ptr<CompilerInstance> Clang(new CompilerInstance());
    IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());

    // Initialize targets first, so that --version shows registered targets.
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();

    IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
    TextDiagnosticBuffer *DiagsBuffer = new TextDiagnosticBuffer;
    DiagnosticsEngine     Diags(DiagID, &*DiagOpts, DiagsBuffer);
    bool                  Success = CompilerInvocation::CreateFromArgs(
        Clang->getInvocation(), Argv.begin(), Argv.end(), Diags);

    if (Clang->getHeaderSearchOpts().UseBuiltinIncludes &&
        Clang->getHeaderSearchOpts().ResourceDir.empty())
        Clang->getHeaderSearchOpts().ResourceDir =
            CompilerInvocation::GetResourcesPath(Argv0, MainAddr);

    Clang->createDiagnostics();
    if (!Clang->hasDiagnostics())
        return 1;

    install_fatal_error_handler(
        LLVMErrorHandler, static_cast<void *>(&Clang->getDiagnostics()));

    DiagsBuffer->FlushDiagnostics(Clang->getDiagnostics());
    if (!Success)
        return 1;

    Success = ExecuteCompilerInvocation(Clang.get());

    TimerGroup::printAll(errs());

    remove_fatal_error_handler();

    if (Clang->getFrontendOpts().DisableFree) {
        BuryPointer(std::move(Clang));
        return !Success;
    }

    llvm_shutdown();

    return !Success;
}

static void SetInstallDir(SmallVectorImpl<const char *>& argv,
                          Driver&                        TheDriver,
                          bool                           CanonicalPrefixes)
{
    SmallString<128> InstalledPath(argv[0]);

    if (sys::path::filename(InstalledPath) == InstalledPath)
        if (ErrorOr<std::string> Tmp = sys::findProgramByName(
                sys::path::filename(InstalledPath.str())))
            InstalledPath = *Tmp;

    // FIXME: We don't actually canonicalize this, we just make it absolute.
    if (CanonicalPrefixes)
        sys::fs::make_absolute(InstalledPath);

    InstalledPath = sys::path::parent_path(InstalledPath);
    if (sys::fs::exists(InstalledPath.c_str()))
        TheDriver.setInstalledDir(InstalledPath);
}

static int ExecuteCC1Tool(ArrayRef<const char *> argv, StringRef Tool)
{
    void *GetExecutablePathVP = (void *)(intptr_t)GetExecutablePath;
    return cc1_main(argv.slice(2), argv[0], GetExecutablePathVP);
}

int csabase::run(int argc_, const char **argv_)
{
    sys::PrintStackTraceOnErrorSignal(argv_[0], true);
    PrettyStackTraceProgram X(argc_, argv_);

    if (sys::Process::FixupStandardFileDescriptors())
        return 1;

    SmallVector<const char *, 2560> argv(argv_, argv_ + argc_);

    std::deque<std::string> scratch;
    std::unique_ptr<CompilationDatabase> Compilations;
    std::string ErrorMessage;
    bool after_dashes = false;
    std::set<std::string> defined, included;

    auto ins = [&](StringRef s, size_t i) {
        scratch.push_back(s);
        argv.insert(argv.begin() + i, scratch.back().data());
        return i + 1;
    };

    // Compilation Database Handling
    //
    // If there is a '--p=directory' option specified, look for a compilation
    // database in that directory.  If there isn't one, assume that this is a
    // deliberate choice and do not attempt to look for one elsewhere.
    //
    // If there is no such option, look for a compilation database with respect
    // to each source file until one is found, and use that one (even if other
    // compilation databases would be found with respect to further sources).
    //
    // For each source file to be processed, look it up in the compilation
    // database and use the first entry (if found).  Entries past the first are
    // ignored.  From that entry, insert all '-D' and '-I' options ahead of the
    // source file in the command line, taking include path options relative to
    // the directory specified in the compilation database command.  Macros and
    // include paths are only inserted once (even if macro definitions change)
    // and they're cumulative.

    for (size_t i = 0; i < argv.size(); ++i) {
        StringRef arg(argv[i]);
        if (arg == "-cc1") {
            break;
        }
        else if (arg.startswith("--p=")) {
            arg = arg.drop_front(4);
            Compilations = CompilationDatabase::autoDetectFromDirectory(
                arg, ErrorMessage);
            argv.erase(argv.begin() + i--);
            if (!Compilations) {
                // Allow opt out of compilation database, e.g., by '--p=-'.
                break;
            }
        }
        else if (arg == "-D") {
            StringRef def = argv[i + 1];
            defined.insert(def.split('=').first);
            ++i;
        }
        else if (arg.startswith("-D")) {
            defined.insert(arg.drop_front(2).split('=').first);
        }
        else if (arg == "-U") {
            StringRef def = argv[i + 1];
            defined.insert(def);
            ++i;
        }
        else if (arg.startswith("-U")) {
            defined.insert(arg.drop_front(2));
        }
        else if (arg == "-I") {
            StringRef dir = argv[i + 1];
            included.insert(dir);
            ++i;
        }
        else if (arg.startswith("-I")) {
            included.insert(arg.drop_front(2));
        }
        else if (after_dashes) {
            if (!Compilations) {
                Compilations = CompilationDatabase::autoDetectFromSource(
                    arg, ErrorMessage);
            }
            if (Compilations) {
                StringRef argstem = sys::path::stem(arg);
                for (auto cc : Compilations->getAllCompileCommands()) {
                    if (sys::path::stem(cc.Filename) == argstem) {
                        size_t n = cc.CommandLine.size();
                        for (size_t j = 0; j < n; ++j) {
                            StringRef ca = cc.CommandLine[j];
                            if (ca.consume_front("-D")) {
                                StringRef def = ca;
                                if (def.size() == 0 && j + 1 < n) {
                                    def = cc.CommandLine[++j];
                                }
                                if (def.size() > 0 &&
                                    defined.insert(def).second) {
                                    i = ins("-D", i);
                                    i = ins(def, i);
                                }
                            }
                            else if (ca.consume_front("-I") ||
                                     ca.consume_front("-isystem")) {
                                StringRef dir = ca;
                                if (dir.size() == 0 && j + 1 < n) {
                                    dir = cc.CommandLine[++j];
                                }
                                if (dir.size() > 0 &&
                                    included.insert(dir).second) {
                                    i = ins("-I", i);
                                    if (sys::path::is_absolute(dir)) {
                                        i = ins(dir, i);
                                    }
                                    else {
                                        SmallVector<char, 1024> path(
                                            cc.Directory.begin(),
                                            cc.Directory.end());
                                        sys::path::append(path, dir);
                                        sys::path::remove_dots(path, true);
                                        i = ins(StringRef(path.begin(),
                                                          path.size()),
                                                i);
                                    }
                                }
                            }
                        }
                        break;  // Use the first command line found only.
                    }
                }
            }
        }
        else if (arg == "--") {
            after_dashes = true;
            argv.erase(argv.begin() + i--);
            if (Compilations && i == argv.size() - 1) {
                for (const auto &s : Compilations->getAllFiles()) {
                    ins(s, i);
                }
            }
        }
    }

    InitializeNativeTarget();

    BumpPtrAllocator Alloc;
    StringSaver Saver(Alloc);

    bool MarkEOLs = argv.size() <= 1 ||
                    !StringRef(argv[1]).startswith("-cc1");
    cl::ExpandResponseFiles(Saver, cl::TokenizeGNUCommandLine, argv, MarkEOLs);

    auto FirstArg = std::find_if(argv.begin() + 1,
                                 argv.end(),
                                 [](const char *A) { return A != nullptr; });
    if (FirstArg != argv.end() && StringRef(*FirstArg).startswith("-cc1")) {
        // If -cc1 came from a response file, remove the EOL sentinels.
        if (MarkEOLs) {
            auto newEnd = std::remove(argv.begin(), argv.end(), nullptr);
            argv.resize(newEnd - argv.begin());
        }
        return ExecuteCC1Tool(argv, argv[1] + 4);
    }

    bool CanonicalPrefixes = true;
    raw_ostream *ForVersion = 0;
    for (int i = 1, size = argv.size(); i < size; ++i) {
        // Skip end-of-line response file markers
        if (argv[i] == nullptr)
            continue;
        if (StringRef(argv[i]) == "-no-canonical-prefixes") {
            CanonicalPrefixes = false;
        }
        else if (StringRef(argv[i]) == "--version") {
            ForVersion = &outs();
        }
        else if (!ForVersion && StringRef(argv[i]) == "-v") {
            ForVersion = &errs();
        }
    }

    std::string Path = GetExecutablePath(argv[0], CanonicalPrefixes);
    StringRef ExFile = StringRef(sys::path::filename(Path));

    if (ForVersion) {
        StringRef Name = ExFile.drop_back(ExFile.endswith("_bin") ? 4 : 0);
        *ForVersion << Name << " version " BDE_VERIFY_VERSION " based on\n";
    }

    IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions;
    std::unique_ptr<OptTable> Opts(createDriverOptTable());
    unsigned MissingIndex, MissingCount;
    InputArgList Args = Opts->ParseArgs(argv, MissingIndex, MissingCount);
    (void)ParseDiagnosticArgs(*DiagOpts, Args);

    TextDiagnosticPrinter *DiagClient =
        new TextDiagnosticPrinter(errs(), &*DiagOpts);
    DiagClient->setPrefix(ExFile);

    IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());

    DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagClient);

    if (!DiagOpts->DiagnosticSerializationFile.empty()) {
        auto SerializedConsumer = clang::serialized_diags::create(
                      DiagOpts->DiagnosticSerializationFile, &*DiagOpts, true);
        Diags.setClient(new ChainedDiagnosticConsumer(
            Diags.takeClient(), std::move(SerializedConsumer)));
    }

    ProcessWarningOptions(Diags, *DiagOpts, false);

    Driver TheDriver(Path, sys::getDefaultTargetTriple(), Diags);

#undef  CCF
#define CCF(D, X, O)                                                          \
    if ((D.X = !!::getenv(#O))) D.X##Filename = ::getenv(#O "_FILE")
    CCF(TheDriver, CCPrintOptions, CC_PRINT_OPTIONS);
    CCF(TheDriver, CCPrintHeaders, CC_PRINT_HEADERS);
    CCF(TheDriver, CCLogDiagnostics, CC_LOG_DIAGNOSTICS);
#undef CCF

    SetInstallDir(argv, TheDriver, CanonicalPrefixes);

    std::unique_ptr<Compilation> C(TheDriver.BuildCompilation(argv));
    int                          Res = 0;
    SmallVector<std::pair<int, const Command *>, 4> FailingCommands;
    if (C.get())
        Res = TheDriver.ExecuteCompilation(*C, FailingCommands);

    for (const auto& P : FailingCommands) {
        int            CommandRes = P.first;
        const Command *FailingCommand = P.second;
        if (!Res)
            Res = CommandRes;

        bool DiagnoseCrash = CommandRes < 0 || CommandRes == 70;
#ifdef LLVM_ON_WIN32
    DiagnoseCrash |= CommandRes == 3;
#endif
    if (DiagnoseCrash) {
      TheDriver.generateCompilationDiagnostics(*C, *FailingCommand);
      break;
    }
  }

  Diags.getClient()->finish();

  TimerGroup::printAll(errs());

  llvm_shutdown();

#ifdef LLVM_ON_WIN32
  if (Res < 0)
    Res = 1;
#endif

  return Res;
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
