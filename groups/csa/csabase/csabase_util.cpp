// csabase_util.cpp                                                   -*-C++-*-
#include <csabase_util.h>
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

namespace cool {
namespace csabase {

std::pair<unsigned, unsigned>
mid_mismatch(const std::string &have, const std::string &want)
{
    std::pair<unsigned, unsigned> result(0, 0);
    while (   result.first < have.size()
           && result.first < want.size()
           && have[result.first] == want[result.first]) {
        ++result.first;
    }
    while (   result.second < have.size()
           && result.second < want.size()
           && have.size() > result.first + result.second
           && have[have.size() - result.second - 1] ==
              want[want.size() - result.second - 1]) {
        ++result.second;
    }
    return result;
}

}
}
