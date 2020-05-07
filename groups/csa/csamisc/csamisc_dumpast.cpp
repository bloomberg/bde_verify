// csamisc_dumpast.cpp                                                -*-C++-*-

#include <clang/AST/Decl.h>
#include <clang/AST/Expr.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/StmtIterator.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <csabase_analyser.h>
#include <csabase_config.h>
#include <csabase_debug.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <llvm/Support/Casting.h>
#include <string.h>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace csabase { class PPObserver; }
namespace csabase { class Visitor; }

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name(".dump-ast");

// ----------------------------------------------------------------------------

namespace
{

struct RDecl
{
    const Decl &decl;
    RDecl(const Decl &decl) : decl(decl) { }
};

}

namespace csabase
{

template<>
llvm::raw_ostream& Debug::operator<<(const RDecl &rdecl) const
{
    llvm::raw_ostream& out = indent();
    rdecl.decl.dump(out);
    return out;
}

}

namespace
{


struct data
{
};

// Callback object invoked upon completion.
struct report : Report<data>
{
    INHERIT_REPORT_CTOR(report, Report, data);

    void operator()()
    {
        if (Debug::get_debug()) {
            Debug("Translation Unit AST Dump:\n", false)
                << RDecl(*a.context()->getTranslationUnitDecl());
        }
    }
};

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
{
    analyser.onTranslationUnitDone += report(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static RegisterCheck c1(check_name, &subscribe);

// ----------------------------------------------------------------------------
// Copyright (C) 2015 Bloomberg Finance L.P.
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
