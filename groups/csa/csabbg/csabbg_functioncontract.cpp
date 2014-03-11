// csabbg_functioncontract.cpp                                        -*-C++-*-
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_util.h>
#include <llvm/Support/Regex.h>
#include <clang/AST/Comment.h>
#include <clang/Frontend/CompilerInstance.h>
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <ctype.h>

#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("function-contract");

// ----------------------------------------------------------------------------

using clang::CXXConstructorDecl;
using clang::CXXMethodDecl;
using clang::CompoundStmt;
using clang::ConstStmtRange;
using clang::FunctionDecl;
using clang::FunctionTemplateDecl;
using clang::ParmVarDecl;
using clang::ReturnStmt;
using clang::SourceLocation;
using clang::SourceManager;
using clang::SourceRange;
using clang::Stmt;
using cool::csabase::Analyser;
using cool::csabase::Location;
using cool::csabase::PPObserver;
using cool::csabase::Range;
using cool::csabase::Visitor;

namespace
{

struct data
    // Data attached to analyzer for this check.
{
    typedef std::vector<SourceRange> Ranges;
    typedef std::map<std::string, Ranges> Comments;
    Comments d_comments;  // Comment blocks per file.

    typedef std::map<const FunctionDecl*, SourceRange> FunDecls;
    FunDecls d_fundecls;  // FunDecl, comment
};

struct fmap
{
    typedef std::multimap<unsigned, data::FunDecls::iterator> FMap;
    FMap fmap_;

    enum Status { e_Empty, e_Found, e_NotFound };

    Status find_contract(unsigned line) const;
        // Return the status of finding a function contract at the specified
        // 'line'.  The status is 'e_Empty' if there are no functions there,
        // 'e_Found' if a function has a contract, and 'e_NotFound' if none do.

    void insert(unsigned from, unsigned to, data::FunDecls::iterator itr);
        // Insert the specified 'itr' into the function map for all keys
        // inclusively between the specified 'from' and 'to'.
};

fmap::Status fmap::find_contract(unsigned line) const
{
    std::pair<FMap::const_iterator, FMap::const_iterator> itrs =
        fmap_.equal_range(line);
    if (itrs.first == itrs.second) {
        return e_Empty;                                               // RETURN
    }
    while (itrs.first != itrs.second) {
        if (itrs.first++->second->second.isValid()) {
            return e_Found;                                           // RETURN
        }
    }
    return e_NotFound;
}

void fmap::insert(unsigned from, unsigned to, data::FunDecls::iterator itr)
{
    while (from <= to) {
        fmap_.insert(std::make_pair(from++, itr));
    }
}

struct comments
    // Callback object for inspecting comments.
{
    Analyser& d_analyser;         // Analyser object.
    SourceManager& d_manager;     // SourceManager within Analyser.
    data::Comments& d_comments;   // Analyser's comment data.

    comments(Analyser& analyser);
        // Create a 'comments' object, accessing the specified 'analyser'.

    bool areConsecutive(const SourceRange& r1, const SourceRange& r2) const;
        // Return true iff the specified 'r1' is immediately followed by the
        // specified 'r2', i.e., 'r2' begins in the same file in which 'r1'
        // ends, either on the same line or the line after.  Note that there
        // might be code (not comments) between the ranges, but this check is
        // good enough for the purpose of considering multi-line "//" comments
        // as single comment blocks.

    static bool isDirective(llvm::StringRef comment);
        // Return wehether the specified 'comment' is a "// = default/delete"
        // comment.

    void operator()(SourceRange range);
        // The specified 'range', representing a comment, is either appended to
        // the previous comment or added separately to the comments list.
};

comments::comments(Analyser& analyser)
: d_analyser(analyser)
, d_manager(analyser.manager())
, d_comments(analyser.attachment<data>().d_comments)
{
}

bool comments::isDirective(llvm::StringRef comment)
{
    // Look for a variety of directives in comments.
    size_t last_line_pos = comment.rfind('\n');
    if (last_line_pos != comment.npos) {
        comment = comment.substr(last_line_pos + 1);
    }
    static llvm::Regex re("^(//|/[*])" "[[:space:]]*"
                          "("
                             "=" "[[:space:]]*" "delete"  "|"
                        ".*" "=" "[[:space:]]*" "default" "|"
                             "DEPRECATED"                 "|"
                             "IMPLICIT"
                          ")"
                          "[;.[:space:]]*" "([*]/)?" "[[:space:]]*" "$",
                          llvm::Regex::IgnoreCase);
    return re.match(comment);
}

bool
comments::areConsecutive(const SourceRange& r1, const SourceRange& r2) const
{
    return cool::csabase::areConsecutive(d_manager, r1, r2) &&
           !comments::isDirective(d_analyser.get_source(r1)) &&
           !comments::isDirective(d_analyser.get_source(r2));
}

void comments::operator()(SourceRange range)
{
    Location location(d_analyser.get_location(range.getBegin()));
    if (d_analyser.is_component(location.file())) {
        data::Ranges& c = d_comments[location.file()];
        if (c.size() == 0 || !areConsecutive(c.back(), range)) {
            c.push_back(range);
        } else {
            c.back().setEnd(range.getEnd());
        }
    }
}

struct ParmInfo
{
    bool is_matched    : 1;
    bool is_quoted     : 1;
    bool is_not_quoted : 1;

