// csafmt_headline.cpp                                                -*-C++-*-

#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <csabase_analyser.h>
#include <csabase_binder.h>
#include <csabase_debug.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_filenames.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_util.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/MemoryBuffer.h>
#include <stddef.h>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <string>
#include <utility>

namespace csabase { class Visitor; }

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("headline");

// ----------------------------------------------------------------------------

static void open_file(Analyser& analyser,
                      SourceLocation where,
                      const std::string&,
                      const std::string& name)
{
    FileName fn(name);
    std::string filename = fn.name();
    if (analyser.is_component_header(name) || analyser.is_toplevel(name)) {
        const SourceManager &m = analyser.manager();
        llvm::StringRef buf = m.getBuffer(m.getFileID(where))->getBuffer();
        if (buf.size() == 0) {
            return;
        }
        buf = buf.substr(0, buf.find('\n')).rtrim();
        std::string expectcpp("// " + filename);
        expectcpp.resize(70, ' ');
        expectcpp += "-*-C++-*-";
        std::string expectc("/* " + filename);
        expectc.resize(69, ' ');
        expectc += "-*-C-*- */";

        if (   !buf.equals(expectcpp)
            && !buf.equals(expectc)
            && buf.find_lower("GENERATED") == buf.npos) {
            std::pair<size_t, size_t> mcpp = mid_mismatch(buf, expectcpp);
            std::pair<size_t, size_t> mc = mid_mismatch(buf, expectc);
            std::pair<size_t, size_t> m;
            std::string expect;

            if (mcpp.first >= mc.first || mcpp.second >= mc.second) {
                m = mcpp;
                expect = expectcpp;
            } else {
                m = mc;
                expect = expectc;
            }
            analyser.report(where.getLocWithOffset(m.first),
                            check_name, "HL01",
                            "File headline incorrect", true);
            auto report = analyser.report(where.getLocWithOffset(m.first),
                            check_name, "HL01",
                            "Correct format is\n%0",
                            true, DiagnosticIDs::Note);
            report << expect;
            if (m.first == 0) {
                analyser.InsertTextBefore(
                    where.getLocWithOffset(m.first), expect + "\n");
            } else {
                analyser.ReplaceText(analyser.get_line_range(where), expect);
            }
        }
    }
}

// ----------------------------------------------------------------------------

static void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
{
    observer.onOpenFile += bind<Analyser&>(analyser, open_file);
}

// ----------------------------------------------------------------------------

static RegisterCheck register_observer(check_name, &subscribe);

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
