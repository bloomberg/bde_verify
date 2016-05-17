// csabase_tool.cpp                                                   -*-C++-*-

#include <csabase_tool.h>
#include <csabase_debug.h>
#include <csabase_diagnostic_builder.h>
#include <llvm/Option/ArgList.h>
#include <llvm/Support/Allocator.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/ManagedStatic.h>
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
#include <clang/Tooling/Tooling.h>
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

static void FixupDiagPrefixExeName(TextDiagnosticPrinter *DiagClient,
                                   const std::string&     Path)
{
    StringRef ExeBasename(sys::path::filename(Path));
    DiagClient->setPrefix(ExeBasename);
}

// This lets us create the DiagnosticsEngine with a properly-filled-out
// DiagnosticOptions instance.
static DiagnosticOptions *
CreateAndPopulateDiagOpts(ArrayRef<const char *> argv)
{
    auto                      *DiagOpts = new DiagnosticOptions;
    std::unique_ptr<OptTable>  Opts(createDriverOptTable());
    unsigned                   MissingArgIndex, MissingArgCount;
    InputArgList               Args =
        Opts->ParseArgs(argv, MissingArgIndex, MissingArgCount);
    (void)ParseDiagnosticArgs(*DiagOpts, Args);
    return DiagOpts;
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
    sys::PrintStackTraceOnErrorSignal();
    PrettyStackTraceProgram X(argc_, argv_);

    if (sys::Process::FixupStandardFileDescriptors())
        return 1;

    SmallVector<const char *, 256> argv;
    SpecificBumpPtrAllocator<char> ArgAllocator;
    std::error_code                EC = sys::Process::GetArgumentVector(
                               argv, makeArrayRef(argv_, argc_), ArgAllocator);
    if (EC) {
        errs() << "error: couldn't get arguments: " << EC.message() << '\n';
        return 1;
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
    for (int i = 1, size = argv.size(); i < size; ++i) {
        // Skip end-of-line response file markers
        if (argv[i] == nullptr)
            continue;
        if (StringRef(argv[i]) == "-no-canonical-prefixes") {
            CanonicalPrefixes = false;
            break;
        }
    }

    std::string Path = GetExecutablePath(argv[0], CanonicalPrefixes);

    IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts =
        CreateAndPopulateDiagOpts(argv);

    TextDiagnosticPrinter *DiagClient =
        new TextDiagnosticPrinter(errs(), &*DiagOpts);
    FixupDiagPrefixExeName(DiagClient, Path);

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
