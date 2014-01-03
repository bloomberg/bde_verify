// csabase_tool.cpp                                                   -*-C++-*-
#include <csabase_tool.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/FrontendDiagnostic.h>
#include <clang/Frontend/TextDiagnosticBuffer.h>
#include <clang/FrontendTool/Utils.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/ADT/Statistic.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/PrettyStackTrace.h>
#include <llvm/Support/Process.h>
#include <llvm/Support/Signals.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Timer.h>
#include <set>

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

static void LLVMErrorHandler(void *UserData, const std::string &Message,
                             bool GenCrashDiag) {
    DiagnosticsEngine &Diags = *static_cast<DiagnosticsEngine*>(UserData);
    Diags.Report(diag::err_fe_error_backend) << Message;
    llvm::sys::RunInterruptHandlers();
    exit(GenCrashDiag ? 70 : 1);
}

namespace {
    class StringSetSaver : public cl::StringSaver {
      public:
        const char *SaveString(const char *Str) LLVM_OVERRIDE {
            return Storage.insert(Str).first->c_str();
        }
      private:
        std::set<std::string> Storage;
    };
}

// -----------------------------------------------------------------------------

int main(int argc_, const char **argv_)
{
    sys::PrintStackTraceOnErrorSignal();
    PrettyStackTraceProgram X(argc_, argv_);

    SmallVector<const char *, 1024> argv;
    SpecificBumpPtrAllocator<char>  ArgAllocator;
    StringSetSaver                  Saver;

    sys::Process::GetArgumentVector(argv,
                                    ArrayRef<const char *>(argv_, argc_),
                                    ArgAllocator);

    cl::ExpandResponseFiles(Saver, cl::TokenizeGNUCommandLine, argv);

    OwningPtr<CompilerInstance>           Clang(new CompilerInstance());
    IntrusiveRefCntPtr<DiagnosticIDs>     DiagID(new DiagnosticIDs());

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();

    IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts(new DiagnosticOptions());

    TextDiagnosticBuffer *DiagsBuffer = new TextDiagnosticBuffer;

    DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagsBuffer);

    bool Success = CompilerInvocation::CreateFromArgs(
        Clang->getInvocation(),
        argv.data() + 1,
        argv.data() + argv.size(),
        Diags);

    Clang->createDiagnostics();

    install_fatal_error_handler(LLVMErrorHandler, &Clang->getDiagnostics());
    DiagsBuffer->FlushDiagnostics(Clang->getDiagnostics());

    if (Success) {
        Success = ExecuteCompilerInvocation(Clang.get());
        remove_fatal_error_handler();
        llvm::llvm_shutdown();
    }

    return !Success;
}
