// csabbg_functioncontract.cpp                                        -*-C++-*-
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <llvm/Support/Regex.h>
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
    // Look for "= default" and "= delete" comments.
    static llvm::Regex re("^(//|/[*])" "[[:space:]]*" "=" "[[:space:]]*"
                          "(delete|default)" "[[:space:]]*" ";?"
                          "[[:space:]]*" "([*]/)?" "[[:space:]]*" "$",
                          llvm::Regex::IgnoreCase);
    return re.match(comment);
}

bool
comments::areConsecutive(const SourceRange& r1, const SourceRange& r2) const
{
    SourceRange between(r1.getEnd(), r2.getBegin());
    llvm::StringRef s = d_analyser.get_source(between, true);
    static llvm::Regex re("^[[:space:]]*$");
    return re.match(s) &&
           !comments::isDirective(d_analyser.get_source(r1)) &&
           !comments::isDirective(d_analyser.get_source(r2));
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
        it->second =
              getContract(it->first, d.d_comments.begin(), d.d_comments.end());
    }

    for (IT it = begin; it != end; ++it) {
        if (doesNotNeedContract(it->first)) {
        }
        else if (it->second.isValid()) {
            critiqueContract(it->first, it->second);
        }
        else {
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
    SourceRange declarator = func->getSourceRange();
    SourceRange contract;

    unsigned bline;
    unsigned eline;
    clang::FileID sfile;

    // Find the line numbers that the function contract may encompass.
    if (func->doesThisDeclarationHaveABody() && func->getBody()) {
        // For functions with bodies, the end of the 'FunctionDecl' is the
        // end of the body, so instead get the line before the body begins,
        // if they're not all on the same line.
        SourceLocation bodyloc = func->getBody()->getLocStart();
        declarator.setEnd(bodyloc);
        sfile = m.getFileID(bodyloc);

        unsigned bodyBegin = m.getPresumedLineNumber(bodyloc);
        unsigned funcBegin = m.getPresumedLineNumber(func->getLocStart());

        bline = funcBegin;

        if (bodyBegin <= funcBegin) {
            eline = bodyBegin + 1;
        } else {
            eline = bodyBegin - 1;
        }
    } else {
        // For plain declarations, use the line after.
        SourceLocation declloc = func->getLocEnd();
        sfile = m.getFileID(declloc);
        bline = m.getPresumedLineNumber(declloc);
        eline = bline + 1;
    }

    for (IT it = comments_begin; it != comments_end; ++it) {
        const SourceRange comment = *it;
        const SourceLocation cloc = comment.getBegin();
        const unsigned cline = m.getPresumedLineNumber(cloc);
        const clang::FileID cfile = m.getFileID(cloc);

        // Find a comment that encompasses the contract line, in the same file
        // as the 'FunctionDecl' and starting after it.  (We want to avoid
        // taking a commnted parameter name as a function contract!)
        if (cfile == sfile) {
            if ((m.getPresumedLineNumber(declarator.getBegin()) == cline &&
                 m.getPresumedColumnNumber(declarator.getEnd()) <=
                 m.getPresumedColumnNumber(comment.getBegin())) ||
                (bline < cline && cline <= eline)) {
                contract = comment;
                break;
            }
        }
    }

    return contract;
}

void report::critiqueContract(const FunctionDecl* func, SourceRange comment)
{
    llvm::StringRef contract = d_analyser.get_source(comment);

    // Ignore "= default" and "= delete" comments.
    if (comments::isDirective(contract)) {
        return;                                                       // RETURN
    }

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
    bool specify_done = false;
    bool optional_done = false;

    for (unsigned i = 0; i < num_parms; ++i) {
        const ParmVarDecl* parm = func->getParamDecl(i);
        if (!parm->getIdentifier()) {
            // Ignore unnamed parameters.
            continue;
        }
        // Compare with case insensitivity to avoid sentence capitalization
        // issues.  This could lead to false positives if there are multiple
        // parameters with the same spelling but different cases, but we'll
        // live with that.
        const std::string name = parm->getNameAsString();
        llvm::Regex pre("^([^_[:alnum:]]*|.*[^_[:alnum:]])" +
                        name +
                        "([^_[:alnum:]]*|[^_[:alnum:]].*)$",
                        llvm::Regex::IgnoreCase);
        llvm::SmallVector<llvm::StringRef, 3> matches;
        if (!pre.match(contract, &matches)) {
            // Did not find parameter name in contract.
            d_analyser.report(parm->getSourceRange().getBegin(),
                check_name, "FD01: "
                "Parameter '%0' is not documented in the function contract")
                << name
                << parm->getSourceRange();
        } else {
            SourceRange crange(comment.getBegin().getLocWithOffset(
                                                       matches[1].size()),
                              comment.getEnd().getLocWithOffset(
                                                      -matches[2].size() - 1));
            llvm::Regex qre("^([^']|'[^']*')*"
                            "'([^_[:alnum:]']*|[^']*[^_[:alnum:]])" +
                            name +
                            "([^_[:alnum:]']*|[^_[:alnum:]][^']*)'",
                            llvm::Regex::IgnoreCase);
            if (!qre.match(contract)) {
                // Found parameter name, but unticked.
                d_analyser.report(crange.getBegin(),
                     check_name, "FD01: "
                    "Parameter '%0' is not single-quoted in the function "
                    "contract")
                    << name
                    << crange;
            }
            static llvm::Regex ore("(^|[^_[:alnum:]])"
                                   "optionally"
                                   "([^_[:alnum:]]|$)",
                                   llvm::Regex::IgnoreCase);
            if (   !optional_done
                && parm->hasDefaultArg()
                && !ore.match(contract)) {
                // The first time we see a named optional parameter,
                // check for "optionally" in the contract.
                optional_done = true;
                d_analyser.report(crange.getBegin(),
                        check_name, "FD01: "
                        "In the function contract, use the phrase 'optionally "
                        "specify' to document parameters that have default "
                        "arguments")
                    << crange;
            }
            static llvm::Regex sre("(^|[^_[:alnum:]])"
                                   "(specified|specify)"
                                   "([^_[:alnum:]]|$)",
                                   llvm::Regex::IgnoreCase);
            if (   !specify_done
                && !parm->hasDefaultArg()
                && !sre.match(contract)) {
                // The first time we see a named required parameter,
                // check for "specified" or "specify" in the contract.
                specify_done = true;
                d_analyser.report(crange.getBegin(),
                        check_name, "FD01: "
                        "In the function contract, use the words 'specified' "
                        "or 'specify' to document parameters")
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
