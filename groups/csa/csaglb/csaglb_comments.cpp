// csaglb_comments.cpp                                                -*-C++-*-

#include <csaglb_comments.h>

#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_util.h>

namespace csabase { class Visitor; }

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

namespace csabase {

void CommentData::append(Analyser& analyser, SourceRange range)
{
    SourceManager& m = analyser.manager();
    d_allComments.emplace_back(range);
    CommentData::Ranges& c = d_comments[m.getFilename(range.getBegin())];
    if (c.size() != 0 && areConsecutive(m, c.back(), range)) {
        c.back().setEnd(range.getEnd());
    } else {
        c.push_back(range);
    }
}

}

namespace {

struct report : Report<CommentData>
{
    INHERIT_REPORT_CTOR(report, Report, CommentData);

    void operator()(SourceRange range);
        // The specified comment 'range' is added to the stored data.

    void operator()();
        // Inspect all comments.
};

// Comment
void report::operator()(SourceRange range)
{
    d.append(a, range);
}

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
    // Hook up the callback functions.
{
    observer.onComment += report(analyser, observer.e_Comment);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static RegisterCheck c1("", &subscribe);

// ----------------------------------------------------------------------------
// Copyright (C) 2017 Bloomberg Finance L.P.
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
