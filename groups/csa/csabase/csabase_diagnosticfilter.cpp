// csabase_diagnosticfilter.cpp                                       -*-C++-*-

#include <csabase_diagnosticfilter.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_registercheck.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Lex/LexDiagnostic.h>  // IWYU pragma: keep
// IWYU pragma: no_include <clang/Basic/DiagnosticLexKinds.inc>
#include <llvm/Support/Regex.h>
#include <llvm/Support/raw_ostream.h>
#include <string>

namespace clang { class LangOptions; }
namespace clang { class Preprocessor; }
namespace clang { class TranslationUnitDecl; }

using namespace csabase;
using namespace llvm;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("diagnostic-filter");

// ----------------------------------------------------------------------------

std::set<unsigned> csabase::DiagnosticFilter::s_fail_ids;
std::map<std::string, std::set<unsigned>>
    csabase::DiagnosticFilter::s_diff_lines;

csabase::DiagnosticFilter::DiagnosticFilter(Analyser const&    analyser,
                                            std::string        diagnose,
                                            DiagnosticOptions& options)
: TextDiagnosticPrinter(errs(), &options)
, d_analyser(&analyser)
, d_diagnose(diagnose)
, d_prev_handle(false)
{
    std::string diff = analyser.diff_file();
    int fd = 0;
    if (diff == "-" || !sys::fs::openFileForRead(diff, fd)) {
        if (auto mb = MemoryBuffer::getOpenFile(fd, diff, -1)) {
            s_diff_lines[""].insert(0);  // Force s_diff_files.size() > 0.
            StringRef diffs((*mb)->getBufferStart(), (*mb)->getBufferSize());
            Regex file("^[+][+][+] +.*/([^[:space:]]+)", Regex::Newline);
            Regex lines("^@+[- 0-9,]*[+]([0-9,]+) *@", Regex::Newline);
            SmallVector<StringRef, 3> matches;
            std::string filename;
            while (diffs.size() != 0) {
                auto p = diffs.split('\n');
                if (file.match(p.first, &matches)) {
                    filename = matches[1];
                }
                else if (lines.match(p.first, &matches)) {
                    auto lc = matches[1].split(',');
                    unsigned line = 0;
                    lc.first.getAsInteger(10, line);
                    unsigned count = 1;
                    if (lc.second.size()) {
                        lc.second.getAsInteger(10, count);
                    }
                    while (count > 0) {
                        s_diff_lines[filename].insert(line + --count);
                    }
                }
                diffs = p.second;
            }
        }
    }
}

// ----------------------------------------------------------------------------

void
csabase::DiagnosticFilter::HandleDiagnostic(DiagnosticsEngine::Level level,
                                            Diagnostic const&        info)
{
    bool handle = false;
    if (level == DiagnosticsEngine::Note) {
        handle = d_prev_handle;
    }
    else if (info.getID() == diag::pp_pragma_once_in_main_file) {
        d_prev_handle = false;
    }
    else {
        if (!handle) {
            handle = level >= DiagnosticsEngine::Error;
        }
        if (!handle) {
            handle = d_diagnose == "all";
        }
        if (!handle) {
            handle = d_diagnose == "nogen" &&
                     !d_analyser->is_generated(info.getLocation());
        }
        if (!handle) {
            handle = d_diagnose == "component" &&
                     d_analyser->is_component(info.getLocation()) &&
                     !d_analyser->is_generated(info.getLocation());
        }
        if (!handle) {
            handle = d_diagnose == "main" &&
                     d_analyser->manager().getMainFileID() ==
                     d_analyser->manager().getFileID(info.getLocation()) &&
                     !d_analyser->is_generated(info.getLocation());
        }
        if (handle &&
            level == DiagnosticsEngine::Warning &&
            s_diff_lines.size() &&
            info.hasSourceManager()) {
            handle = false;
            auto &sm = info.getSourceManager();
            auto sl = info.getLocation();
            auto file = sm.getFilename(sl);
            size_t n = file.rfind('/');
            if (n != file.npos) {
                file = file.drop_front(n + 1);
            }
            auto fi = s_diff_lines.find(file);
            if (fi != s_diff_lines.end()) {
                unsigned line = sm.getSpellingLineNumber(sl);
                handle = fi->second.count(line);
            }
        }
        if (level < DiagnosticsEngine::Error) {
            d_prev_handle = handle;
        }
    }
    if (handle) {
        TextDiagnosticPrinter::HandleDiagnostic(level, info);
        if (csabase::DiagnosticFilter::is_fail(info.getID())) {
            csabase::diagnostic_builder::failed(true);
        }
    }
}

// ----------------------------------------------------------------------------

static void check(Analyser& analyser, const TranslationUnitDecl*)
{
}

// ----------------------------------------------------------------------------

static RegisterCheck register_check(check_name, &check);

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