    ParmInfo();
};

ParmInfo::ParmInfo()
: is_matched(false)
, is_quoted(false)
, is_not_quoted(false)
{
}

struct Word
{
    llvm::StringRef word;
    size_t offset;
    size_t parm;
    bool is_quoted     : 1;
    bool is_noise      : 1;
    bool is_specify    : 1;
    bool is_optionally : 1;

    Word();

    void set(std::vector<ParmInfo>* parm_info,
             llvm::StringRef s,
             size_t position,
             bool single_quoted,
             const std::vector<llvm::StringRef>& parms,
             const std::vector<llvm::StringRef>& noise);
};

Word::Word()
: word()
, offset(llvm::StringRef::npos)
, parm(~size_t(0))
, is_quoted(false)
, is_noise(false)
, is_specify(false)
, is_optionally(false)
{
}

void Word::set(std::vector<ParmInfo>* parm_info,
               llvm::StringRef s,
               size_t position,
               bool single_quoted,
               const std::vector<llvm::StringRef>& parms,
               const std::vector<llvm::StringRef>& noise)
{
    word = s;
    offset = position;
    is_quoted = single_quoted;
    is_specify = !single_quoted &&
                 (s.equals_lower("specify") || s.equals_lower("specified"));
    is_optionally = !single_quoted && s.equals_lower("optionally");

    parm = parms.size();
    for (size_t i = 0; i < parms.size(); ++i) {
        llvm::StringRef p = parms[i];
        if (   p == s
            || (   p.size() > 0
                && s.size() > 0
                && std::toupper(static_cast<unsigned char>(p[0])) ==
                                static_cast<unsigned char>(s[0])
                && p.substr(1) == s.substr(1))) {
            parm = i;
            break;
        }
    }

    is_noise = false;
    for (size_t i = 0; !is_noise && i < noise.size(); ++i) {
        is_noise = noise[i].equals_lower(s);
    }

    if (parm < parm_info->size()) {
        ParmInfo& pi = (*parm_info)[parm];
        pi.is_matched = true;
        if (is_quoted) {
            pi.is_quoted = true;
        } else {
            pi.is_not_quoted = true;
        }
    }
}

void break_into_words(std::vector<Word>* words,
                      std::vector<ParmInfo>* parm_info,
                      llvm::StringRef comment,
                      const std::vector<llvm::StringRef>& parms,
                      const std::vector<llvm::StringRef>& noise)
{
    words->clear();
    parm_info->clear();
    parm_info->resize(parms.size());
    bool in_single_quotes = false;
    bool last_char_was_backslash = false;
    bool in_word = false;
    size_t start_of_last_word = 0;
    static llvm::Regex code("^[[:blank:]]*//[.][.]$", llvm::Regex::Newline);
    llvm::SmallVector<llvm::StringRef, 7> matches;
    llvm::StringRef c = comment;
    size_t code_pos = c.size();
    // If the contract has a "//.." line, note its end position.
    if (code.match(c, &matches)) {
        llvm::StringRef m = matches[0];
        code_pos = c.find(m) + m.size() - 1;
    }
    for (size_t i = 0; i < comment.size(); ++i) {
        if (i == code_pos) {
            c = comment.drop_front(i);
            // At a "//.." line, go to the next one unless we're in quotes.
            if (!in_single_quotes && code.match(c, &matches)) {
                llvm::StringRef m = matches[0];
                i += c.find(m) + m.size() - 1;
                c = comment.drop_front(i);
            }
            // If the contract has another "//.." line, note its end position.
            if (code.match(c, &matches)) {
                llvm::StringRef m = matches[0];
                code_pos = i + c.find(m) + m.size() - 1;
            } else {
                code_pos = comment.size();
            }
        }

        unsigned char c = static_cast<unsigned char>(comment[i]);
        bool is_id = std::isalnum(c) || c == '_' || c == '-';
        if (in_word) {
            if (!is_id) {
                words->back().set(parm_info,
                                  comment.slice(start_of_last_word, i),
                                  start_of_last_word,
                                  in_single_quotes,
                                  parms,
                                  noise);
            }
        } else if (is_id) {
            start_of_last_word = i;
            words->push_back(Word());
            words->back().set(parm_info,
                              comment.substr(start_of_last_word),
                              start_of_last_word,
                              in_single_quotes,
                              parms,
                              noise);
        }
        if (!is_id) {
            if (c == '\\') {
                last_char_was_backslash = !last_char_was_backslash;
            } else if (c == '\'') {
                if (in_word) {
                    if (in_single_quotes) {
                        in_single_quotes = false;
                    }
                } else if (!last_char_was_backslash) {
                    in_single_quotes = !in_single_quotes;
                }
            }
        }
        in_word = is_id;
    }
}

void allFunDecls(Analyser& analyser, const FunctionDecl* func)
    // Callback function for inspecting function declarations.
{
    // Don't process compiler-defaulted methods, main, template instantiations,
    // or macro expansions
    if (   !func->isDefaulted()
        && !func->isMain()
        && !func->getLocation().isMacroID()
        && (   func->getTemplatedKind() == func->TK_NonTemplate
            || func->getTemplatedKind() == func->TK_FunctionTemplate)
            ) {
        analyser.attachment<data>().d_fundecls[func];
    }
}
 
void allTpltFunDecls(Analyser& analyser, const FunctionTemplateDecl* func)
    // Callback function for inspecting function template declarations.
{
    allFunDecls(analyser, func->getTemplatedDecl());
}

struct report
    // Callback object invoked upon completion.
{
    Analyser& d_analyser;       // Analyser object.
    SourceManager& d_manager;   // SourceManager within Analyser.
    data& d;                    // Analyser's data for this module.

    report(Analyser& analyser);
        // Create a 'report' object, accessing the specified 'analyser'.

    void operator()();
        // Invoked to process reports.

    void processAllFunDecls(data::FunDecls& decls);
        // Utility method to process function declarations from the specified
        // 'decls' container.

    bool isGenerated(const FunctionDecl *func);
        // Return true iff the specified 'func' appears between comments
        // "// {{{ BEGIN GENERATED CODE" and "// }}} END GENERATED CODE".

    bool doesNotNeedContract(const FunctionDecl *func);
        // Return 'true' iff the specified 'func' does not need a contract.
        //
        // Reasons:
        //: o Not the canonical declaration
        //: o Private copy constructor declaration.
        //: o Private assignment operator declaration.
        //: o Template method specialization.

    SourceRange getContract(const FunctionDecl *func,
                            data::Ranges::iterator comments_begin,
                            data::Ranges::iterator comments_end);
        // Return the 'SourceRange' of the function contract of the specified
        // 'func' if it is present in the specified range of 'comments_begin'
        // up to 'comments_end', and return an invalid 'SourceRange' otherwise.

    void critiqueContract(const FunctionDecl *func, SourceRange comment);
        // Issue diagnostics for deficiencies in the specified 'comment' with
        // respect to being a contract for the specified 'func'.

    bool hasCommentedCognate(const FunctionDecl *func, data::FunDecls& decls);
        // Return 'true' iff the specified function declaration 'decl' can be
        // satisfied by a function contract appearing on a declaration in the
        // specified 'decls' container.
};

report::report(Analyser& analyser)
: d_analyser(analyser)
, d_manager(analyser.manager())
, d(analyser.attachment<data>())
{
}

void report::operator()()
{
    processAllFunDecls(d.d_fundecls);
}

bool report::hasCommentedCognate(const clang::FunctionDecl *func,
                                 data::FunDecls& decls)
{
    const clang::DeclContext *parent = func->getLookupParent();
    std::string name = func->getNameAsString();
    fmap fm;

    clang::DeclContext::decl_iterator declsb = parent->decls_begin();
    clang::DeclContext::decl_iterator declse = parent->decls_end();
    while (declsb != declse) {
        const clang::Decl *decl = *declsb++;
        const clang::FunctionDecl* cfunc =
            llvm::dyn_cast<clang::FunctionDecl>(decl);
        const clang::FunctionTemplateDecl* ctplt =
            llvm::dyn_cast<clang::FunctionTemplateDecl>(decl);
        if (ctplt) {
            cfunc = ctplt->getTemplatedDecl();
        }
        data::FunDecls::iterator itr = decls.find(cfunc);
        if (itr != decls.end()) {
            if (itr->second.isValid() && cfunc->getNameAsString() == name) {
                // Functions in the same scope with the same name are cognates.
                // (This is, perhaps, simplistic.)
                return true;                                          // RETURN
            }
            if (cfunc != func) {
                fm.insert(d_manager.getPresumedLineNumber(cfunc->getLocStart()),
                          d_manager.getPresumedLineNumber(cfunc->getLocEnd()),
                          itr);
            }
        }
    }

    // A consecutive set of function declarations with nothing else intervening
    // are cognates.
    unsigned el = d_manager.getPresumedLineNumber(func->getLocEnd());
    while (fmap::Status status = fm.find_contract(++el)) {
        if (status == fmap::e_Found) {
            return true;                                              // RETURN
        }
    }

    unsigned bl = d_manager.getPresumedLineNumber(func->getLocStart());
    while (fmap::Status status = fm.find_contract(--bl)) {
        if (status == fmap::e_Found) {
            return true;                                              // RETURN
        }
    }

    return false;
}

void report::processAllFunDecls(data::FunDecls& decls)
{
    for (data::FunDecls::iterator it = decls.begin(); it != decls.end(); ++it) {
        Location location(d_analyser.get_location(it->first->getLocStart()));
        data::Ranges& c = d.d_comments[location.file()];
        it->second = getContract(it->first, c.begin(), c.end());
    }

    for (data::FunDecls::iterator it = decls.begin(); it != decls.end(); ++it) {
        if (doesNotNeedContract(it->first)) {
        }
        else if (it->second.isValid()) {
            critiqueContract(it->first, it->second);
        }
        else if (!hasCommentedCognate(it->first, decls)) {
            d_analyser.report(it->first->getNameInfo().getLoc(),
                              check_name, "FD01",
                              "Function declaration requires contract")
                << it->first->getNameInfo().getSourceRange();
        }
    }
}

bool report::isGenerated(const FunctionDecl *func)
{
    // Note that it is necessary to scan the file buffer rather than using the
    // comments callback from the preprocessor because the generator brackets
    // its generated code awkwardly within '#if/#else' constructs, and thus
    // some of the brackets never show up:
    //..
    //  #if !BSLS_COMPILERFEATURES_SIMULATE_CPP11_FEATURES
    //  #elif BSLS_COMPILERFEATURES_SIMULATE_VARIADIC_TEMPLATES
    //  // {{{ BEGIN GENERATED CODE
    //  #else
    //  // }}} END GENERATED CODE
    //  #endif
    //..

    SourceManager&  m    = d_manager;
    clang::FileID   file = m.getFileID(func->getLocStart());
    llvm::StringRef buf  = m.getBufferData(file);
    unsigned        pos  = m.getFileOffset(func->getLocStart());

    const char *const bg = "\n// {{{ BEGIN GENERATED CODE";
    const char *const eg = "\n// }}} END GENERATED CODE";

    unsigned bpos = buf.npos;
    for (unsigned p = 0; (p = buf.find(bg, p)) < pos; ++p) {
        bpos = p;
    }
    unsigned epos = buf.find(eg, bpos);
    return bpos != buf.npos && (epos == buf.npos || bpos < epos);
}

bool report::doesNotNeedContract(const FunctionDecl *func)
{
    const CXXConstructorDecl *ctor;
    const CXXMethodDecl *meth;

    return func != func->getCanonicalDecl()
        || (   func->getAccess() == clang::AS_private
            && !func->hasBody()
            && (   (   (ctor = llvm::dyn_cast<CXXConstructorDecl>(func))
                    && ctor->isCopyConstructor())
                || (   (meth = llvm::dyn_cast<CXXMethodDecl>(func))
                    && meth->isCopyAssignmentOperator())))
        || (   d_analyser.is_test_driver()
            && func->getNameAsString() == "aSsErT")
        || isGenerated(func);
}

SourceRange report::getContract(const FunctionDecl     *func,
                                data::Ranges::iterator  comments_begin,
                                data::Ranges::iterator  comments_end)
{
    SourceRange declarator = func->getSourceRange();
    declarator.setEnd(declarator.getEnd().getLocWithOffset(1));
    SourceRange contract;

    if (func->doesThisDeclarationHaveABody() && func->getBody()) {
        // Function with body - look for a comment that starts no earlier than
        // the function declarator and has only whitespace between itself and 
        // the open brace of the function.
        SourceLocation bodyloc = func->getBody()->getLocStart();
        data::Ranges::iterator it;
        for (it = comments_begin; it != comments_end; ++it) {
            if (d_manager.isBeforeInTranslationUnit(bodyloc, it->getBegin())) {
                break;
            }
            if (d_manager.isBeforeInTranslationUnit(
                    it->getEnd(), declarator.getBegin())) {
                continue;
            }
            llvm::StringRef s = d_analyser.get_source(
                SourceRange(it->getEnd(), bodyloc), true);
            if (s.find_first_not_of(" \n") == llvm::StringRef::npos) {
                contract = *it;
                break;
            }
        }
    } else {
        // Function without body - look for a comment following the declaration
        // separated from it by only whitespace and semicolon.
        SourceLocation endloc = declarator.getEnd();
        data::Ranges::iterator it;
        for (it = comments_begin; it != comments_end; ++it) {
            if (d_manager.isBeforeInTranslationUnit(it->getEnd(), endloc)) {
                continue;
            }
            llvm::StringRef s = d_analyser.get_source(
                SourceRange(endloc, it->getBegin()), true);
            if (s.find_first_not_of("; \n") == llvm::StringRef::npos) {
                contract = *it;
            }
            break;
        }
    }
    return contract;
}

bool are_numeric_cognates(llvm::StringRef a, llvm::StringRef b)
{
    llvm::StringRef digits = "0123456789";
    size_t ai = 0;
    size_t bi = 0;

    while (ai < a.size() && bi < b.size()) {
        size_t adi = a.find_first_of(digits, ai);
        size_t bdi = b.find_first_of(digits, bi);
        if (a.slice(ai, adi) != b.slice(bi, bdi)) {
            break;
        }
        ai = a.find_first_not_of(digits, adi);
        bi = b.find_first_not_of(digits, bdi);
    }
    return ai == a.npos && bi == b.npos;
}

SourceRange word_range(SourceRange context, const Word& word)
{
    SourceLocation begin = context.getBegin().getLocWithOffset(word.offset);
    return SourceRange(begin, begin.getLocWithOffset(word.word.size() - 1));
}

void report::critiqueContract(const FunctionDecl* func, SourceRange comment)
{
    llvm::StringRef contract = d_analyser.get_source(comment);

    // Ignore "= default" and "= delete" comments and deprecated functions.
    if (comments::isDirective(contract)) {
        return;                                                       // RETURN
    }

    const SourceLocation cloc = comment.getBegin();

    // Check for bad indentation.
    const int fcolm = d_manager.getPresumedColumnNumber(func->getLocStart());
    const int ccolm = d_manager.getPresumedColumnNumber(cloc);
    if (ccolm != fcolm + 4) {
        d_analyser.report(cloc, check_name, "FD02",
            "Function contracts should be indented 4, not %0, spaces "
            "from their function declaration")
            << (ccolm - fcolm)
            << comment;
    }

    // Now check that the function contract documents the parameters.
    const unsigned num_parms = func->getNumParams();
    static const char *const noise_words[] = {
        "a", "an", "and", "are", "is", "not", "or", "the"
    };
    static const std::vector<llvm::StringRef> noise(
        noise_words, noise_words + sizeof noise_words / sizeof *noise_words);
    std::vector<llvm::StringRef> parms(num_parms);

    for (unsigned i = 0; i < num_parms; ++i) {
        const ParmVarDecl* parm = func->getParamDecl(i);
        if (parm->getIdentifier()) {
            parms[i] = parm->getName();
        }
    }

    std::vector<Word> words;
    std::vector<ParmInfo> parm_info(num_parms);
    break_into_words(&words, &parm_info, contract, parms, noise);

    for (size_t i = 0; i < num_parms; ++i) {
        const ParmVarDecl* parm = func->getParamDecl(i);
        if (!parm->getIdentifier()) {
            // Ignore unnamed parameters.
            continue;
        }

        bool matched = parm_info[i].is_matched;
        bool really_matched = matched;
        for (size_t j = 0; !matched && j < num_parms; ++j) {
            matched = i != j && are_numeric_cognates(parms[i], parms[j]);
        }

        if (!matched) {
            llvm::StringRef name = parm->getName();
            d_analyser.report(
                parm->getLocation(),
                check_name, "FD03",
                "Parameter '%0' is not documented in the function contract")
                << name
                << parm->getSourceRange();
        }

        bool first = true;
        size_t first_index = 0;
        for (size_t j = 0; j < words.size(); ++j) {
            if (words[j].parm == i) {
                if (first) {
                    // First use of parameter name in contract.
                    bool specify_found = false;
                    size_t word_slack = std::strtoul(d_analyser.config()
                            ->value("word_slack", comment.getBegin()).c_str(),
                        0, 10);
                    size_t k;
                    // Look for "specify" before parameter.  Intervening words
                    // may be other parameters, noise words, or up to
                    // "word_slack" arbitray words.
                    for (k = j; !specify_found && k > 0; --k) {
                        const Word& word = words[k - 1];
                        if (word.is_specify) {
                            specify_found = true;
                        } else if (!word.is_noise &&
                                   !word.is_quoted &&
                                   word.parm >= parms.size() &&
                                   word_slack-- == 0) {
                            break;
                        }
                    }

                    // If we know that the parameter name appears both quoted
                    // and unquoted, and that this instance of the parameter
                    // name is unquoted and doesn't have "specified", assume
                    // that this is a Standard English use of the word rather
                    // than a naming of the parameter, and so ignore it.  E.g.,
                    //..
                    //  For the hash key, use the specified 'key' combined with
                    //  the specified 'salt'.
                    //..
                    if (   !specify_found
                        && parm_info[i].is_quoted
                        && parm_info[i].is_not_quoted
                        && !words[j].is_quoted) {
                        continue;
                    }

                    first_index = j;

                    if (parm->hasDefaultArg() &&
                        (!specify_found ||
                         k < 1 ||
                         !words[k - 1].is_optionally)) {
                        SourceRange r = word_range(comment, words[j]);
                        d_analyser.report(
                                r.getBegin(),
                                check_name, "FD05",
                                "Call out the first appearance of an optional "
                                "parameter in a function contract using the "
                                "phrase 'optionally specify'")
                            << r;
                    } else if (!specify_found) {
                        SourceRange r = word_range(comment, words[j]);
                        d_analyser.report(
                                r.getBegin(),
                                check_name, "FD06",
                                "Call out the first appearance of a parameter "
                                "in a function contract using the word "
                                "'specified' or 'specify'")
                            << r;
                    }
                    first = false;
                } else {
                    for (size_t k = j; k > 0; --k) {
                        const Word& word = words[k - 1];
                        if (word.is_specify) {
                            SourceRange r = word_range(comment, word);
                            d_analyser.report(
                                r.getBegin(),
                                check_name, "FD07",
                                "Call out only the first appearance of a "
                                "parameter in a function contract with the "
                                "word 'specified' or 'specify'")
                                << r;
                            break;
                        } else if (!word.is_noise) {
                            break;
                        }
                    }
                }
            }
        }

        // Warn about unquoted parameters unless they're on the whitelist.
        if (really_matched && parm_info[i].is_not_quoted) {
            std::string ok =
                " " + llvm::StringRef(d_analyser.config()->value(
                                          "ok_unquoted", comment.getBegin()))
                          .lower() +
                " ";
            std::string mw = " " + parms[i].lower() + " ";
            if (ok.find(mw) == ok.npos) {
                for (size_t j = first_index; j < words.size(); ++j) {
                    const Word& word = words[j];
                    if (word.parm == i && !word.is_quoted && !word.is_noise) {
                        SourceRange r = word_range(comment, word);
                        d_analyser.report(
                                r.getBegin(),
                                check_name, "FD04",
                                "Parameter '%0' is not single-quoted in the "
                                "function contract")
                            << parms[i]
                            << r;
                    }
                }
            }
        }
    }
}

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
    // Hook up the callback functions.
{
    analyser.onTranslationUnitDone += report(analyser);
    observer.onComment += comments(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static cool::csabase::RegisterCheck c1(check_name, &allFunDecls);
static cool::csabase::RegisterCheck c2(check_name, &allTpltFunDecls);
static cool::csabase::RegisterCheck c3(check_name, &subscribe);
