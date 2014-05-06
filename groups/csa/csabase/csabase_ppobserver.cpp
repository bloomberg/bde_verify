// csabase_ppobserver.cpp                                             -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_ppobserver.h>
#include <csabase_debug.h>
#include <csabase_location.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceManager.h>
#include <llvm/Support/Regex.h>
#include <llvm/Support/raw_ostream.h>
#ident "$Id$"

// -----------------------------------------------------------------------------

csabase::PPObserver::PPObserver(
    clang::SourceManager const* source_manager,
    csabase::Config* config)
: source_manager_(source_manager)
, connected_(true)
, config_(config)
{
}

// -----------------------------------------------------------------------------

csabase::PPObserver::~PPObserver()
{
}

// -----------------------------------------------------------------------------

void
csabase::PPObserver::detach()
{
    connected_ = false;
}

// -----------------------------------------------------------------------------

namespace
{
    struct CommentHandler
        : clang::CommentHandler
    {
        CommentHandler(csabase::PPObserver* observer): observer_(observer) {}
        bool HandleComment(clang::Preprocessor&, clang::SourceRange range)
        {
            observer_->HandleComment(range);
            return false;
        }
        csabase::PPObserver* observer_;
    };
}

clang::CommentHandler*
csabase::PPObserver::get_comment_handler()
{
    return new CommentHandler(this);
}

// -----------------------------------------------------------------------------

std::string
csabase::PPObserver::get_file(clang::SourceLocation location) const
{
    return source_manager_->getPresumedLoc(location).getFilename();
}

// -----------------------------------------------------------------------------

void
csabase::PPObserver::do_include_file(clang::SourceLocation location, bool is_angled, std::string const& file)
{
    std::string msg("do_include_file '" + file + "' angled=" + (is_angled? "true": "false"));
    csabase::Debug d(msg.c_str());
    onInclude(location, is_angled, file);
}

void
csabase::PPObserver::do_open_file(clang::SourceLocation location, std::string const& from, std::string const& file)
{
    std::string msg("do_open_file '" + file + "'");
    csabase::Debug d(msg.c_str());
    onOpenFile(location, from, file);
}

void
csabase::PPObserver::do_close_file(clang::SourceLocation location, std::string const& from, std::string const& file)
{
    std::string msg("do_close_file '" + file + "'");
    csabase::Debug d(msg.c_str());
    onCloseFile(location, from, file);
}

void
csabase::PPObserver::do_skip_file(std::string const& from, std::string const& file)
{
    std::string msg("do_skip_file(" + from + ", " + file + ")");
    csabase::Debug d(msg.c_str());
    onSkipFile(from, file);
}

void
csabase::PPObserver::do_file_not_found(std::string const& file)
{
    std::string msg("do_file_not_found(" + file + ")");
    csabase::Debug d(msg.c_str());
    onFileNotFound(file);
}

void
csabase::PPObserver::do_other_file(std::string const& file, clang::PPCallbacks::FileChangeReason reason)
{
    std::string msg("do_other_file '" + file + "'");
    csabase::Debug d(msg.c_str());
    onOtherFile(file, reason);
}

void
csabase::PPObserver::do_ident(clang::SourceLocation location, std::string const& ident)
{
    csabase::Debug d("do_ident");
    onIdent(location, ident);
}

void
csabase::PPObserver::do_pragma(clang::SourceLocation location, std::string const& value)
{
    csabase::Debug d("do_pragma");
    onPragma(location, value);
}

void
csabase::PPObserver::do_macro_expands(clang::Token const&          token,
                                            const clang::MacroDirective *macro)
{
    csabase::Debug d("do_macro_expands");
    onMacroExpands(token, macro);
}

void
csabase::PPObserver::do_macro_defined(clang::Token const&          token,
                                            const clang::MacroDirective *macro)
{
    csabase::Debug d("do_macro_defined");
    onMacroDefined(token, macro);
}

void
csabase::PPObserver::do_macro_undefined(
                                            clang::Token const&          token,
                                            const clang::MacroDirective *macro)
{
    csabase::Debug d("do_macro_undefined");
    onMacroUndefined(token, macro);
}

void
csabase::PPObserver::do_if(clang::SourceLocation where,
                                 clang::SourceRange range)
{
    csabase::Debug d("do_if");
    onIf(where, range);
}

void
csabase::PPObserver::do_elif(clang::SourceLocation where,
                                   clang::SourceRange range)
{
    csabase::Debug d("do_elif");
    onElif(where, range);
}

void
csabase::PPObserver::do_ifdef(clang::SourceLocation where,
                                    clang::Token const& token)
{
    csabase::Debug d("do_ifdef");
    onIfdef(where, token);
}

