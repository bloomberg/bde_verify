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

static std::string const check_name("dump-ast");

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
    using Report<data>::Report;

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
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------- END-OF-FILE ----------------------------------
