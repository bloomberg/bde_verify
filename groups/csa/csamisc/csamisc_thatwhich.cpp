// csafmt_thatwhich.cpp                                               -*-C++-*-

#include <llvm/ADT/StringExtras.h>
#include <clang/AST/Decl.h>
#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_util.h>
#include <cctype>
#include <set>
#include <string>
#include <vector>

using namespace clang;
using namespace csabase;

static std::string const check_name("that-which");

namespace
{

struct Word
{
    llvm::StringRef word;
    size_t          offset;
    bool            is_comma         : 1;
    bool            is_em_dash       : 1;
    bool            is_period        : 1;
    bool            is_question_mark : 1;
    bool            is_semicolon     : 1;
    bool            is_copyright     : 1;
    bool            is_preposition   : 1;
    bool            is_that          : 1;
    bool            is_which         : 1;
    bool            is_of            : 1;
    bool            is_punct         : 1;

    Word();

    void set(llvm::StringRef s, size_t position);
};

Word::Word()
: word()
, offset(llvm::StringRef::npos)
, is_comma(false)
, is_em_dash(false)
, is_period(false)
, is_question_mark(false)
, is_semicolon(false)
, is_copyright(false)
, is_preposition(false)
, is_that(false)
, is_which(false)
, is_of(false)
, is_punct(false)
{
}

void Word::set(llvm::StringRef s, size_t position)
{
    static std::set<llvm::StringRef> prepositions{
        "about",   "above",  "across",  "after",   "against",    "among",
        "and",     "around", "at",      "before",  "behind",     "below",
        "beneath", "beside", "besides", "between", "beyond",     "but",
        "by",      "during", "for",     "from",    "in",         "inside",
        "into",    "near",   "of",      "on",      "or",         "out",
        "outside", "over",   "since",   "through", "throughout", "till",
        "to",      "toward", "unclear", "under",   "until",      "up",
        "upon",    "with",   "within",  "without",
    };

    word             = s;
    offset           = position;
    is_comma         = s.equals(",");
    is_em_dash       = s.equals("-");
    is_period        = s.equals(".");
    is_question_mark = s.equals("?");
    is_semicolon     = s.equals(";");
    is_copyright     = s.equals_insensitive("copyright");
    is_that          = s.equals_insensitive("that");
    is_which         = s.equals_insensitive("which");
    is_preposition   = prepositions.count(s) || s.endswith("ing");
    is_of            = s.equals_insensitive("of");
    is_punct         = s.size() == 1 && !std::isalpha(s[0] & 0xFF);
}

struct data
{
    std::vector<SourceRange> comments;
};

struct report : Report<data>
{
    INHERIT_REPORT_CTOR(report, Report, data);

    void operator()();
    void operator()(SourceRange range);

    void that_which(SourceRange range);
    void split(std::vector<Word> *words, llvm::StringRef comment);
};

void report::split(std::vector<Word> *words, llvm::StringRef comment)
{
    words->clear();
    bool in_single_quotes = false;
    bool last_char_was_backslash = false;
    bool in_word = false;
    size_t start_of_last_word = 0;
    bool in_code = false;

    for (size_t i = 0; i < comment.size(); ++i) {
        if (i > 0 && comment[i - 1] == '\n') {
            while (comment[i] == ' ' && i < comment.size()) {
                ++i;
            }
        }
        llvm::StringRef sub = comment.substr(i);
        if (sub.startswith("//..\n") || sub.startswith("//..\r\n")) {
            i += 4 - 1;
            if (in_code) {
                words->push_back(Word());
                words->back().set(",", i);
                in_code = false;
            } else {
                in_code = true;
            }
            continue;
        }
        if (in_code) {
            continue;
        }
        if (sub.startswith("//")) {
            i += 2 - 1;
            continue;
        }
        unsigned char c = static_cast<unsigned char>(comment[i]);
        bool is_id = std::isalnum(c) || c == '_' || c == '-';
        if (in_word) {
            if (!is_id) {
                words->back().set(comment.slice(start_of_last_word, i),
                                  start_of_last_word);
            }
        } else if (is_id) {
            start_of_last_word = i;
            words->push_back(Word());
            words->back().set(comment.substr(start_of_last_word),
                              start_of_last_word);
        }
        if (!is_id) {
            last_char_was_backslash = c == '\\' && !last_char_was_backslash;
            if (c == '\'') {
                if (in_word) {
                    if (in_single_quotes) {
                        in_single_quotes = false;
                    } else {
                        is_id = true;
                    }
                } else if (!last_char_was_backslash) {
                    in_single_quotes = !in_single_quotes;
                }
            }
        }
        in_word = is_id;
        if (!is_id) {
            if (!std::isspace(c)) {
                words->push_back(Word());
                words->back().set(comment.slice(i, i + 1), i);
            }
        }
    }
}

void report::that_which(SourceRange range)
{
    llvm::StringRef c = a.get_source(range);
    std::vector<Word> w;
    split(&w, c);

    for (size_t i = 0; i + 1 < w.size(); ++i) {
        if (w[i].is_copyright) {
            break;
        }
        if (i == 0) {
            continue;
        }
        if (w[i].is_which &&
            !w[i - 1].is_punct &&
            !w[i - 1].is_comma &&
            !w[i - 1].is_that &&
            !w[i - 1].is_preposition &&
            !w[i + 1].is_of) {
            a.report(range.getBegin().getLocWithOffset(w[i].offset),
                     check_name, "TW01",
                     "Possibly prefer 'that' over 'which'");
        }
#if 0  // We haven't found a good ", that" rule yet.
        size_t np = 0;
        size_t nd = 0;
        if (w[i].is_that && w[i - 1].is_comma) {
            size_t prev_comma = i;
            for (size_t j = 0; j < i - 1; ++j) {
                if (w[j].is_comma) {
                    prev_comma = j;
                    np = 0;
                    nd = 0;
                } else if (w[j].is_period ||
                           w[j].is_semicolon ||
                           w[j].is_question_mark) {
                    ++np;
                } else if (w[j].is_em_dash) {
                    ++nd;
                }
            }
            if (prev_comma == i || np || nd & 1) {
                a.report(range.getBegin().getLocWithOffset(w[i].offset),
                         check_name, "TW02",
                         "Possibly incorrect comma before 'that'");
            }
        }
#endif
    }
}

void report::operator()(SourceRange range)
{
    if (a.is_component(range.getBegin())) {
        if (d.comments.size() && areConsecutive(m, d.comments.back(), range)) {
            d.comments.back().setEnd(range.getEnd());
        } else {
            d.comments.push_back(range);
        }
    }
}

void report::operator()()
{
    for (const auto& r : d.comments) {
        that_which(r);
    }
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    analyser.onTranslationUnitDone += report(analyser);
    observer.onComment             += report(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static RegisterCheck c1(check_name, &subscribe);

// ----------------------------------------------------------------------------
// Copyright (C) 2015 Bloomberg Finance L.P.
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