void
csabase::PPObserver::do_ifndef(clang::SourceLocation where,
                                     clang::Token const& token)
{
    csabase::Debug d("do_ifndef");
    onIfndef(where, token);
}

void
csabase::PPObserver::do_else(clang::SourceLocation where,
                                   clang::SourceLocation what)
{
    csabase::Debug d("do_else");
    onElse(where, what);
}

void
csabase::PPObserver::do_endif(clang::SourceLocation where,
                                    clang::SourceLocation what)
{
    csabase::Debug d("do_endif");
    onEndif(where, what);
}

void
csabase::PPObserver::do_comment(clang::SourceRange range)
{
    csabase::Debug d("do_comment");
    onComment(range);
}

void
csabase::PPObserver::do_context()
{
    csabase::Debug d("do_context");
    onContext();
}

// -----------------------------------------------------------------------------

void
csabase::PPObserver::FileChanged(
                                 clang::SourceLocation                location,
                                 clang::PPCallbacks::FileChangeReason reason,
                                 clang::SrcMgr::CharacteristicKind    kind,
                                 clang::FileID                        prev)
{
    if (connected_)
    {
        switch (reason)
        {
        case clang::PPCallbacks::EnterFile:
            {
                std::string file(get_file(location));
                do_open_file(location,
                             files_.empty() ? std::string() : files_.top(),
                             file);
                files_.push(file);
            }
            break;
        case clang::PPCallbacks::ExitFile:
            {
                std::string file(files_.top());
                files_.pop();
                do_close_file(source_manager_->getLocForEndOfFile(prev),
                              files_.empty() ? std::string() : files_.top(),
                              file);
            }
            break;
        default:
            do_other_file(get_file(location), reason);
            break;
        }
    }
}

void
csabase::PPObserver::EndOfMainFile()
{
    if (connected_)
    {
        std::string file(files_.top());
        files_.pop();
        do_close_file(source_manager_->getLocForEndOfFile(
                          source_manager_->getMainFileID()),
                      files_.empty() ? std::string() : files_.top(),
                      file);
    }
}

// -----------------------------------------------------------------------------

void
csabase::PPObserver::FileSkipped(clang::FileEntry const& file, clang::Token const&,
                             clang::SrcMgr::CharacteristicKind kind)
{
    do_skip_file(files_.empty()? std::string(): files_.top(), file.getName());
}

// -----------------------------------------------------------------------------

bool
csabase::PPObserver::FileNotFound(llvm::StringRef name,
                                        llvm::SmallVectorImpl<char>& path)
{
    do_file_not_found(name);
    return false;
}

// -----------------------------------------------------------------------------

void
csabase::PPObserver::Ident(clang::SourceLocation location, std::string const& ident)
{
    do_ident(location, ident);
}

// -----------------------------------------------------------------------------

static llvm::Regex pragma_bdeverify(
    "^[[:blank:]]*" "#" "[[:blank:]]*" "pragma" "[[:blank:]]+"
    "bde_?verify" "[[:blank:]]+" "("                        // 1
        "(" "push"                                    ")|"  // 2
        "(" "pop"                                     ")|"  // 3
        "(" "[-]" "[[:blank:]]*" "([[:alnum:]]+|[*])" ")|"  // 4 5
        "(" "[+]" "[[:blank:]]*" "([[:alnum:]]+|[*])" ")|"  // 6 7
        "(" "set" "[[:blank:]]*" "([_[:alnum:]]+)"          // 8 9
                  "[[:blank:]]*" "(.*[^[:blank:]])"   ")|"  // 10
        "$"
    ")",
    llvm::Regex::NoFlags);

void
csabase::PPObserver::PragmaDirective(clang::SourceLocation location, clang::PragmaIntroducerKind introducer)
{
    const clang::SourceManager& m = *source_manager_;
    clang::FileID fid = m.getFileID(location);
    unsigned line = m.getPresumedLineNumber(location);
    llvm::StringRef directive = 
        m.getBufferData(fid).slice(
            m.getFileOffset(m.translateLineCol(fid, line, 1)),
            m.getFileOffset(m.translateLineCol(fid, line, 0)));
    llvm::SmallVector<llvm::StringRef, 8> matches;
    if (pragma_bdeverify.match(directive, &matches)) {
        if (!matches[2].empty()) {
            config_->push_suppress(location);
        } else if (!matches[3].empty()) {
            config_->pop_suppress(location);
        } else if (!matches[5].empty()) {
            config_->suppress(matches[5], location, true);
        } else if (!matches[7].empty()) {
            config_->suppress(matches[7], location, false);
        } else if (!matches[8].empty()) {
            config_->set_bv_value(location, matches[9], matches[10]);
        }
    }
}

void
csabase::PPObserver::PragmaComment(clang::SourceLocation location, clang::IdentifierInfo const*, std::string const& value)
{
    do_pragma(location, value);
}

