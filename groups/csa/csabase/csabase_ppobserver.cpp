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
#include <llvm/Support/raw_ostream.h>
#ident "$Id$"

// -----------------------------------------------------------------------------

cool::csabase::PPObserver::PPObserver(clang::SourceManager const* source_manager):
    source_manager_(source_manager),
    connected_(true)
{
}

// -----------------------------------------------------------------------------

cool::csabase::PPObserver::~PPObserver()
{
}

// -----------------------------------------------------------------------------

void
cool::csabase::PPObserver::detach()
{
    this->connected_ = false;
}

// -----------------------------------------------------------------------------

namespace
{
    struct CommentHandler
        : clang::CommentHandler
    {
        CommentHandler(cool::csabase::PPObserver* observer): observer_(observer) {}
        bool HandleComment(clang::Preprocessor&, clang::SourceRange range)
        {
            this->observer_->HandleComment(range);
            return false;
        }
        cool::csabase::PPObserver* observer_;
    };
}

clang::CommentHandler*
cool::csabase::PPObserver::get_comment_handler()
{
    return new CommentHandler(this);
}

// -----------------------------------------------------------------------------

std::string
cool::csabase::PPObserver::get_file(clang::SourceLocation location) const
{
    return this->source_manager_->getPresumedLoc(location).getFilename();
}

// -----------------------------------------------------------------------------

void
cool::csabase::PPObserver::do_include_file(clang::SourceLocation location, bool is_angled, std::string const& file)
{
    std::string msg("do_include_file '" + file + "' angled=" + (is_angled? "true": "false"));
    cool::csabase::Debug d(msg.c_str());
    this->onInclude(location, is_angled, file);
}

void
cool::csabase::PPObserver::do_open_file(clang::SourceLocation location, std::string const& from, std::string const& file)
{
    std::string msg("do_open_file '" + file + "'");
    cool::csabase::Debug d(msg.c_str());
    this->onOpenFile(location, from, file);
}

void
cool::csabase::PPObserver::do_close_file(clang::SourceLocation location, std::string const& from, std::string const& file)
{
    std::string msg("do_close_file '" + file + "'");
    cool::csabase::Debug d(msg.c_str());
    this->onCloseFile(location, from, file);
}

void
cool::csabase::PPObserver::do_skip_file(std::string const& from, std::string const& file)
{
    std::string msg("do_skip_file(" + from + ", " + file + ")");
    cool::csabase::Debug d(msg.c_str());
    this->onSkipFile(from, file);
}

void
cool::csabase::PPObserver::do_file_not_found(std::string const& file)
{
    std::string msg("do_file_not_found(" + file + ")");
    cool::csabase::Debug d(msg.c_str());
    this->onFileNotFound(file);
}

void
cool::csabase::PPObserver::do_other_file(std::string const& file, clang::PPCallbacks::FileChangeReason reason)
{
    std::string msg("do_other_file '" + file + "'");
    cool::csabase::Debug d(msg.c_str());
    this->onOtherFile(file, reason);
}

void
cool::csabase::PPObserver::do_ident(clang::SourceLocation location, std::string const& ident)
{
    cool::csabase::Debug d("do_ident");
    this->onIdent(location, ident);
}

void
cool::csabase::PPObserver::do_pragma(clang::SourceLocation location, std::string const& value)
{
    cool::csabase::Debug d("do_pragma");
    this->onPragma(location, value);
}

void
cool::csabase::PPObserver::do_macro_expands(clang::Token const&          token,
                                            const clang::MacroDirective *macro)
{
    cool::csabase::Debug d("do_macro_expands");
    this->onMacroExpands(token, macro);
}

void
cool::csabase::PPObserver::do_macro_defined(clang::Token const&          token,
                                            const clang::MacroDirective *macro)
{
    cool::csabase::Debug d("do_macro_defined");
    this->onMacroDefined(token, macro);
}

void
cool::csabase::PPObserver::do_macro_undefined(
                                            clang::Token const&          token,
                                            const clang::MacroDirective *macro)
{
    cool::csabase::Debug d("do_macro_undefined");
    this->onMacroUndefined(token, macro);
}

