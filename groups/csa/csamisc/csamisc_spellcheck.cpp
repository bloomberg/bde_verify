// csamisc_spellcheck.cpp                                             -*-C++-*-

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchersInternal.h>
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
#include <ctype.h>
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/VariadicFunction.h>
#include <llvm/Support/Regex.h>
#include <stddef.h>
#include <stdlib.h>
#include <utils/array.hpp>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace csabase { class Visitor; }

using namespace clang;
using namespace csabase;
using namespace clang::ast_matchers;

// ----------------------------------------------------------------------------

static std::string const check_name("spell-check");

// ----------------------------------------------------------------------------

#if SPELL_CHECK

#include <aspell.h>

namespace
{

struct data
    // Data holding seen comments.
{
    typedef std::vector<SourceRange> Ranges;
    typedef std::map<std::string, Ranges> Comments;
    Comments d_comments;

    typedef std::map<std::string, std::set<Location>> BadParms;
    BadParms d_bad_parms;

    void append(Analyser& analyser, SourceRange range);
};

void data::append(Analyser& analyser, SourceRange range)
{
    SourceManager& m = analyser.manager();
    data::Ranges& c = d_comments[m.getFilename(range.getBegin())];
    if (c.size() != 0 && areConsecutive(m, c.back(), range)) {
        c.back().setEnd(range.getEnd());
    } else {
        c.push_back(range);
    }
}

struct report : Report<data>
    // Callback object for inspecting files.
{
    INHERIT_REPORT_CTOR(report, Report, data);

    void operator()(SourceRange range);
        // The specified comment 'range' is added to the stored data.

    void operator()();
        // Inspect all comments.

    void check_spelling(SourceRange range);
        // Spell check the comment.

    void break_for_spelling(std::vector<SourceRange>* words,
                            SourceRange               range);
        // Break the comment at the specified 'range' into words suitable for
        // spell checking and load them into the specified 'words' vector.

    void check_parameters();
        // Check that function parameters consist of real words.

    void match_parameter(const BoundNodes &nodes);
        // Callback for named function parameters.

    typedef std::map<std::string, std::vector<SourceRange> > Errors;
    Errors d_errors;

    AspellSpeller *spell_checker;
};

void report::operator()(SourceRange range)
{
    d.append(a, range);
}

void report::operator()()
{
    AspellConfig *spell_config = new_aspell_config();
    aspell_config_replace(spell_config, "lang", "en_US");
    aspell_config_replace(spell_config, "size", "90");
    aspell_config_replace(spell_config, "ignore-case", "true");
    aspell_config_replace(spell_config, "add-extra-dicts", "en_CA");
    aspell_config_replace(spell_config, "add-extra-dicts", "en_GB");
    aspell_config_replace(spell_config, "guess", "true");
    aspell_config_replace(spell_config, "run-together", "true");
    AspellCanHaveError *possible_err = new_aspell_speller(spell_config);
    if (aspell_error_number(possible_err) != 0) {
        auto report = a.report(m.getLocForStartOfFile(m.getMainFileID()), check_name, "SP02",
                 "Cannot start spell checker: %0");
        report << aspell_error_message(possible_err);
        return;                                                         // RETURN
    }
    spell_checker = to_aspell_speller(possible_err);
    std::vector<std::string> good_words;
    a.config()->appendGoodWords(good_words);
    for (const auto& s : good_words) {
        aspell_speller_add_to_session(spell_checker, s.data(), s.size());
    }

    for (const auto& file_comment : d.d_comments) {
        if (a.is_component(file_comment.first)) {
            for (const auto& comment : file_comment.second) {
                check_spelling(comment);
            }
        }
    }

    size_t limit =
        std::strtoul(a.config()->value("spelled_ok_count").c_str(), 0, 10);

    Errors::const_iterator b = d_errors.begin();
    Errors::const_iterator e = d_errors.end();
    for (; b != e; ++b) {
        const std::vector<SourceRange>& locs = b->second;
        if (limit == 0 || locs.size() < limit) {
            for (size_t j = 0; j < locs.size(); ++j) {
                auto report = a.report(locs[j].getBegin(), check_name, "SP01",
                         "Misspelled word '%0'");
                report << b->first << locs[j];
            }
        }
    }

    check_parameters();

    delete_aspell_config(spell_config);
}

void
report::break_for_spelling(std::vector<SourceRange>* words, SourceRange range)
{
    llvm::StringRef comment = a.get_source(range, true);
    words->clear();
    if (comment.startswith("// close namespace ")) {
        return;
    }
    bool in_single_quotes = false;
    bool in_double_quotes = false;
    bool in_block = false;
    size_t start_of_last_block = 0;
    static llvm::Regex code("^[[:blank:]]*//[.][.]\r*$", llvm::Regex::Newline);
    llvm::SmallVector<llvm::StringRef, 7> matches;
    size_t code_pos = comment.size() + 1;
    // If the comment has a "//.." line, note its end position.
    if (code.match(comment, &matches)) {
        llvm::StringRef m = matches[0];
        code_pos = comment.find(m) + m.size() - 1;
    }
    static const llvm::StringRef punct("!:;,.?");
    static const std::vector<llvm::StringRef> empty;
    for (size_t i = 0; i <= comment.size(); ++i) {
        if (i == code_pos) {
            llvm::StringRef tail = comment.drop_front(i);
            // At a "//.." line, go to the next one.
            if (code.match(tail, &matches)) {
                llvm::StringRef m = matches[0];
                i += tail.find(m) + m.size() - 1;
                tail = comment.drop_front(i);
            }
            // If the contract has another "//.." line, note its end position.
            if (code.match(tail, &matches)) {
                llvm::StringRef m = matches[0];
                code_pos = i + tail.find(m) + m.size() - 1;
            } else {
                code_pos = comment.size() + 1;
            }
        }

        unsigned char c =
            i == comment.size() ? ' ' : static_cast<unsigned char>(comment[i]);
        if (c == '\'') {
            if (in_block &&
                !in_single_quotes &&
                i > 0 &&
                std::isalpha(static_cast<unsigned char>(comment[i - 1]))) {
                // This is an apostrophe, not a single quote.
            } else if (!in_double_quotes) {
                in_single_quotes = !in_single_quotes;
                if (!in_block) {
                    in_block = true;
                    start_of_last_block = i;
                }
            }
        } else if (c == '"') {
            if (!in_single_quotes) {
                in_double_quotes = !in_double_quotes;
                if (!in_block) {
                    in_block = true;
                    start_of_last_block = i;
                }
            }
        } else if (!in_single_quotes && !in_double_quotes) {
            if (c == '/' &&
                (i == 0 || comment[i - 1] == '\n') &&
                (comment.substr(i).startswith("//@AUTHOR:") ||
                 comment.substr(i).startswith("//@CONTACT:"))) {
                size_t j = comment.substr(i).find("\n//\n");
                if (j == comment.npos) {
                    j = comment.substr(i).find("\n//\r\n");
                }
                if (j != comment.npos) {
                    i += j;
                }
            } else if (std::isspace(c)) {
                if (in_block) {
                    in_block = false;
                    bool saw_lower = false;
                    size_t last = i;
                    bool found = true;
                    for (size_t j = start_of_last_block; found && j < last;
                         ++j) {
                        c = static_cast<unsigned char>(comment[j]);
                        if (!std::isalpha(c) &&
                            (c != '\'' || j == start_of_last_block)) {
                            if (j != i - 1) {
                                found = false;
                            } else if (punct.find(c) != punct.npos) {
                                --last;
                            } else {
                                found = false;
                            }
                        } else if (std::islower(c)) {
                            saw_lower = true;
                        } else if (std::isupper(c) && saw_lower) {
                            found = false;
                        }
                    }
                    if (found) {
                        while (last > start_of_last_block &&
                               comment[last - 1] == '\'') {
                            --last;
                        }
                        words->push_back(
                            getOffsetRange(range,
                                           start_of_last_block,
                                           last - start_of_last_block));
                    }
                    start_of_last_block = 0;
                }
            } else {
                if (!in_block) {
                    in_block = true;
                    start_of_last_block = i;
                }
            }
        }
    }
}

void report::check_spelling(SourceRange comment)
{
    std::vector<SourceRange> words;
    break_for_spelling(&words, comment);
    for (size_t i = 0; i < words.size(); ++i) {
        llvm::StringRef word = a.get_source(words[i], true);
        if (!aspell_speller_check(spell_checker, word.data(), word.size())) {
            d_errors[word.lower()].push_back(words[i]);
        }
    }
}

internal::DynTypedMatcher parameter_matcher()
    // Return an AST matcher which looks for named parameters.
{
    return decl(forEachDescendant(parmVarDecl(matchesName(".")).bind("parm")));
}

void report::match_parameter(const BoundNodes &nodes)
{
    static std::set<llvm::StringRef> ok{
        "arg",
        "argc", // main argument
        "argv", // main argument
        "cb",   // callback
        "dst",  // destination
        "fn",
        "id",   // identifier (not identity)
        "init", // initializ(e,er)
        "iter", // iterator
        "max",  // maximum
        "min",  // minimum
        "msg",  // message
        "num",  // number
        "pos",  // position
        "ptr",  // pointer
        "ref",  // reference
        "src",  // source
        "tmp",  // temporary
    };

    const ParmVarDecl *parm = nodes.getNodeAs<ParmVarDecl>("parm");
    if (a.is_component(parm)) {
        llvm::StringRef name = parm->getName();
        static llvm::Regex words("[[:digit:]_]*([[:alpha:]][[:lower:]]*)");
        llvm::SmallVector<llvm::StringRef, 7> matches;
        size_t pos = 0;
        while (words.match(name.substr(pos), &matches)) {
            pos = name.find(matches[1], pos);
            std::string word = matches[1].lower();
            if (!ok.count(word) &&
                !aspell_speller_check(
                    spell_checker, word.data(), word.size())) {
                llvm::SmallVector<llvm::StringRef, 100> var_abbrs;
                llvm::StringRef(a.config()->value("variable_abbreviations",
                                                  parm->getLocation()))
                    .split(var_abbrs, " ", -1, false);
                std::set<llvm::StringRef> vars{
                    var_abbrs.begin(), var_abbrs.end()};
                if (!vars.count(word)) {
                    d.d_bad_parms[word].insert(Location(
                        m, parm->getLocation().getLocWithOffset(pos)));
                }
            }
            pos += word.size();
        }
    }
}

void report::check_parameters()
{
    MatchFinder mf;
    OnMatch<report, &report::match_parameter> m1(this);
    mf.addDynamicMatcher(parameter_matcher(), &m1);
    mf.match(*a.context()->getTranslationUnitDecl(), *a.context());
    size_t limit =
        std::strtoul(a.config()->value("spelled_ok_count").c_str(), 0, 10);
    for (const auto& p : d.d_bad_parms) {
        if (limit == 0 || p.second.size() < limit) {
            for (const auto& l : p.second) {
                auto report = a.report(l.location(), check_name, "SP03",
                         "Parameter name contains misspelled word '%0'");
                report << p.first;
            }
        }
    }
}

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
    // Hook up the callback functions.
{
    analyser.onTranslationUnitDone += report(analyser);
    observer.onComment             += report(analyser);
}

}  // close anonymous namespace

#else   // SPELL_CHECK

void subscribe(Analyser&, Visitor&, PPObserver&)
    // Do nothing
{
}

#endif  // SPELL_CHECK

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
