// csafmt_nonascii.cpp                                                -*-C++-*-

#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <csabase_analyser.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_util.h>
#include <llvm/Support/MemoryBuffer.h>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <string>

namespace csabase { class Visitor; }

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("nonascii");

// ----------------------------------------------------------------------------

namespace
{

struct files
    // Callback object for inspecting files.
{
    Analyser& d_analyser;                   // Analyser object.

    files(Analyser& analyser);
        // Create a 'files' object, accessing the specified 'analyser'.

    void operator()(SourceLocation     loc,
                    std::string const &from,
                    std::string const &file);
        // The file specified by 'loc' is examined for non-ascii characters.
};

files::files(Analyser& analyser)
: d_analyser(analyser)
{
}

void files::operator()(SourceLocation     loc,
                       std::string const &from,
                       std::string const &file)
{
    const SourceManager &m = d_analyser.manager();
    const llvm::StringRef buf = m.getBufferData(m.getFileID(loc));
    const char *b = buf.begin();
    const char *e = buf.end();

    const char *begin = 0;
    for (const char *s = b; s < e; ++s) {
        if (!(*s & 0x80)) {
            if (begin != 0) {
                SourceRange bad(getOffsetRange(loc, begin - b, s - begin - 1));
                d_analyser.report(bad.getBegin(), check_name, "NA01",
                                  "Non-ASCII characters")
                    << bad;
                begin = 0;
            }
        } else {
            if (begin == 0) {
                begin = s;
            }
        }
    }
}

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
    // Hook up the callback functions.
{
    observer.onOpenFile += files(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static RegisterCheck c1(check_name, &subscribe);

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
