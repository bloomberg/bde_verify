// csafmt_comments.cpp                                                -*-C++-*-

#if SPELL_CHECK

#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_util.h>
#include <llvm/Support/Regex.h>
#include <string>
#include <aspell.h>

#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("spell-check");

// ----------------------------------------------------------------------------

using namespace clang;
using namespace bde_verify::csabase;

namespace
{

struct comments
    // Data holding seen comments.
{
    typedef std::vector<SourceRange> Ranges;
    typedef std::map<std::string, Ranges> Comments;
    Comments d_comments;

    void append(Analyser& analyser, SourceRange range);
};

void comments::append(Analyser& analyser, SourceRange range)
{
    SourceManager& m = analyser.manager();
    comments::Ranges& c = d_comments[m.getFilename(range.getBegin())];
    if (c.size() != 0 &&
        bde_verify::csabase::areConsecutive(m, c.back(), range)) {
        c.back().setEnd(range.getEnd());
    } else {
        c.push_back(range);
    }
}

struct files
    // Callback object for inspecting files.
{
    Analyser& d_analyser;                   // Analyser object.

    files(Analyser& analyser);
        // Create a 'files' object, accessing the specified 'analyser'.

    void operator()(SourceRange range);
        // The specified comment 'range' is added to the stored data.

    void operator()();
        // Inspect all comments.

    void check_spelling(AspellSpeller *spell_checker, SourceRange range);
        // Spell check the comment.

    void break_for_spelling(std::vector<SourceRange>* words,
                            SourceRange               range);
        // Break the comment at the specified 'range' into words suitable for
        // spell checking and load them into the specified 'words' vector.

    typedef std::map<std::string, std::vector<SourceRange> > Errors;
    Errors d_errors;
};

files::files(Analyser& analyser)
: d_analyser(analyser)
{
}

void files::operator()(SourceRange range)
{
    d_analyser.attachment<comments>().append(d_analyser, range);
}

void files::operator()()
{
    AspellConfig *spell_config = new_aspell_config();
    aspell_config_replace(spell_config, "lang", "en_US");
    aspell_config_replace(spell_config, "size", "90");
    aspell_config_replace(spell_config, "ignore-case", "true");
    aspell_config_replace(spell_config, "add-extra-dicts", "en_CA");
    aspell_config_replace(spell_config, "add-extra-dicts", "en_GB");
    AspellCanHaveError *possible_err = new_aspell_speller(spell_config);
    AspellSpeller *spell_checker = 0;
    if (aspell_error_number(possible_err) != 0) {
        clang::SourceManager& m = d_analyser.manager();
        d_analyser.report(m.getLocForStartOfFile(m.getMainFileID()),
                          check_name, "SP02",
                          "Cannot start spell checker")
            << aspell_error_message(possible_err);
            return;                                                   // RETURN
    }
    spell_checker = to_aspell_speller(possible_err);
    llvm::SmallVector<llvm::StringRef, 10> good_words;
    llvm::StringRef(d_analyser.config()->value("dictionary")).
        split(good_words, " ", -1, false);
    for (size_t i = 0; i < good_words.size(); ++i) {
        aspell_speller_add_to_session(
            spell_checker, good_words[i].data(), good_words[i].size());
    }

    comments::Comments& file_comments =
        d_analyser.attachment<comments>().d_comments;

    for (comments::Comments::iterator file_begin = file_comments.begin(),
                                      file_end   = file_comments.end(),
                                      file_itr   = file_begin;
         file_itr != file_end;
         ++file_itr) {
        const std::string &file_name = file_itr->first;
        if (!d_analyser.is_component(file_name)) {
            continue;
        }
        comments::Ranges& comments = file_itr->second;
        for (comments::Ranges::iterator comments_begin = comments.begin(),
                                        comments_end   = comments.end(),
                                        comments_itr   = comments_begin;
             comments_itr != comments_end;
             ++comments_itr) {
            check_spelling(spell_checker, *comments_itr);
        }
    }

    size_t limit = std::strtoul(
        d_analyser.config()->value("spelled_ok_count").c_str(),
        0, 10);

    Errors::const_iterator b = d_errors.begin();
    Errors::const_iterator e = d_errors.end();
    for (; b != e; ++b) {
        const std::vector<SourceRange>& locs = b->second;
        if (limit == 0 || locs.size() < limit) {
            for (size_t j = 0; j < locs.size(); ++j) {
                d_analyser.report(locs[j].getBegin(), check_name, "SP01",
                                  "Misspelled word '%0'")
                    << b->first
                    << locs[j];
            }
        }
    }

    delete_aspell_config(spell_config);
}

void
files::break_for_spelling(std::vector<SourceRange>* words, SourceRange range)
{
    llvm::StringRef comment = d_analyser.get_source(range, true);
    words->clear();
    bool in_single_quotes = false;
    bool in_double_quotes = false;
    bool in_block = false;
    size_t start_of_last_block = 0;
    static llvm::Regex code("^[[:blank:]]*//[.][.]$", llvm::Regex::Newline);
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
            if (!in_double_quotes) {
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
                comment.substr(i).startswith("//@AUTHOR:")) {
                size_t j = comment.substr(i).find("\n//\n");
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
                        if (!std::isalpha(c)) {
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
                        words->push_back(SourceRange(
                            range.getBegin().getLocWithOffset(
                                start_of_last_block),
                            range.getBegin().getLocWithOffset(last)));
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

void files::check_spelling(AspellSpeller *spell_checker, SourceRange comment)
{
    std::vector<SourceRange> words;
    break_for_spelling(&words, comment);
    for (size_t i = 0; i < words.size(); ++i) {
        llvm::StringRef word = d_analyser.get_source(words[i], true);
        if (!aspell_speller_check(
                 spell_checker, word.data(), word.size())) {
            d_errors[word.lower()].push_back(words[i]);
        }
    }
}

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
    // Hook up the callback functions.
{
    analyser.onTranslationUnitDone += files(analyser);
    observer.onComment             += files(analyser);
}

}  // close anonymous namespace

static bde_verify::csabase::RegisterCheck c1(check_name, &subscribe);

#endif  // SPELL_CHECK
