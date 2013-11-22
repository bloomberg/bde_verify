// csabbg_functioncontract.cpp                                        -*-C++-*-
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
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
    std::vector<SourceRange>                   d_comments;  // Comment blocks.
    std::map<const FunctionDecl*, SourceRange> d_fundecls;  // FunDecl, comment
};

struct comments
    // Callback object for inspecting comments.
{
    Analyser& d_analyser;                   // Analyser object.
    SourceManager& d_manager;               // SourceManager within Analyser.
    std::vector<SourceRange>& d_comments;   // Analyser's comment data.

    comments(Analyser& analyser);
        // Create a 'comments' object, accessing the specified 'analyser'.

    bool areConsecutive(const SourceRange& r1, const SourceRange& r2) const;
        // Return whether the specified 'r1' is immediately followed by the
        // specified 'r2', i.e., 'r2' begins in the same file in which 'r1'
        // ends, either on the same line or the line after.  Note that there
        // might be code (not comments) between the ranges, but this check is
        // good enough for the purpose of considering multi-line "//" comments
        // as single comment blocks.

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

bool
comments::areConsecutive(const SourceRange& r1, const SourceRange& r2) const
{
    unsigned r1e = d_manager.getPresumedLineNumber(r1.getEnd());
    unsigned r2b = d_manager.getPresumedLineNumber(r2.getBegin());
    return d_manager.getFileID(r1.getEnd()) ==
           d_manager.getFileID(r2.getBegin()) &&
           (r1e == r2b || r1e + 1 == r2b);
}

void comments::operator()(SourceRange range)
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

void allFunDecls(Analyser& analyser, const FunctionDecl* func)
    // Callback function for inspecting function declarations.
{
    // Don't process compiler-defaulted methods, main, or macro expansions.
    if (   !func->isDefaulted()
        && !func->isMain()
        && !func->getLocation().isMacroID()) {
        analyser.attachment<data>().d_fundecls[func];
    }
}
 
void allTpltFunDecls(Analyser& analyser, const FunctionTemplateDecl* func)
    // Callback function for inspecting function template declarations.
{
    allFunDecls(analyser, func->getTemplatedDecl());
}

static char
toLower(unsigned char c)
    // Return the lower-case version of the specified 'c'.
{
    return tolower(c);
}

static std::string
toLower(std::string value)
    // Return the lower-case version of the specified 'value'.
{
    std::transform(value.begin(), value.end(), value.begin(),
            static_cast<char(*)(unsigned char)>(&toLower));
    return value;
}

static bool
isIdChar(unsigned char c)
    // Return whether the specified 'c' is a C++ identifier character.
{
    return c == '_' || isalnum(c);
}

static SourceRange
getWordLoc(SourceLocation loc, const std::string& src, const std::string& word)
    // Return a range within the specified 'loc' corresponding to the position
    // of the specified isolated 'word' within the specified 'src' (i.e., not
    // immediately preceeded or followed by a C++ identifier character) or an
    // invalid range if 'word' cannot be so found in 'src'.
{
    SourceRange ret;
    for (std::string::size_type pos = 0;
                 (pos = src.find(word, pos)) != src.npos; pos += word.size()) {
        if (   (   pos == 0
                || !isIdChar(src[pos - 1]))
            && (   pos + word.size() >= src.size()
                || !isIdChar(src[pos + word.size()]))) {
            ret = SourceRange(loc.getLocWithOffset(pos),
                              loc.getLocWithOffset(pos + word.size() - 1));
            break;
        }
    }
    return ret;
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

    template <class IT>
    void processAllFunDecls(IT begin, IT end);
        // Utility method to process function declarations from the specified
        // 'begin' up to the specified 'end'.

    bool doesNotNeedContract(const FunctionDecl *func);
        // Return 'true' iff the specified 'func' does not need a contract.
        //
        // Reasons:
        //: o Not the canonical declaration
        //: o Private copy constructor declaration.
        //: o Private assignment operator declaration.
        //: o Template method specialization.

    template <class IT>
    SourceRange getContract(const FunctionDecl *func,
                            IT comments_begin, IT comments_end);
        // Return the 'SourceRange' of the function contract of the specified
        // 'func' if it is present in the specified range of 'comments_begin'
        // up to 'comments_end', and return an invalid 'SourceRange' otherwise.

    void critiqueContract(const FunctionDecl *func, SourceRange comment);
        // Issue diagnostics for deficiencies in the specified 'comment' with
        // respect to being a contract for the specified 'func'.

    bool areCognates(const FunctionDecl *a, const FunctionDecl *b);
        // Return 'true' iff the specified 'a' and 'b' are similar in a way
        // such that it is sufficient for only one of them to have a contract.
};

report::report(Analyser& analyser)
: d_analyser(analyser)
, d_manager(analyser.manager())
, d(analyser.attachment<data>())
{
}

void report::operator()()
{
    processAllFunDecls(d.d_fundecls.begin(), d.d_fundecls.end());
}

template <class IT>
void report::processAllFunDecls(IT begin, IT end)
{
    for (IT it = begin; it != end; ++it) {
        // Check whether a comment exists before checking whether a comment
        // is needed, so that unneeded but present comments will be checked
        // for malformations.
        it->second =
              getContract(it->first, d.d_comments.begin(), d.d_comments.end());
    }

    for (IT it = begin; it != end; ++it) {
        if (it->second.isValid()) {
            critiqueContract(it->first, it->second);
        } else if (doesNotNeedContract(it->first)) {
        } else {
            bool hasCommentedCognate = false;
            for (IT cit = begin; !hasCommentedCognate && cit != end; ++cit) {
                hasCommentedCognate =
                   cit->second.isValid() && areCognates(cit->first, it->first);
            }
            if (!hasCommentedCognate) {
                d_analyser.report(it->first->getNameInfo().getLoc(),
                                  check_name, "FD01: "
                                  "Function declaration requires contract")
                    << it->first->getNameInfo().getSourceRange();
            }
        }
    }
}

bool report::doesNotNeedContract(const FunctionDecl *func)
{
    const CXXConstructorDecl *ctor;
    const CXXMethodDecl *meth;
    const FunctionDecl *ifmf;

    return func != func->getCanonicalDecl()
        || (   func->getAccess() == clang::AS_private
            && !func->hasBody()
            && (   (   (ctor = llvm::dyn_cast<CXXConstructorDecl>(func))
                    && ctor->isCopyConstructor())
                || (   (meth = llvm::dyn_cast<CXXMethodDecl>(func))
                    && meth->isCopyAssignmentOperator())))
        || (   (ifmf = func->getInstantiatedFromMemberFunction())
            && (   func != ifmf
                || doesNotNeedContract(ifmf)));
}

template <class IT>
SourceRange report::getContract(const FunctionDecl *func,
                                IT comments_begin, IT comments_end)
{
    SourceManager& m = d_manager;
    SourceRange contract;

    unsigned sline;
    clang::FileID sfile;

    // Find the line number that the function contract should encompass.
    if (func->doesThisDeclarationHaveABody()) {
        // For functions with bodies, the end of the 'FunctionDecl' is the
        // end of the body, so instead get the line before the body begins,
        // if they're not all on the same line.
        SourceLocation bodyloc = func->getBody()->getLocStart();
        sfile = m.getFileID(bodyloc);

        unsigned bodyBegin = m.getPresumedLineNumber(bodyloc);
        unsigned funcBegin = m.getPresumedLineNumber(func->getLocStart());

        if (bodyBegin <= funcBegin) {
            sline = bodyBegin + 1;
        } else {
            sline = bodyBegin - 1;
        }
    } else {
        // For plain declarations, use the line after.
        SourceLocation declloc = func->getLocEnd();
        sfile = m.getFileID(declloc);
        sline = m.getPresumedLineNumber(declloc) + 1;
    }

    for (IT it = comments_begin; it != comments_end; ++it) {
        const SourceRange comment = *it;
        const SourceLocation cloc = comment.getBegin();
        const unsigned cline = m.getPresumedLineNumber(cloc);
        const unsigned clineend = m.getPresumedLineNumber(comment.getEnd());
        const clang::FileID cfile = m.getFileID(cloc);

        // Find a comment that encompasses the contract line, in the same
        // file as the 'FunctionDecl'.
        if (cfile == sfile && cline <= sline && clineend >= sline) {
            contract = comment;
            break;
        }
    }

    return contract;
}

void report::critiqueContract(const FunctionDecl* func, SourceRange comment)
{
    const SourceLocation cloc = comment.getBegin();

    // Check for bad indentation.
    const int fcolm = d_manager.getPresumedColumnNumber(func->getLocStart());
    const int ccolm = d_manager.getPresumedColumnNumber(cloc);
    if (ccolm != fcolm + 4) {
        d_analyser.report(cloc, check_name, "FD01: "
            "Function contracts should be indented 4, not %0, spaces "
            "from their function declaration")
            << (ccolm - fcolm)
            << comment;
    }

    // Now check that the function contract documents the parameters.
    // If there are any parameters, we only check for a single instance
    // of the words 'specify' or 'specified', because contracts can say
    // things like "the specified 'a' and 'b'".  Similarly, if there
    // are optional parameters, we only check for a single instance of
    // the word "optionally".
    const unsigned num_parms = func->getNumParams();
    const std::string contract = toLower(d_analyser.get_source(comment));
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
        const std::string name = parm->getNameAsString();
        const std::string lcname = toLower(name);
        const SourceRange crange = getWordLoc(cloc, contract, lcname);
        if (!crange.isValid()) {
            // Did not find parameter name in contract.
            d_analyser.report(parm->getSourceRange().getBegin(),
                check_name, "FD01: "
                "Parameter '%0' is not documented in the function "
                "contract")
                << name
                << parm->getSourceRange();
        } else {
            if (contract.find("'" + lcname + "'") == contract.npos) {
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
                && contract.find(optionally) == contract.npos) {
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
                && contract.find(specified) == contract.npos
                && contract.find(specify) == contract.npos) {
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
}

bool report::areCognates(const FunctionDecl* a, const FunctionDecl* b)
{
    // Temporary simple version

    return a->getLookupParent()->Equals(b->getLookupParent())
        && a->getNameAsString() == b->getNameAsString();
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