void
csabase::PPObserver::PragmaDetectMismatch(clang::SourceLocation  loc,
                                                const std::string     &name,
                                                const std::string     &value)
{
}

void
csabase::PPObserver::PragmaDebug(clang::SourceLocation loc,
                                       llvm::StringRef       debugtype)
{
}

void
csabase::PPObserver::PragmaDiagnosticPush(
                                               clang::SourceLocation loc,
                                               llvm::StringRef       nmspc)
{
}

void
csabase::PPObserver::PragmaDiagnosticPop(clang::SourceLocation loc,
                                               llvm::StringRef       nmspc)
{
}

void
csabase::PPObserver::PragmaDiagnostic(clang::SourceLocation loc,
                                            llvm::StringRef       nmspc,
                                            clang::diag::Mapping  mapping,
                                            llvm::StringRef       str)
{
}

void
csabase::PPObserver::PragmaOpenCLExtension(
                                         clang::SourceLocation        nameloc,
                                         const clang::IdentifierInfo *name,
                                         clang::SourceLocation        stateloc,
                                         unsigned                     state)
{
}

void
csabase::PPObserver::PragmaWarning(clang::SourceLocation loc,
                                         llvm::StringRef       warningspec,
                                         llvm::ArrayRef<int>   ids)
{
}

void
csabase::PPObserver::PragmaWarningPush(clang::SourceLocation loc,
                                             int                   level)
{
}

void
csabase::PPObserver::PragmaWarningPop(clang::SourceLocation loc)
{
}

void
csabase::PPObserver::PragmaMessage(clang::SourceLocation location,
                                         llvm::StringRef       nmspc,
                                         PragmaMessageKind     kind,
                                         llvm::StringRef       value)
{
    do_pragma(location, value);
}

// -----------------------------------------------------------------------------

void
csabase::PPObserver::MacroExpands(clang::Token const&          token,
                                        const clang::MacroDirective *macro,
                                        clang::SourceRange           range,
                                        const clang::MacroArgs      *args)
{
    do_macro_expands(token, macro);
}

void
csabase::PPObserver::MacroDefined(clang::Token const&          token,
                                        const clang::MacroDirective *macro)
{
    do_macro_defined(token, macro);
}

void
csabase::PPObserver::MacroUndefined(clang::Token const&          token,
                                          const clang::MacroDirective *macro)
{
    do_macro_undefined(token, macro);
}

void
csabase::PPObserver::Defined(const clang::Token&          token,
                                   const clang::MacroDirective *macro,
                                   clang::SourceRange           range)
{
}

void
csabase::PPObserver::SourceRangeSkipped(clang::SourceRange range)
{
}

// ----------------------------------------------------------------------------

void
csabase::PPObserver::If(clang::SourceLocation loc,
                              clang::SourceRange    range,
                              bool                  conditionvalue)
{
    do_if(loc, range);
}

void
csabase::PPObserver::Elif(clang::SourceLocation loc,
                                clang::SourceRange    range,
                                bool                  conditionvalue,
                                clang::SourceLocation ifloc)
{
    do_elif(loc, range);
}

void
csabase::PPObserver::Ifdef(clang::SourceLocation        loc,
                                 clang::Token const&          token,
                                 const clang::MacroDirective *md)
{
    do_ifdef(loc, token);
}

void
csabase::PPObserver::Ifndef(clang::SourceLocation        loc,
                                  clang::Token const&          token,
                                  const clang::MacroDirective *md)
{
    do_ifndef(loc, token);
}

void
csabase::PPObserver::Else(clang::SourceLocation loc,
                                clang::SourceLocation ifloc)
{
    do_else(loc, ifloc);
}

void
csabase::PPObserver::Endif(clang::SourceLocation loc,
                                 clang::SourceLocation ifloc)
{
    do_endif(loc, ifloc);
}

// ----------------------------------------------------------------------------

void
csabase::PPObserver::HandleComment(clang::SourceRange range)
{
    do_comment(range);
}

void
csabase::PPObserver::Context()
{
    do_context();
}

void
csabase::PPObserver::InclusionDirective(
                                         clang::SourceLocation   HashLoc,
                                         const clang::Token&     IncludeTok,
                                         llvm::StringRef         FileName,
                                         bool                    IsAngled,
                                         clang::CharSourceRange  FilenameRange,
                                         const clang::FileEntry *File,
                                         llvm::StringRef         SearchPath,
                                         llvm::StringRef         RelativePath,
                                         const clang::Module    *Imported)
{
    do_include_file(HashLoc, IsAngled, FileName);
    //-dk:TODO make constructive use of this...
}

void
csabase::PPObserver::moduleImport(clang::SourceLocation  ImportLoc,
                                        clang::ModuleIdPath    Path,
                                        const clang::Module   *Imported)
{
}
