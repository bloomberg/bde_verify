// csafmt_whitespace.cpp                                              -*-C++-*-

#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <csabase_analyser.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_util.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Regex.h>
#include <stddef.h>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <string>
#include <utility>

namespace csabase { class Visitor; }

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("whitespace");

// ----------------------------------------------------------------------------

namespace
{

struct files
    // Callback object for inspecting files.
{
    Analyser& d_analyser;                   // Analyser object.

    files(Analyser& analyser);
        // Create a 'files' object, accessing the specified 'analyser'.

    void operator()(SourceLocation loc,
                    std::string const &,
                    std::string const &);
        // The file specified by 'loc' is examined for tab characters and
        // trailing spaces.
};

files::files(Analyser& analyser)
: d_analyser(analyser)
{
}

llvm::Regex bad_ws("\t+| +\r*\n", llvm::Regex::NoFlags);

void files::operator()(SourceLocation loc,
                       std::string const &,
                       std::string const &)
{
    const SourceManager &m = d_analyser.manager();
    llvm::StringRef buf = m.getBufferData(m.getFileID(loc));
    if (d_analyser.is_component(loc) &&
        (buf.find('\t') != buf.npos ||
         buf.find(" \n") != buf.npos ||
         buf.find(" \r\n") != buf.npos)) {
        loc = m.getLocForStartOfFile(m.getFileID(loc));
        size_t offset = 0;
        llvm::StringRef s;
        llvm::SmallVector<llvm::StringRef, 7> matches;
        while (bad_ws.match(s = buf.drop_front(offset), &matches)) {
            llvm::StringRef text = matches[0];
            size_t n = text.size();
            if (text.endswith("\r\n")) {
                --n;
            }
            std::pair<size_t, size_t> m = mid_match(s.str(), text.str());
            size_t matchpos = offset + m.first;
            offset = matchpos + n;
            SourceLocation sloc = loc.getLocWithOffset(matchpos);
            if (text[0] == '\t') {
                d_analyser.report(sloc, check_name, "TAB01",
                        "Tab character%s0 in source")
                    << static_cast<long>(n);
                d_analyser.ReplaceText(
                    sloc, n, std::string(n, ' '));
            }
            else {
                d_analyser.report(loc.getLocWithOffset(matchpos),
                        check_name, "ESP01",
                        "Space%s0 at end of line")
                    << static_cast<long>(n - 1);
                d_analyser.RemoveText(sloc, n - 1);
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