void
cool::csabase::PPObserver::do_if(clang::SourceLocation where,
                                 clang::SourceRange range)
{
    cool::csabase::Debug d("do_if");
    this->onIf(where, range);
}

void
cool::csabase::PPObserver::do_elif(clang::SourceLocation where,
                                   clang::SourceRange range)
{
    cool::csabase::Debug d("do_elif");
    this->onElif(where, range);
}

void
cool::csabase::PPObserver::do_ifdef(clang::SourceLocation where,
                                    clang::Token const& token)
{
    cool::csabase::Debug d("do_ifdef");
    this->onIfdef(where, token);
}

void
cool::csabase::PPObserver::do_ifndef(clang::SourceLocation where,
                                     clang::Token const& token)
{
    cool::csabase::Debug d("do_ifndef");
    this->onIfndef(where, token);
}

void
cool::csabase::PPObserver::do_else(clang::SourceLocation where,
                                   clang::SourceLocation what)
{
    cool::csabase::Debug d("do_else");
    this->onElse(where, what);
}

void
cool::csabase::PPObserver::do_endif(clang::SourceLocation where,
                                    clang::SourceLocation what)
{
    cool::csabase::Debug d("do_endif");
    this->onEndif(where, what);
}

void
cool::csabase::PPObserver::do_comment(clang::SourceRange range)
{
    cool::csabase::Debug d("do_comment");
    this->onComment(range);
}

void
cool::csabase::PPObserver::do_context()
{
    cool::csabase::Debug d("do_context");
    this->onContext();
}

// -----------------------------------------------------------------------------

void
cool::csabase::PPObserver::FileChanged(
                                 clang::SourceLocation                location,
                                 clang::PPCallbacks::FileChangeReason reason,
                                 clang::SrcMgr::CharacteristicKind    kind,
                                 clang::FileID                        prev)
{
    if (this->connected_)
    {
        switch (reason)
        {
        case clang::PPCallbacks::EnterFile:
            {
                std::string file(this->get_file(location));
                this->do_open_file(location, this->files_.empty()? std::string(): this->files_.top(), file);
                this->files_.push(file);
            }
            break;
        case clang::PPCallbacks::ExitFile:
            {
                std::string file(this->files_.top());
                this->files_.pop();
                this->do_close_file(location, this->files_.empty()? std::string(): this->files_.top(), file);
            }
            break;
        default:
            this->do_other_file(this->get_file(location), reason);
            break;
        }
    }
}

void
cool::csabase::PPObserver::EndOfMainFile()
{
    if (this->connected_)
    {
        std::string file(this->files_.top());
        this->files_.pop();
        this->do_close_file(clang::SourceLocation(), this->files_.empty()? std::string(): this->files_.top(), file);
    }
}

// -----------------------------------------------------------------------------

void
cool::csabase::PPObserver::FileSkipped(clang::FileEntry const& file, clang::Token const&,
                             clang::SrcMgr::CharacteristicKind kind)
{
    this->do_skip_file(this->files_.empty()? std::string(): this->files_.top(), file.getName());
}

// -----------------------------------------------------------------------------

bool
cool::csabase::PPObserver::FileNotFound(llvm::StringRef name,
                                        llvm::SmallVectorImpl<char>& path)
{
    this->do_file_not_found(name);
    return false;
}

// -----------------------------------------------------------------------------

void
cool::csabase::PPObserver::Ident(clang::SourceLocation location, std::string const& ident)
{
    this->do_ident(location, ident);
}

// -----------------------------------------------------------------------------

void
cool::csabase::PPObserver::PragmaDirective(clang::SourceLocation location, clang::PragmaIntroducerKind introducer)
{
}

void
cool::csabase::PPObserver::PragmaComment(clang::SourceLocation location, clang::IdentifierInfo const*, std::string const& value)
{
    this->do_pragma(location, value);
}

void
cool::csabase::PPObserver::PragmaDetectMismatch(clang::SourceLocation  loc,
                                                const std::string     &name,
                                                const std::string     &value)
{
}

void
cool::csabase::PPObserver::PragmaDebug(clang::SourceLocation loc,
                                       llvm::StringRef       debugtype)
{
}

void
cool::csabase::PPObserver::PragmaDiagnosticPush(
                                               clang::SourceLocation loc,
                                               llvm::StringRef       nmspc)
{
}

