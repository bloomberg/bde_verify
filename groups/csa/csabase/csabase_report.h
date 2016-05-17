// csabase_report.h                                                   -*-C++-*-

#ifndef INCLUDED_CSABASE_REPORT
#define INCLUDED_CSABASE_REPORT

#include <csabase_analyser.h>
#include <csabase_ppobserver.h>

#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/CompilerInstance.h>

namespace csabase
{

template <class Data>
struct Report
{
    Report(Analyser& analyser,
           PPObserver::CallbackType type = PPObserver::e_None);
        // Create an object of this type, that will use the specified
        // 'analyser'.  Optionally specify a 'type' to identify the callback
        // that will be invoked, for preprocessor callbacks that have the same
        // signature.

    Analyser& d_analyser;             // 'analyser' from the constructor
    Analyser& a;

    Data& d_data;                     // persistent data for this check
    Data& d;

    PPObserver::CallbackType d_type;  // 'type' from the constructor
    PPObserver::CallbackType t;

    clang::SourceManager& d_source_manager;
    clang::SourceManager& m;

    clang::CompilerInstance& d_compiler;
    clang::CompilerInstance& c;

    clang::Preprocessor& d_preprocessor;
    clang::Preprocessor& p;
};

}  // end package namespace

template <class Data>
csabase::Report<Data>::Report(csabase::Analyser& analyser,
                              csabase::PPObserver::CallbackType type)
: d_analyser(analyser)
, a(analyser)
, d_data(analyser.attachment<Data>())
, d(analyser.attachment<Data>())
, d_type(type)
, t(type)
, d_source_manager(analyser.manager())
, m(analyser.manager())
, d_compiler(analyser.compiler())
, c(analyser.compiler())
, d_preprocessor(analyser.compiler().getPreprocessor())
, p(analyser.compiler().getPreprocessor())
{
}

#if 0 < _MSC_VER && _MSC_VER <= 1800
#define INHERIT_REPORT_CTOR(ME, REPORT, DATA)                                 \
    ME(csabase::Analyser& analyser,                                           \
       csabase::PPObserver::CallbackType type = csabase::PPObserver::e_None)  \
    : REPORT<DATA>(analyser, type)                                            \
    {                                                                         \
    }
#else
#define INHERIT_REPORT_CTOR(ME, REPORT, DATA) using REPORT<DATA>::REPORT;
#endif

// -----------------------------------------------------------------------------

#endif

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
