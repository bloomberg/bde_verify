// csabase_diagnosticfilter.h                                         -*-C++-*-

#ifndef INCLUDED_CSABASE_DIAGNOSTICFILTER_H
#define INCLUDED_CSABASE_DIAGNOSTICFILTER_H

#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <memory>
#include <map>
#include <set>
#include <string>

// ----------------------------------------------------------------------------

namespace clang { class DiagnosticOptions; }

namespace csabase { class Analyser; }
namespace csabase
{
class DiagnosticFilter : public clang::TextDiagnosticPrinter
{
  public:
    DiagnosticFilter(Analyser const&           analyser,
                     std::string               diagnose,
                     clang::DiagnosticOptions& options);

    void HandleDiagnostic(clang::DiagnosticsEngine::Level level,
                          clang::Diagnostic const&        info) override;

    static void fail_on(unsigned id);
    static bool is_fail(unsigned id);

  private:
    const Analyser                                    *d_analyser;
    std::string                                        d_diagnose;
    bool                                               d_prev_handle;
    static std::set<unsigned>                          s_fail_ids;
    static std::map<std::string, std::set<unsigned> >  s_diff_lines;
};
}

inline
void csabase::DiagnosticFilter::fail_on(unsigned id)
{
    s_fail_ids.insert(id);
}

inline
bool csabase::DiagnosticFilter::is_fail(unsigned id)
{
    return s_fail_ids.count(id);
}

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