void
cool::csabase::PPObserver::PragmaDiagnosticPop(clang::SourceLocation loc,
                                               llvm::StringRef       nmspc)
{
}

void
cool::csabase::PPObserver::PragmaDiagnostic(clang::SourceLocation loc,
                                            llvm::StringRef       nmspc,
                                            clang::diag::Mapping  mapping,
                                            llvm::StringRef       str)
{
}

void
cool::csabase::PPObserver::PragmaOpenCLExtension(
                                         clang::SourceLocation        nameloc,
                                         const clang::IdentifierInfo *name,
                                         clang::SourceLocation        stateloc,
                                         unsigned                     state)
{
}

void
cool::csabase::PPObserver::PragmaWarning(clang::SourceLocation loc,
                                         llvm::StringRef       warningspec,
                                         llvm::ArrayRef<int>   ids)
{
}

void
cool::csabase::PPObserver::PragmaWarningPush(clang::SourceLocation loc,
                                             int                   level)
{
}

void
cool::csabase::PPObserver::PragmaWarningPop(clang::SourceLocation loc)
{
}

void
cool::csabase::PPObserver::PragmaMessage(clang::SourceLocation location,
                                         llvm::StringRef       nmspc,
                                         PragmaMessageKind     kind,
                                         llvm::StringRef       value)
{
    this->do_pragma(location, value);
}

// -----------------------------------------------------------------------------

void
cool::csabase::PPObserver::MacroExpands(clang::Token const&          token,
                                        const clang::MacroDirective *macro,
                                        clang::SourceRange           range,
                                        const clang::MacroArgs      *args)
{
    this->do_macro_expands(token, macro);
}

void
cool::csabase::PPObserver::MacroDefined(clang::Token const&          token,
                                        const clang::MacroDirective *macro)
{
    this->do_macro_defined(token, macro);
}

void
cool::csabase::PPObserver::MacroUndefined(clang::Token const&          token,
                                          const clang::MacroDirective *macro)
{
    this->do_macro_undefined(token, macro);
}

void
cool::csabase::PPObserver::Defined(const clang::Token&          token,
                                   const clang::MacroDirective *macro,
                                   clang::SourceRange           range)
{
}

void
cool::csabase::PPObserver::SourceRangeSkipped(clang::SourceRange range)
{
}

// ----------------------------------------------------------------------------

void
cool::csabase::PPObserver::If(clang::SourceLocation loc,
                              clang::SourceRange    range,
                              bool                  conditionvalue)
{
    this->do_if(loc, range);
}

void
cool::csabase::PPObserver::Elif(clang::SourceLocation loc,
                                clang::SourceRange    range,
                                bool                  conditionvalue,
                                clang::SourceLocation ifloc)
{
    this->do_elif(loc, range);
}

void
cool::csabase::PPObserver::Ifdef(clang::SourceLocation        loc,
                                 clang::Token const&          token,
                                 const clang::MacroDirective *md)
{
    this->do_ifdef(loc, token);
}

void
cool::csabase::PPObserver::Ifndef(clang::SourceLocation        loc,
                                  clang::Token const&          token,
                                  const clang::MacroDirective *md)
{
    this->do_ifndef(loc, token);
}

void
cool::csabase::PPObserver::Else(clang::SourceLocation loc,
                                clang::SourceLocation ifloc)
{
    this->do_else(loc, ifloc);
}

void
cool::csabase::PPObserver::Endif(clang::SourceLocation loc,
                                 clang::SourceLocation ifloc)
{
    this->do_endif(loc, ifloc);
}

// ----------------------------------------------------------------------------

void
cool::csabase::PPObserver::HandleComment(clang::SourceRange range)
{
    this->do_comment(range);
}

void
cool::csabase::PPObserver::Context()
{
    this->do_context();
}

void
cool::csabase::PPObserver::InclusionDirective(
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
    this->do_include_file(HashLoc, IsAngled, FileName);
    //-dk:TODO make constructive use of this...
}

void
cool::csabase::PPObserver::moduleImport(clang::SourceLocation  ImportLoc,
                                        clang::ModuleIdPath    Path,
                                        const clang::Module   *Imported)
{
}
