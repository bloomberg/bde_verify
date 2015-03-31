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
    using Report<data>::Report;

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
        do {
            if ((next = b.find('\n', prev + 1)) == b.npos) {
                next = b.size();
            }
            if (next - prev > 80) {
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
