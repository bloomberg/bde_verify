// csabbg_functioncontract.cpp                                        -*-C++-*-
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <set>
#include <string>
#include <sstream>
#include <vector>

#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("function-contract");

// ----------------------------------------------------------------------------

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
using cool::csabase::RegisterCheck;
using cool::csabase::Visitor;

namespace
{

struct data
    // Data attached to analyzer for this check.
{
    std::set<const FunctionDecl*> d_fundecls;   // Canonical function decls.
    std::vector<SourceRange>      d_comments;   // Commenty blocks.
};

struct comments
    // Callback object for inspecting comments.
{
    Analyser& d_analyser;                   // Analyser object.
    SourceManager& d_manager;               // SourceManager within Analyser.
    std::vector<SourceRange>& d_comments;   // Analyser's comment data.

    comments(Analyser& analyser)
        // Create a 'comments' object.
    : d_analyser(analyser)
    , d_manager(analyser.manager())
    , d_comments(analyser.attachment<data>().d_comments)
    { }

    bool areConsecutive(const SourceRange& r1, const SourceRange& r2)
        // Return whether the specified 'r1' is immediately followed by the
        // specified 'r2', i.e., 'r2' begins in the same file in which 'r1'
        // ends, either on the same line or the line after.  Note that there
        // might be code (not comments) between the ranges, but this check is
        // good enough for the purpose of considering multi-line "//" comments
        // as single comment blocks.
    {
        unsigned r1e = d_manager.getPresumedLineNumber(r1.getEnd());
        unsigned r2b = d_manager.getPresumedLineNumber(r2.getBegin());
        return d_manager.getFileID(r1.getEnd()) ==
               d_manager.getFileID(r2.getBegin()) &&
               (r1e == r2b || r1e + 1 == r2b);
    }

    void operator()(SourceRange range)
        // The specified 'range', representing a comment, is either appended to
        // the previous comment or added separately to the comments list.
    {
        Location location(d_analyser.get_location(range.getBegin()));
        if (d_analyser.is_component(location.file())) {
            if (d_comments.size() == 0 ||
                !areConsecutive(d_comments.back(), range)) {
                d_comments.push_back(range);
            } else {
                d_comments.back().setEnd(range.getEnd());
            }
        }
    }
};

void all_fundecls(Analyser& analyser, const FunctionDecl* func)
    // Callback function for inspecting function declarations.
{
    // Process only the first declaration of a function.
    // Don't process compiler-defaulted methods.
    if (   !func->isDefaulted()
        && !func->isMain()
        && func == func->getCanonicalDecl()) {
        analyser.attachment<data>().d_fundecls.insert(func);
    }
}
 
void all_tpltfundecls(Analyser& analyser, const FunctionTemplateDecl* func)
    // Callback function for inspecting function template declarations.
{
    // Process only the first declaration of a function template.
    if (func == func->getCanonicalDecl()) {
        analyser.attachment<data>().d_fundecls.insert(func->getTemplatedDecl());
    }
}

static char
toLower(unsigned char c)
    // Return the lower-case version of the specified 'c'.
{
    return std::tolower(c);
}

static std::string
toLower(std::string value)
    // Return the lower-case version of the specified 'value'.
{
    std::transform(value.begin(), value.end(), value.begin(),
            static_cast<char(*)(unsigned char)>(&::toLower));
    return value;
}

static bool
isIdChar(unsigned char c)
    // Return whether the specified 'c' is a C++ identifier character.
{
    return c == '_' || std::isalnum(c);
}

static SourceRange
getWordLoc(SourceLocation loc, const std::string& src, const std::string& word)
    // Return a range within the specified 'loc' corresponding to the position
    // of the specified isolated 'word' within the specified 'src' (i.e., not
    // immediately preceeded or followed by a C++ identifier character) or an
    // invalid range if 'word' cannot be so found in 'src'.
{
    SourceRange ret;
    std::string::size_type pos = src.find(word);
    if (   pos != src.npos
        && word.size() > 0
        && (   pos == 0
            || !isIdChar(src[pos - 1]))
        && (   pos + word.size() >= src.size()
            || !isIdChar(src[pos + word.size()]))) {
        ret.setBegin(loc.getLocWithOffset(pos));
        ret.setEnd(ret.getBegin().getLocWithOffset(word.length() - 1));
    }
    return ret;
}

struct report
    // Callback object invoked upon completion.
{
    Analyser& d_analyser;       // Analyser object.
    SourceManager& d_manager;   // SourceManager within Analyser.
    const data& d;              // Analyser's data for this module.

    report(Analyser& analyser)
    : d_analyser(analyser)
    , d_manager(analyser.manager())
    , d(analyser.attachment<data>()) {}


    void operator()()
    {
        process_all_fundecls(d.d_fundecls.begin(), d.d_fundecls.end());
    }

    template <class IT>
    void process_all_fundecls(IT begin, IT end)
    {
        for (IT it = begin; it != end; ++it) {
            if (!is_commented(*it, d.d_comments.begin(), d.d_comments.end())) {
                d_analyser.report((*it)->getNameInfo().getLoc(),
                                  check_name, "FD01: "
                                  "Function declaration requires contract")
                    << (*it)->getNameInfo().getSourceRange();
            }
        }
    }

    template <class IT>
    bool is_commented(const FunctionDecl *func,
                      IT comments_begin,
                      IT comments_end)
        // Return whether a function declaration has a contract comment, and
        // issue warnings for various malformations.
    {
        SourceManager& m = d_manager;

        unsigned sline;
        clang::FileID sfile;

        // Find the line number that the function contract should encompass.
        if (func->doesThisDeclarationHaveABody()) {
            // For functions with bodies, the end of the 'FunctionDecl' is the
            // end of the body, so instead get the line before the body begins.
            SourceLocation bodyloc = func->getBody()->getLocStart();
            sfile = m.getFileID(bodyloc);
            sline = m.getPresumedLineNumber(bodyloc) - 1;
            if (sline < m.getPresumedLineNumber(func->getLocStart())) {
                // If the body begins on the same line as the 'FunctionDecl',
                // use that line.
                ++sline;
            }
        } else {
            // For plain declarations, use the line after.
            SourceLocation declloc = func->getLocEnd();
            sfile = m.getFileID(declloc);
            sline = m.getPresumedLineNumber(declloc) + 1;
        }

        for (IT it = comments_begin; it != comments_end; ++it) {
            const SourceLocation cloc = (*it).getBegin();
            const unsigned cline = m.getPresumedLineNumber(cloc);
            const unsigned clineend = m.getPresumedLineNumber((*it).getEnd());
            const clang::FileID cfile = m.getFileID(cloc);

            // Find a comment that encompasses the contract line, in the same
            // file as the 'FunctionDecl'.
            if (cfile != sfile || cline > sline || clineend < sline) {
                continue;
            }

            // Check for bad indentation.
            const int fcolm = m.getPresumedColumnNumber(func->getLocStart());
            const int ccolm = m.getPresumedColumnNumber(cloc);
            if (ccolm != fcolm + 4) {
                d_analyser.report(cloc, check_name, "FD01: "
                    "Function contracts should be indented 4, not %0, spaces "
                    "from their function declaration")
                    << (ccolm - fcolm)
                    << (*it);
            }

            // Now check that the function contract documents the parameters.
            // If there are any parameters, we only check for a single instance
            // of the words 'specify' or 'specified', because contracts can say
            // things like "the specified 'a' and 'b'".  Similarly, if there
            // are optional parameters, we only check for a single instance of
            // the word "optionally".
            const unsigned num_parms = func->getNumParams();
            const std::string comment = toLower(d_analyser.get_source(*it));
            bool specify_done = false;
            bool optional_done = false;

            for (unsigned i = 0; i < num_parms; ++i) {
                const ParmVarDecl* parm = func->getParamDecl(i);
                if (!parm->getIdentifier()) {
                    // Ignore unnamed parameters.
                    continue;
                }
                // Convert to lower case to avoid sentence capitalization
                // issues.  This could lead to false positives if there are
                // multiple parameters with the same spelling but different
                // cases, but we'll live with that.
                const std::string name = toLower(parm->getNameAsString());
                const SourceRange crange = getWordLoc(cloc, comment, name);
                if (!crange.isValid()) {
                    // Did not find parameter name in contract.
                    d_analyser.report(parm->getSourceRange().getBegin(),
                        check_name, "FD01: "
                        "Parameter '%0' is not documented in the function "
                        "contract")
                        << name
                        << parm->getSourceRange();
                } else {
                    if (comment.find("'" + name + "'") == comment.npos) {
                        // Found parameter name, but unticked.
                        d_analyser.report(crange.getBegin(),
                             check_name, "FD01: "
                            "Parameter '%0' is not single-quoted in the "
                            "function contract")
                            << name
                            << crange;
                    }
                    static std::string optionally("optionally");
                    if (   !optional_done
                        && parm->hasDefaultArg()
                        && comment.find(optionally) == comment.npos) {
                        // The first time we see a named optional parameter,
                        // check for "optionally" in the contract.
                        optional_done = true;
                        d_analyser.report(crange.getBegin(), check_name,
                            "FD01: "
                            "In the function contract, use the phrase "
                            "'optionally specify' to document parameters "
                            "that have default arguments")
                            << crange;
                    }
                    static std::string specified("specified");
                    static std::string specify("specify");
                    if (   !specify_done
                        && !parm->hasDefaultArg()
                        && comment.find(specified) == comment.npos
                        && comment.find(specify) == comment.npos) {
                        // The first time we see a named required parameter,
                        // check for "specified" or "specify" in the contract.
                        specify_done = true;
                        d_analyser.report(crange.getBegin(), check_name,
                            "FD01: "
                            "In the function contract, use the words "
                            "'specified' or 'specify' to document "
                            "parameters")
                            << crange;
                    }
                }
            }

            // Any comment at all encompassing the desired line qualifies.
            return true;
        }

        // Found no comment at the desired location.
        return false;
    }
};

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
    // Hook up the callback functions.
{
    analyser.onTranslationUnitDone += report(analyser);
    observer.onComment += comments(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static RegisterCheck c1(check_name, &all_fundecls);
static RegisterCheck c2(check_name, &all_tpltfundecls);
static RegisterCheck c3(check_name, &subscribe);
