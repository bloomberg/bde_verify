// csafmt_comments.cpp                                                -*-C++-*-

#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <csabase_analyser.h>
#include <csabase_config.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_util.h>
#include <csaglb_comments.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/Regex.h>
#include <stddef.h>
#include <stdlib.h>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <cctype>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace csabase { class Visitor; }

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("comments");

// ----------------------------------------------------------------------------

namespace
{

struct data
{
    typedef std::set<Range> Reported;
    Reported d_reported;
};

struct report : Report<data>
    // Callback object for inspecting report.
{
    INHERIT_REPORT_CTOR(report, Report, data);

    void operator()();
        // Inspect all comments.

    void check_fvs(SourceRange range, llvm::StringRef comment);
        // Warn about comment containing "fully value semantic".

    void check_pp(SourceRange range, llvm::StringRef comment);
        // Warn about comment containing "pure procedure(s)".

    void check_mr(SourceRange range, llvm::StringRef comment);
        // Warn about comment containing "modifiable reference".

    void check_bubble(SourceRange range, llvm::StringRef comment);
        // Warn about comment containing badly formed inheritance diagram.

    void report_bubble(const Range &r, llvm::StringRef text);
        // Warn about a single bad inheritance bubble at the specified 'r'
        // containing the specified 'text'.

    void check_wrapped(SourceRange range, llvm::StringRef comment);
        // Warn about comment containing incorrectly wrapped text.

    void check_purpose(SourceRange range, llvm::StringRef comment);
        // Warn about incorrectly formatted @PURPOSE line.

    void check_description(SourceRange range, llvm::StringRef comment);
        // Warn if the @DESCRIPTION doesn't contain the component name.
};

void report::operator()()
{
    for (auto& c : a.attachment<CommentData>().d_comments) {
        const std::string& file_name = c.first;
        if (a.is_component(file_name)) {
            for (auto& r : c.second) {
                llvm::StringRef comment = a.get_source(r, true);
                check_fvs(r, comment);
                check_pp(r, comment);
                check_mr(r, comment);
                check_bubble(r, comment);
                check_wrapped(r, comment);
                check_purpose(r, comment);
                check_description(r, comment);
            }
        }
    }
}

llvm::Regex fvs(
    "fully" "[^_[:alnum:]]*" "value" "[^_[:alnum:]]*" "semantic",
    llvm::Regex::IgnoreCase);

void report::check_fvs(SourceRange range, llvm::StringRef comment)
{
    llvm::SmallVector<llvm::StringRef, 7> matches;

    size_t offset = 0;
    llvm::StringRef s;
    while (fvs.match(s = comment.drop_front(offset), &matches)) {
        llvm::StringRef text = matches[0];
        std::pair<size_t, size_t> mm = mid_match(s, text);
        size_t matchpos = offset + mm.first;
        offset = matchpos + text.size();
        a.report(range.getBegin().getLocWithOffset(matchpos),
                 check_name, "FVS01",
                 "The term \"%0\" is deprecated; use a description "
                 "appropriate to the component type")
            << text
            << getOffsetRange(range, matchpos, offset - 1 - matchpos);
    }
}

llvm::Regex pp(
    "pure" "[^_[:alnum:]]*" "procedure(s?)",
    llvm::Regex::IgnoreCase);

void report::check_pp(SourceRange range, llvm::StringRef comment)
{
    llvm::SmallVector<llvm::StringRef, 7> matches;

    size_t offset = 0;
    llvm::StringRef s;
    while (pp.match(s = comment.drop_front(offset), &matches)) {
        llvm::StringRef text = matches[0];
        std::pair<size_t, size_t> mm = mid_match(s, text);
        size_t matchpos = offset + mm.first;
        offset = matchpos + text.size();
        a.report(range.getBegin().getLocWithOffset(matchpos),
                 check_name, "PP01",
                 "The term \"%0\" is deprecated; use 'function%1'")
            << text
            << (matches[1].size() == 1 ? "s" : "")
            << getOffsetRange(range, matchpos, offset - 1 - matchpos);
    }
}

llvm::Regex mr(
    "((non-?)?" "modifiable)" "[^_[:alnum:]]*" "(references?)",
    llvm::Regex::IgnoreCase);

void report::check_mr(SourceRange range, llvm::StringRef comment)
{
    llvm::SmallVector<llvm::StringRef, 7> matches;

    size_t offset = 0;
    llvm::StringRef s;
    while (mr.match(s = comment.drop_front(offset), &matches)) {
        llvm::StringRef text = matches[0];
        std::pair<size_t, size_t> mm = mid_match(s, text);
        size_t matchpos = offset + mm.first;
        offset = matchpos + text.size();
        a.report(range.getBegin().getLocWithOffset(matchpos),
                 check_name, "MOR01",
                 "The term \"%0 %1\" is deprecated; use \"%1 "
                 "offering %0 access\"")
            << matches[1]
            << matches[3]
            << getOffsetRange(range, matchpos, offset - 1 - matchpos);
    }
}


llvm::Regex bad_bubble(
                             "([(]" "[[:blank:]]*"              // 1
                                    "([[:alnum:]_:]+)"          // 2
                                    "[[:blank:]]*"
                             "[)])" ".*\n"
        "//" "([[:blank:]]+)" "[|]" "[[:blank:]]*.*\n"          // 3
    "(" "//" "\\3"            "[|]" "[[:blank:]]*.*\n" ")*"     // 4
        "//" "\\3"            "[V]" "[[:blank:]]*.*\n"
        "//" "[[:blank:]]*"  "([(]" "[[:blank:]]*"              // 5
                                    "([[:alnum:]_:]+)"          // 6
                                    "[[:blank:]]*"
                              "[)])",
    llvm::Regex::Newline);

llvm::Regex good_bubble(
    "//( *)" " [,](-+)[.]" "\r*\n"
    "//\\1"  "[(]  .* [)]" "\r*\n"
    "//\\1"  " [`]\\2[']"  "\r*$",
    llvm::Regex::Newline);

std::string bubble(llvm::StringRef s, size_t column)
{
    std::string lead = "\n//" + std::string(column > 4 ? column - 4 : 1, ' ');
    return lead + " ,"  + std::string(s.size() + 1, '-') + "." +
           lead + "(  " + s.str()                        + " )" +
           lead + " `"  + std::string(s.size() + 1, '-') + "'";
}

void report::report_bubble(const Range &r, llvm::StringRef text)
{
    auto& reported_ranges = d.d_reported;
    if (!reported_ranges.count(r)) {
        reported_ranges.insert(r);
        a.report(r.from().location(), check_name, "BADB01",
                 "Incorrectly formed inheritance bubble")
            << SourceRange(r.from().location(), r.to().location());
        a.report(r.from().location(), check_name, "BADB01",
                 "Correct format is%0",
                 false, DiagnosticIDs::Note)
            << bubble(text, r.from().column());
    }
}

void report::check_bubble(SourceRange range, llvm::StringRef comment)
{
    llvm::SmallVector<llvm::StringRef, 7> matches;

    size_t offset = 0;
    llvm::StringRef s;
    size_t left_offset = comment.size();
    size_t leftmost_position = comment.size();

    while (good_bubble.match(s = comment.drop_front(offset), &matches)) {
        llvm::StringRef text = matches[0];
        std::pair<size_t, size_t> mm = mid_match(s, text);
        size_t matchpos = offset + mm.first;
        offset = matchpos + text.size();
        if (matches[1].size() < left_offset) {
            left_offset = matches[1].size();
            leftmost_position =
                matchpos + comment.drop_front(matchpos).find('\n') + 1;
        }
    }
    if (left_offset != 2 && left_offset != comment.size()) {
        a.report(range.getBegin().getLocWithOffset(leftmost_position + 4),
                 check_name, "AD01",
                 "Display should begin in column 5 (from start of comment)");
    }

    offset = 0;
    while (bad_bubble.match(s = comment.drop_front(offset), &matches)) {
        llvm::StringRef text = matches[0];
        std::pair<size_t, size_t> mm = mid_match(s, text);
        size_t matchpos = offset + mm.first;
        offset = matchpos + matches[1].size();

        Range b1(m, getOffsetRange(range, matchpos, matches[1].size() - 1));
        report_bubble(b1, matches[2]);

        Range b2(
            m,
            getOffsetRange(range,
                           matchpos + matches[0].size() - matches[5].size(),
                           matches[5].size() - 1));
        report_bubble(b2, matches[6]);
    }
}

std::pair<size_t, size_t> bad_wrap_pos(llvm::StringRef text, size_t ll)
    // Return the position of a word in the specified 'text' that can fit on
    // the previous line, where line length is the specified 'll'.  If there is
    // no such word, return a pair of 'npos'.
{
    size_t offset = 0;
    for (;;) {
        size_t b = text.find("// ", offset);
        if (b == text.npos) {
            break;
        }
        size_t ecr = 0;
        size_t e = text.find('\n', b);
        if (e == text.npos) {
            e = text.size();
        }
        else if (text[e - 1] == '\r') {
            ecr = 1;
        }
        offset = e;
        size_t nextb = text.find("// ", e);
        if (nextb == text.npos) {
            break;
        }
        size_t nexte = text.find('\n', nextb);
        if (nexte == text.npos) {
            nexte = text.size();
        }
        enum State { e_C, e_Q1, e_Q2 } state = e_C;
        size_t i;
        for (i = nextb + 3; i < nexte; ++i) {
            char c = text[i];
            if (state == e_C) {
                if (c == ' ' || c == '\r') {
                    break;
                }
                state = c == '\'' && text[i - 1] == ' ' ? e_Q1 :
                        c == '"'  && text[i - 1] == ' ' ? e_Q2 :
                                                          e_C;
            }
            else if (state == e_Q1) {
                state = c == '\'' ? e_C : e_Q1;
            }
            else if (state == e_Q2) {
                state = c == '"'  ? e_C : e_Q2;
            }
        }

        if (e > ecr && text[e - 1 - ecr] == '.') {
            // Double space after periods.
            ++e;
        }

        if ((e - (b + 3) - ecr) + (i - (nextb + 3)) + 1 <= ll &&
            text.substr(nextb + 3, i - (nextb + 3)).find_first_of(
                "abcdefghijklmnopqrstuvwxyz") != text.npos) {
            return std::make_pair(nextb + 3, i - 1);
        }
    }
    return std::make_pair(text.npos, text.npos);
}

llvm::Regex display("^( *//[.][.])$", llvm::Regex::Newline);

void get_displays(llvm::StringRef text,
                  llvm::SmallVector<std::pair<size_t, size_t>, 7>* displays)
{
    llvm::SmallVector<llvm::StringRef, 7> matches;

    size_t offset = 0;
    llvm::StringRef s;
    int n = 0;
    displays->clear();
    while (display.match(s = text.drop_front(offset), &matches)) {
        llvm::StringRef d = matches[0];
        std::pair<size_t, size_t> mm = mid_match(s, d);
        size_t matchpos = offset + mm.first;
        offset = matchpos + d.size();

        if (n++ & 1) {
            displays->back().second = matchpos;
        } else {
            displays->push_back(std::make_pair(matchpos, text.size()));
        }
    }
}

llvm::Regex block_comment("((^ *// [^[ ].*\r*$\n?){2,})", llvm::Regex::Newline);
llvm::Regex banner("^ *(// ?([-=_] ?)+)\r*$", llvm::Regex::Newline);
llvm::Regex copyright("Copyright.*[[:digit:]]{4}", llvm::Regex::IgnoreCase);
llvm::Regex sentence_end(" [a-z]+[.] ", llvm::Regex::NoFlags);

void report::check_wrapped(SourceRange range, llvm::StringRef comment)
{
    llvm::SmallVector<llvm::StringRef, 7> matches;
    llvm::SmallVector<llvm::StringRef, 7> banners;
    llvm::SmallVector<llvm::StringRef, 7> periods;
    llvm::SmallVector<std::pair<size_t, size_t>, 7> displays;

    if (copyright.match(comment) && banner.match(comment)) {
        return;                                                       // RETURN
    }

    get_displays(comment, &displays);

    size_t dnum = 0;
    size_t offset = 0;
    llvm::StringRef s;
    size_t wrap_slack = std::strtoul(
        a.config()->value("wrap_slack", range.getBegin()).c_str(), 0, 10);
    while (block_comment.match(s = comment.drop_front(offset), &matches)) {
        llvm::StringRef text = matches[0];
        std::pair<size_t, size_t> mm = mid_match(s, text);
        size_t matchpos = offset + mm.first;
        offset = matchpos + text.size();

        while (dnum < displays.size() && displays[dnum].second < matchpos) {
            ++dnum;
        }
        if (   dnum < displays.size()
            && displays[dnum].first <= matchpos
            && matchpos <= displays[dnum].second) {
            offset = displays[dnum].second;
            continue;
        }

        size_t n = text.find('\n');
        size_t c = text.find("//", n);
        size_t ll = banner.match(text, &banners) ? banners[1].size() - 3 :
                                                   77 - (c - n);

        size_t sp = 0;
        while (sentence_end.match(text.drop_front(sp), &periods)) {
            sp += text.drop_front(sp).find(periods[0]) + periods[0].size();
            if (sp < text.size() &&
                !std::isspace(text[sp] & 0xFF) &&
                !std::islower(text[sp] & 0xFF) &&
                !(text.slice(0, sp).count('\'') & 1)) {
                a.report(
                    range.getBegin().getLocWithOffset(matchpos + sp - 2),
                    check_name, "PSS01",
                    "Use two spaces after a period - consider using bdewrap");
            }
        }

        std::pair<size_t, size_t> bad_pos =
            bad_wrap_pos(text, ll - wrap_slack);
        if (bad_pos.first != text.npos) {
            a.report(
                range.getBegin().getLocWithOffset(matchpos + bad_pos.first),
                check_name, "BW01",
                "This text fits on the previous line - consider using bdewrap")
                << getOffsetRange(range,
                                  matchpos + bad_pos.first,
                                  bad_pos.second - bad_pos.first);
        }
    }
}

llvm::Regex loose_purpose(
    "^//"
    "[[:blank:]]*"
    "@"
    "[[:blank:]]*"
    "PURPOSE"
    "[[:blank:]]*"
    ":?"
    "[[:blank:]]*"
    "(([.]*[^.])*)([.]*)",
    llvm::Regex::Newline | llvm::Regex::IgnoreCase);

llvm::Regex strict_purpose(
    "^//@PURPOSE: [^[:blank:]].*[^.[:blank:]][.]\r*$",
    llvm::Regex::Newline);

void report::check_purpose(SourceRange range, llvm::StringRef comment)
{
    llvm::SmallVector<llvm::StringRef, 7> matches;

    size_t offset = 0;
    llvm::StringRef s;
    while (loose_purpose.match(s = comment.drop_front(offset), &matches)) {
        llvm::StringRef text = matches[0];
        std::pair<size_t, size_t> mm = mid_match(s, text);
        size_t matchpos = offset + mm.first;
        offset = matchpos + text.size();

        if (!strict_purpose.match(text)) {
            std::string expected =
                "//@PURPOSE: " + matches[1].trim().str() + ".";
            std::pair<size_t, size_t> mm = mid_mismatch(text, expected);
            a.report(range.getBegin().getLocWithOffset(matchpos + mm.first),
                     check_name, "PRP01",
                     "Invalid format for @PURPOSE line")
                << text;
            a.report(range.getBegin().getLocWithOffset(matchpos + mm.first),
                     check_name, "PRP01",
                     "Correct format is\n%0",
                     false, DiagnosticIDs::Note)
                << expected;
        }
    }
}

llvm::Regex classes(
    "^// *(@CLASSES: *)?" "("
                     "[[:alpha:]][[:alnum:]<_>]*"
               "(" "::[[:alpha:]][[:alnum:]<_>]*" ")*"
           ")" "( *: *[^:].*)?");

void report::check_description(SourceRange range, llvm::StringRef comment)
{
    size_t cpos = comment.find("//@CLASSES:");
    size_t end = comment.find("//\n", cpos);
    if (end == comment.npos) {
        end = comment.find("//\r\n", cpos);
        if (end == comment.npos) {
            end = comment.size();
        }
    }
    size_t dpos = comment.find("//@DESCRIPTION:", cpos);
    size_t t1 = comment.find("\n///", dpos);
    size_t t2 = comment.find("\n//@", dpos);
    llvm::StringRef desc = comment.slice(dpos, t1 < t2 ? t1 : t2);

    if (cpos == comment.npos) {
        return;                                                       // RETURN
    }

    if (a.get_source_line(range.getBegin().getLocWithOffset(cpos)).trim() !=
        "//@CLASSES:") {
        a.report(range.getBegin().getLocWithOffset(cpos + 11),
                 check_name, "CLS01",
                 "'//@CLASSES:' line should contain no other text "
                 "(classes go on subsequent lines, one per line)");
    }
    else {
        cpos = comment.find('\n', cpos) + 1;
    }

    while (cpos < end) {
        llvm::SmallVector<llvm::StringRef, 7> matches;
        if (!classes.match(comment.slice(cpos, end), &matches)) {
            a.report(range.getBegin().getLocWithOffset(cpos),
                     check_name, "CLS03",
                     "Badly formatted class line; should be "
                     "'//  class: description'");
        } else {
            cpos += comment.slice(cpos, end).find(matches[2]) +
                    matches[2].size();
            if (matches[4].empty()) {
                a.report(range.getBegin().getLocWithOffset(cpos),
                         check_name, "CLS02",
                         "Class name must be followed by ': description'");
            }
            std::string qc = ("'" + matches[2] + "'").str();
            if (dpos != comment.npos && desc.find(qc) == desc.npos) {
                a.report(range.getBegin().getLocWithOffset(dpos),
                         check_name, "DC01",
                         "Description should contain single-quoted "
                         "class name %0")
                    << qc;
            }
        }
        cpos = comment.find('\n', cpos);
        if (cpos != comment.npos) {
            ++cpos;
        }
    }
}

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
    // Hook up the callback functions.
{
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
