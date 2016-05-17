// csafmt_longlines.cpp                                               -*-C++-*-

#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <llvm/ADT/StringRef.h>
#include <stddef.h>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <set>
#include <string>

namespace csabase { class Visitor; }

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("longlines");

// ----------------------------------------------------------------------------

namespace
{

struct data
{
    std::set<FileID> d_files;
};

struct report : Report<data>
    // Callback object for inspecting report.
{
    INHERIT_REPORT_CTOR(report, Report, data);

    void operator()(SourceLocation      loc,
                    std::string const &,
                    std::string const &);
        // The file specified by 'loc' is examined for long lines.

    void operator()();
        // Called on end of translation unit.
};

void report::operator()(SourceLocation     loc,
                        std::string const&,
                        std::string const&)
{
    d.d_files.insert(m.getFileID(loc));

}

void report::operator()()
{
    d.d_files.insert(m.getMainFileID());

    for (auto fid : d.d_files) {
        SourceLocation loc = m.getLocForStartOfFile(fid);
        llvm::StringRef b  = m.getBufferData(fid);

        size_t prev = ~size_t(0);
        size_t next;
        size_t cr;
        do {
            if ((cr = next = b.find('\n', prev + 1)) == b.npos) {
                cr = next = b.size();
            }
            else if (b.find('\r', prev + 1) == cr - 1) {
                --cr;
            }
            if (cr - prev > 80) {
                d_analyser.report(loc.getLocWithOffset(prev + 80), check_name,
                                  "LL01",
                                  "Line exceeds 79 characters in length");
            }
        } while ((prev = next) < b.size());
    }
}

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
    // Hook up the callback functions.
{
    observer.onCloseFile += report(analyser);
    analyser.onTranslationUnitDone += report(analyser);
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
