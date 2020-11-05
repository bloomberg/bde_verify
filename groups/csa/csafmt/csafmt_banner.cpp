// csafmt_banner.cpp                                                  -*-C++-*-

#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <csabase_analyser.h>
#include <csabase_config.h>
#include <csabase_debug.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_util.h>
#include <csabase_visitor.h>
#include <csaglb_comments.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Regex.h>
#include <stddef.h>
#include <stdlib.h>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace csabase { class Visitor; }

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("banner");

// ----------------------------------------------------------------------------

namespace
{

struct data
{
    SourceLocation d_inline_banner;      // banner for inline definitions
    SourceLocation d_inline_definition;  // first inline definition
};

struct report : Report<data>
{
    INHERIT_REPORT_CTOR(report, Report, data);

    void check_comment(SourceRange comment_range);
        // Check banner in one comment.

    void operator()();
        // Report improper banners.

    void operator()(const FunctionDecl *func);
        // Look for inline function definitions ahead of banner.
};

#undef  aba
#define aba(a, b) "(" a "(" b a ")*)"
#undef  SP
#define SP "[[:space:]]*"

static llvm::Regex generic_banner(      // things that look like banners
        "//"     SP     aba("[-=_]",        SP) SP          // 1, 2
        "//" "(" SP ")" aba("[_[:alnum:]]", SP) SP          // 3, 4, 5
        "//" "(" SP ")" aba("[-=_]",        SP) SP "\r*$",  // 6, 7, 8
    llvm::Regex::Newline);

static llvm::Regex generic_separator(  // things that look like separators
    "(//.*[[:alnum:]].*)?\r*\n?"
    "((//([[:space:]]*)[-=_]"
    "("
     "([[:space:]][-=_]*( END-OF-FILE )?[[:space:]][-=_])*" "|"
     "([-=_]*( END-OF-FILE )?[-=_]*)"
    "))[[:space:]]*)\r*$",
    llvm::Regex::Newline);

#undef SP
#undef aba

llvm::Regex display("^( *//[.][.])$", llvm::Regex::Newline);

void get_displays(llvm::StringRef text,
                  llvm::SmallVector<std::pair<size_t, size_t>, 7>* displays)
{
    llvm::SmallVector<llvm::StringRef, 7> matches;

    size_t          offset = 0;
    llvm::StringRef s;
    int             n = 0;
    displays->clear();
    while (display.match(s = text.drop_front(offset), &matches)) {
        llvm::StringRef           d        = matches[0];
        std::pair<size_t, size_t> m        = mid_match(s, d);
        size_t                    matchpos = offset + m.first;
        offset                             = matchpos + d.size();

        if (n++ & 1) {
            displays->back().second = matchpos;
        }
        else {
            displays->push_back(std::make_pair(matchpos, text.size()));
        }
    }
}

void report::check_comment(SourceRange comment_range)
{
    if (!a.is_component(comment_range.getBegin())) {
        return;                                                       // RETURN
    }

    llvm::SmallVector<llvm::StringRef, 8> matches;

    llvm::StringRef comment = a.get_source(comment_range, true);
    llvm::SmallVector<std::pair<size_t, size_t>, 7> displays;
    get_displays(comment, &displays);

    size_t offset = 0;
    size_t dnum = 0;
    for (llvm::StringRef suffix = comment;
         generic_separator.match(suffix, &matches);
         suffix = comment.drop_front(offset)) {
        llvm::StringRef separator = matches[2];
        size_t separator_pos = offset + suffix.find(separator);
        offset = separator_pos + separator.size();
        while (dnum < displays.size() && displays[dnum].second < separator_pos)
            ++dnum;
        if (dnum < displays.size() &&
            displays[dnum].first <= separator_pos &&
            separator_pos <= displays[dnum].second)
            continue;
        SourceLocation separator_start =
            comment_range.getBegin().getLocWithOffset(separator_pos);
        if (m.getPresumedColumnNumber(separator_start) == 1 &&
            matches[3].size() != 79 &&
            matches[4].size() <= 1 &&
            separator.size() != matches[1].size()) {
            std::string expected_banner = matches[3].trim();
            std::string extra =
                expected_banner.substr(expected_banner.size() - 2);
            while (expected_banner.size() < 79) {
                expected_banner += extra;
            }
            expected_banner = expected_banner.substr(0, 79);
            auto report = a.report(separator_start, check_name, "BAN02",
                     "Banner ends at column %0 instead of 79");
            report << static_cast<int>(matches[3].size());
            SourceRange line_range = a.get_line_range(separator_start);
            if (line_range.isValid()) {
                a.ReplaceText(line_range, expected_banner);
            }
        }
    }

    offset = 0;
    dnum = 0;
    for (llvm::StringRef suffix = comment;
         generic_banner.match(suffix, &matches);
         suffix = comment.drop_front(offset)) {
        llvm::StringRef banner = matches[0];
        size_t banner_pos = offset + suffix.find(banner);
        offset = banner_pos + banner.rfind('\n');
        while (dnum < displays.size() && displays[dnum].second < banner_pos)
            ++dnum;
        if (dnum < displays.size() &&
            displays[dnum].first <= banner_pos &&
            banner_pos <= displays[dnum].second)
            continue;
        SourceLocation banner_start =
            comment_range.getBegin().getLocWithOffset(banner_pos);
        if (m.getPresumedColumnNumber(banner_start) != 1) {
            continue;
        }
        llvm::StringRef text = matches[4];
        if (text == "INLINE DEFINITIONS") {
            SourceLocation& ib = d.d_inline_banner;
            if (!ib.isValid()) {
                ib = banner_start;
            }
        }
        size_t text_pos = banner.find(text);
        size_t actual_last_space_pos =
            m.getPresumedColumnNumber(
                    banner_start.getLocWithOffset(text_pos)) - 1;
        size_t banner_slack = std::strtoul(
            a.config()->value("banner_slack", banner_start).c_str(),
            0, 10);
        size_t expected_last_space_pos =
            ((79 - 2 - text.size()) / 2 + 2) & ~3;
        if (actual_last_space_pos == 19) {
            // Banner text can start in column 20.
            expected_last_space_pos = 19;
        }
        if (actual_last_space_pos + banner_slack < expected_last_space_pos ||
            actual_last_space_pos > expected_last_space_pos + banner_slack) {
            std::string expected_text =
                "//" + std::string(expected_last_space_pos - 2, ' ') +
                text.str();
            const char* error = (actual_last_space_pos & 3) ?
                "Improperly centered banner text"
                " (not reachable using tab key)" :
                "Improperly centered banner text";
            SourceLocation sl = banner_start.getLocWithOffset(text_pos);
            a.report(sl, check_name, "BAN03", error);
            auto report = a.report(sl, check_name, "BAN03", "Correct text is\n%0",
                     false, DiagnosticIDs::Note);
            report << expected_text;
            SourceRange line_range = a.get_line_range(sl);
            if (line_range.isValid()) {
                a.ReplaceText(line_range, expected_text);
            }
        }

        llvm::StringRef bottom_rule = matches[7];
        if (banner.size() - banner.rfind('\n') != 80 &&
            banner.size() - banner.rfind('\r') != 80 &&
            text.size() == bottom_rule.size() &&
            matches[3] != matches[6]) {
            // It's a misaligned underline for the banner text.
            SourceLocation bottom_loc = banner_start.getLocWithOffset(
                banner.size() - bottom_rule.size());
            std::string expected_text =
                "//" + std::string(expected_last_space_pos - 2, ' ') +
                bottom_rule.str();
            a.report(bottom_loc, check_name, "BAN04",
                     "Improperly centered underlining");
            auto report = a.report(bottom_loc, check_name, "BAN04", "Correct version is\n%0",
                     false, DiagnosticIDs::Note);
            report << expected_text;
            SourceRange line_range = a.get_line_range(bottom_loc);
            if (line_range.isValid()) {
                a.ReplaceText(line_range, expected_text);
            }
        }
    }
}

void report::operator()()
{
    for (const auto& c : a.attachment<CommentData>().d_comments) {
        if (a.is_component(c.first)) {
            for (const auto& r : c.second) {
                check_comment(r);
            }
        }
    }

    SourceLocation& ib = d.d_inline_banner;
    SourceLocation& id = d.d_inline_definition;
    if (id.isValid() &&
        !(ib.isValid() && m.isBeforeInTranslationUnit(ib, id))) {
        a.report(id, check_name, "FB01",
                 "Inline functions in header must be preceded by an INLINE "
                 "DEFINITIONS banner");
    }
}

void report::operator()(const FunctionDecl *func)
{
    // Process only function definition.
    const CXXMethodDecl *md = llvm::dyn_cast<CXXMethodDecl>(func);
    SourceLocation& id = d.d_inline_definition;
    if (!id.isValid() &&
        !func->getDeclContext()->isRecord() &&
        func->doesThisDeclarationHaveABody() &&
        func->isInlineSpecified() &&
        (!md || md->isUserProvided()) &&
        a.is_component_header(func)) {
        id = func->getLocation();
    }
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    analyser.onTranslationUnitDone += report(analyser);
    visitor.onFunctionDecl         += report(analyser);
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
