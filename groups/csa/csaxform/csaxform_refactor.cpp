// csaxform_refactor.cpp                                              -*-C++-*-

#include "csabase_analyser.h"
#include "csabase_debug.h"
#include "csabase_ppobserver.h"
#include "csabase_registercheck.h"
#include "csabase_report.h"
#include "csabase_visitor.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Regex.h>

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("refactor");

// ----------------------------------------------------------------------------

namespace
{

struct data
{
    std::map<std::string              /* original     */,
             std::vector<std::string> /* replacements */> d_files;
        // Include files to be replaced; each time one of the key files is
        // found to be included, it is removed and replaced by all of the files
        // in the value vector (which may be empty).

    std::map<SourceLocation /* location */, std::string /* guard */> d_ifs;
        // Guarded region beginnings.

    std::map<std::string              /* file      */,
             std::vector<SourceRange> /* locations */> d_includes;
        // Locations where file is included, including those elided by external
        // include guards.  If a file is included within a guarded region, the
        // range starts on the '#ifndef' line and ends on the '#endif' line.
        // Otherwise the range starts and ends on the '#include' line.

    std::string d_most_recent_guard;
        // The most recently seen guard in an '#ifndef' condition.
};

// Callback object invoked upon completion.
struct report : Report<data>
{
    INHERIT_REPORT_CTOR(report, Report, data);

    const std::string guard_for_file(const std::string &file);
        // Return an include guard derived from the specified 'file' name,
        // formulated as "INCLUDED_{STEM}" where "STEM" is the capitalized part
        // of 'file' after path and extension components are removed.

    const std::string file_for_guard(const std::string &guard);
        // return a file name derived from the specified 'guard'.  It consists
        // of a lower-case version of 'guard' with its "INCLUDE[D]_" prefix
        // removed and ".h" appended.

    bool is_guard(llvm::StringRef macro);
        // Return whether the specified 'macro' looks like an include guard
        // (starts with "INCLUDE_" or "INCLUDED_").

    void operator()();
        // Callback for end of compilation unit.

    void operator()(SourceLocation, const Token &, const MacroDirective *);
        // Callback for ifndef.

    void operator()(SourceLocation,
                    const Token &,
                    StringRef,
                    bool,
                    CharSourceRange,
                    const FileEntry *,
                    StringRef,
                    StringRef,
                    const Module *);
        // Callback for inclusion directive.

    void operator()(SourceLocation, SourceLocation);
        // Callback for endif.

    void operator()(SourceRange);
        // Callback for skipped region.
};

const std::string report::guard_for_file(const std::string &file)
{
    return "INCLUDED_" + llvm::sys::path::stem(file).upper();
}

const std::string report::file_for_guard(const std::string &guard)
{
    llvm::StringRef g(guard);
    if (g.startswith("INCLUDED_")) {
        g = g.drop_front(9);
    }
    else if (g.startswith("INCLUDE_")) {
        g = g.drop_front(8);
    }
    if (g.endswith("_H")) {
        g = g.drop_back(2);
    }
    return g.lower() + ".h";
}

bool report::is_guard(llvm::StringRef macro)
{
    return macro.startswith("INCLUDED_") ||
           macro.startswith("INCLUDE_");
}

void report::operator()()
{
    auto tu = a.context()->getTranslationUnitDecl();
    (void)tu;

    d.d_files["g.h"].emplace_back("a.h");
    d.d_files["g.h"].emplace_back("b.h");
    d.d_files["g.h"].emplace_back("c.h");

    for (auto &include_locations : d.d_includes) {
        auto replace_iter = d.d_files.find(include_locations.first);
        if (replace_iter != d.d_files.end()) {
            for (auto &replace_range : include_locations.second) {
                std::string rep_text = "";
                std::string sep = "";
                for (std::string &replacement : replace_iter->second) {
                    bool already_included = false;
                    auto included_iter = d.d_includes.find(replacement);
                    if (included_iter != d.d_includes.end()) {
                        for (auto &included_range : included_iter->second) {
                            if (m.getFileID(included_range.getBegin()) ==
                                m.getFileID(replace_range.getBegin())) {
                                already_included = true;
                                break;
                            }
                        }
                    }
                    if (!already_included) {
                        if (replace_range.getBegin() ==
                            replace_range.getEnd()) {
                            rep_text += sep + "#include <" + replacement + ">";
                        }
                        else {
                            rep_text += sep + "#ifndef " +
                                        guard_for_file(replacement) + "\n" +
                                        "#include <" + replacement + ">\n" +
                                        "#endif";
                        }
                        sep = "\n";
                    }
                }
                SourceRange extended_range(
                    a.get_line_range(replace_range.getBegin()).getBegin(),
                    a.get_line_range(replace_range.getEnd()).getEnd());
                a.report(extended_range.getBegin(), check_name, "RF01",
                         "Replacing \n" + a.get_source(extended_range).str() +
                         "\nwith\n" + rep_text + "\n");
                a.ReplaceText(extended_range, rep_text);
            }
        }
    }
}

void report::operator()(SourceLocation        loc,
                        const Token&          token,
                        const MacroDirective *)
{
    llvm::SmallString<64> buf;
    llvm::StringRef may_be_guard = p.getSpelling(token, buf);
    if (is_guard(may_be_guard)) {
        d.d_ifs.emplace(loc, may_be_guard);
        d.d_most_recent_guard = may_be_guard;
    }
}

void report::operator()(SourceLocation loc,
                        const Token &token,
                        StringRef file,
                        bool,
                        CharSourceRange,
                        const FileEntry *,
                        StringRef,
                        StringRef,
                        const Module *)
{
    if (d.d_most_recent_guard != guard_for_file(file)) {
        d.d_includes[file].emplace_back(SourceRange(loc, loc));
    }

}

void report::operator()(SourceLocation loc, SourceLocation ifloc)
{
    d.d_most_recent_guard = std::string();

    if (d.d_ifs.count(ifloc)) {
        llvm::StringRef src = a.get_source(SourceRange(ifloc, loc));
        std::string guard = d.d_ifs[ifloc];
        std::string file = file_for_guard(guard);
        std::string match = "^ifndef *" + guard + ".*$\n"
                            "^ *# *include *[<\"]" + file + "[\">].*$\n"
                            "(^ *# *define *" + guard + ".*$\n)?"
                            "^ *#endif";
        llvm::Regex re(match, llvm::Regex::Newline);
        if (re.match(src)) {
            d.d_includes[file].emplace_back(SourceRange(ifloc, loc));
        }
    }
}

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
{
    analyser.onTranslationUnitDone += report(analyser);
    observer.onPPIfndef += report(analyser, observer.e_Ifndef);
    observer.onPPInclusionDirective +=
        report(analyser, observer.e_InclusionDirective);
    observer.onPPEndif += report(analyser, observer.e_Endif);
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
