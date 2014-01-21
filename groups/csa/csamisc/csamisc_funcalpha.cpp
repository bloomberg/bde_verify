// csabbg_funcalpha.cpp                                               -*-C++-*-
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_util.h>
#include <csabase_visitor.h>
#include <llvm/Support/Regex.h>
#include <clang/AST/TypeLoc.h>
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <ctype.h>

#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("alphabetical-functions");

// ----------------------------------------------------------------------------

using clang::Decl;
using clang::DeclarationName;
using clang::FunctionDecl;
using clang::FunctionTemplateDecl;
using clang::FunctionTypeLoc;
using clang::SourceManager;
using clang::SourceRange;
using clang::TypeLoc;
using cool::csabase::Analyser;
using cool::csabase::Location;
using cool::csabase::PPObserver;
using cool::csabase::Visitor;

namespace
{

struct data
    // Data attached to analyzer for this check.
{
    typedef std::set<unsigned> Lines;
    typedef std::map<std::string, Lines> Comments;
    Comments d_comments;  // Comment lines per file.
};

struct comments
    // Callback object for inspecting comments.
{
    Analyser& d_analyser;         // Analyser object.
    SourceManager& d_manager;     // SourceManager within Analyser.
    data::Comments& d_comments;   // Analyser's comment data.

    comments(Analyser& analyser);
        // Create a 'comments' object, accessing the specified 'analyser'.

    static bool isReset(llvm::StringRef comment);
        // Return wehether the specified 'comment' acts as a "reset" marker for
        // alphabetical ordering.

    void operator()(SourceRange range);
        // The specified 'range', representing a comment, is added to the
        // comments list.
};

comments::comments(Analyser& analyser)
: d_analyser(analyser)
, d_manager(analyser.manager())
, d_comments(analyser.attachment<data>().d_comments)
{
}

bool comments::isReset(llvm::StringRef comment)
{
    // Look for "aspect" directives in comments.
    static llvm::Regex re("^(//|/[*])" "[[:space:]]*"
                          "("
                             "CLASS"     "|"
                             "FREE"      "|"
                             "PRIVATE"   "|"
                             "PROTECTED" "|"
                             "PUBLIC"    "|"
                             "[[:space:]]"
                          ")*"
                          "("
                             "Aspects?"      "|"
                             "ACCESSORS?"    "|"
                             "METHODS?"      "|"
                             "CREATORS?"     "|"
                             "DATA"          "|"
                             "FUNCTIONS?"    "|"
                             "OPERATORS?"    "|"
                             "MANIPULATORS?" "|"
                             "TRAITS?"       "|"
                             "[-=_]+" "(" "[[:space:]]*" "[-=_]+" ")*"
                          ")"
                          "[:;.[:space:]]*" "([*]/)?" "[[:space:]]*" "$",
                          llvm::Regex::IgnoreCase);
    return re.match(comment);
}

void comments::operator()(SourceRange range)
{
    Location location(d_analyser.get_location(range.getBegin()));
    if (   d_analyser.is_component(location.file())
        && isReset(d_analyser.get_source(range))) {
        d_comments[location.file()].insert(location.line());
    }
}

struct report
    // Callback object invoked upon completion.
{
    Analyser& d_analyser;       // Analyser object.
    SourceManager& d_manager;   // SourceManager within Analyser.
    data& d;                    // Analyser's data for this module.

    report(Analyser& analyser);
        // Create a 'report' object, accessing the specified 'analyser'.

    void operator()(const FunctionDecl *decl);
        // Invoked to process function declarations.

    void operator()(const FunctionTemplateDecl *decl);
        // Invoked to process function template declarations.
};

report::report(Analyser& analyser)
: d_analyser(analyser)
, d_manager(analyser.manager())
, d(analyser.attachment<data>())
{
}

void report::operator()(const FunctionTemplateDecl *decl)
{
    (*this)(decl->getTemplatedDecl());
}

void report::operator()(const FunctionDecl *decl)
{
    DeclarationName name = decl->getDeclName();
    const Decl *next = decl->getNextDeclInContext();
    if (   (   d_analyser.is_component_header(decl)
            || d_analyser.is_component_source(decl))
        && next
        && name.isIdentifier()
        && !name.isEmpty()) {
        const FunctionDecl *nextf = llvm::dyn_cast<FunctionDecl>(next);
        const FunctionTemplateDecl* nextt =
            llvm::dyn_cast<FunctionTemplateDecl>(next);
        if (nextt) {
            nextf = nextt->getTemplatedDecl();
        }
        if (nextf) {
            DeclarationName next_name = nextf->getDeclName();
            if (next_name.isIdentifier() && !next_name.isEmpty() &&
                name.getAsString() > next_name.getAsString()) {
                Location l1 = d_analyser.get_location(decl);
                Location l2 = d_analyser.get_location(nextf);
                const data::Lines &lines = d.d_comments[l1.file()];
                bool reset = false;
                for (unsigned i = l1.line(); !reset && i <= l2.line(); ++i) {
                    reset = lines.count(i);
                }
                if (!reset) {
                    d_analyser.report(decl->getLocation(),
                                      check_name, "FABC01",
                                      "Function '%0' not in alphabetical order")
                        << name.getAsString();
                    d_analyser.report(nextf->getLocation(),
                                      check_name, "FABC01",
                                      "Next function is '%0'",
                                      false, clang::DiagnosticsEngine::Note)
                        << next_name.getAsString();
                }
            }
        }
    }
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    observer.onComment             += comments(analyser);
    visitor.onFunctionDecl         += report(analyser);
    visitor.onFunctionTemplateDecl += report(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static cool::csabase::RegisterCheck c3(check_name, &subscribe);
