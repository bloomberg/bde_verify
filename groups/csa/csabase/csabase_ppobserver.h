// csabase_ppobserver.h                                               -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de
// Distributed under the Boost Software License, Version 1.0. (See file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).
// -----------------------------------------------------------------------------

#if !defined(INCLUDED_CSABASE_PPOSERVER)
#define INCLUDED_CSABASE_PPOSERVER 1
#ident "$Id$"

#include <utils/event.hpp>
#include <csabase_config.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Preprocessor.h>
#include <string>
#include <stack>

// -----------------------------------------------------------------------------

namespace bde_verify
{
    namespace csabase
    {
        class PPObserver;
    }
}

// -----------------------------------------------------------------------------

class bde_verify::csabase::PPObserver
    : public clang::PPCallbacks
{
public:
    PPObserver(clang::SourceManager const*, bde_verify::csabase::Config*);
    ~PPObserver();
    void detach();
    clang::CommentHandler* get_comment_handler();

    utils::event<void(clang::SourceLocation, bool, std::string const&)>               onInclude;
    utils::event<void(clang::SourceLocation, std::string const&, std::string const&)> onOpenFile;
    utils::event<void(clang::SourceLocation, std::string const&, std::string const&)> onCloseFile;
    utils::event<void(std::string const&, std::string const&)>                        onSkipFile;
    utils::event<void(std::string const&)>                                            onFileNotFound;
    utils::event<void(std::string const&, clang::PPCallbacks::FileChangeReason)>      onOtherFile;
    utils::event<void(clang::SourceLocation, std::string const&)>                     onIdent;
    utils::event<void(clang::SourceLocation, std::string const&)>                     onPragma;
    utils::event<void(clang::Token const&, clang::MacroDirective const*)>                  onMacroExpands;
    utils::event<void(clang::Token const&, clang::MacroDirective const*)>                  onMacroDefined;
    utils::event<void(clang::Token const&, clang::MacroDirective const*)>                  onMacroUndefined;
    utils::event<void(clang::SourceLocation, clang::SourceRange)>                     onIf;
    utils::event<void(clang::SourceLocation, clang::SourceRange)>                     onElif;
    utils::event<void(clang::SourceLocation, clang::Token const&)>                    onIfdef;
    utils::event<void(clang::SourceLocation, clang::Token const&)>                    onIfndef;
    utils::event<void(clang::SourceLocation, clang::SourceLocation)>                  onElse;
    utils::event<void(clang::SourceLocation, clang::SourceLocation)>                  onEndif;
    utils::event<void(clang::SourceRange)>                                            onComment;
    utils::event<void()>                                                              onContext;

    void FileChanged(
                  clang::SourceLocation             Loc,
                  FileChangeReason                  Reason,
                  clang::SrcMgr::CharacteristicKind FileType,
                  clang::FileID                     PrevFID = clang::FileID())
    override;

    void FileSkipped(const clang::FileEntry            &ParentFile,
                     const clang::Token                &FilenameTok,
                     clang::SrcMgr::CharacteristicKind  FileType)
    override;

    bool FileNotFound(llvm::StringRef              FileName,
                      llvm::SmallVectorImpl<char> &RecoveryPath)
    override;

    void InclusionDirective(clang::SourceLocation   HashLoc,
                            const clang::Token&     IncludeTok,
                            llvm::StringRef         FileName,
                            bool                    IsAngled,
                            clang::CharSourceRange  FilenameRange,
                            const clang::FileEntry *File,
                            llvm::StringRef         SearchPath,
                            llvm::StringRef         RelativePath,
                            const clang::Module    *Imported)
    override;

    void moduleImport(clang::SourceLocation  ImportLoc,
                      clang::ModuleIdPath    Path,
                      const clang::Module   *Imported)
    override;

    void EndOfMainFile()
    override;

    void Ident(clang::SourceLocation Loc, const std::string &Str)
    override;

    virtual void PragmaDirective(clang::SourceLocation       Loc,
                                 clang::PragmaIntroducerKind Introducer)
    override;

    void PragmaComment(clang::SourceLocation         Loc,
                       const clang::IdentifierInfo  *Kind,
                       const std::string&            Str)
    override;

    void PragmaDetectMismatch(clang::SourceLocation     Loc,
                              const std::string        &Name,
                              const std::string        &Value)
    override;

    void PragmaDebug(clang::SourceLocation Loc,
                     llvm::StringRef       DebugType)
    override;

    void PragmaMessage(clang::SourceLocation Loc,
                       llvm::StringRef       Namespace,
                       PragmaMessageKind     Kind,
                       llvm::StringRef       Str)
    override;

    void PragmaDiagnosticPush(clang::SourceLocation Loc,
                              llvm::StringRef       Namespace)
    override;

    void PragmaDiagnosticPop(clang::SourceLocation Loc,
                             llvm::StringRef       Namespace)
    override;

    void PragmaDiagnostic(clang::SourceLocation Loc,
                          llvm::StringRef       Namespace,
                          clang::diag::Mapping  Mapping,
                          llvm::StringRef       Str)
    override;

    void PragmaOpenCLExtension(clang::SourceLocation        NameLoc,
                               const clang::IdentifierInfo *Name,
                               clang::SourceLocation        StateLoc,
                               unsigned                     State)
    override;

    void PragmaWarning(clang::SourceLocation Loc,
                       llvm::StringRef       WarningSpec,
                       llvm::ArrayRef<int>   Ids)
    override;

    void PragmaWarningPush(clang::SourceLocation Loc,
                           int                   Level)
    override;

    void PragmaWarningPop(clang::SourceLocation Loc)
    override;

    void MacroExpands(const clang::Token&          MacroNameTok,
                      const clang::MacroDirective *MD,
                      clang::SourceRange           Range,
                      const clang::MacroArgs      *Args)
    override;

    void MacroDefined(const clang::Token&          MacroNameTok,
                      const clang::MacroDirective *MD)
    override;

    void MacroUndefined(const clang::Token&          MacroNameTok,
                        const clang::MacroDirective *MD)
    override;

    void Defined(const clang::Token&          MacroNameTok,
                 const clang::MacroDirective *MD,
                 clang::SourceRange           Range)
    override;

    void SourceRangeSkipped(clang::SourceRange Range)
    override;

    void If(clang::SourceLocation Loc,
            clang::SourceRange    ConditionRange,
            bool                  ConditionValue)
    override;

    void Elif(clang::SourceLocation Loc,
              clang::SourceRange    ConditionRange,
              bool                  ConditionValue,
              clang::SourceLocation IfLoc)
    override;

    void Ifdef(clang::SourceLocation        Loc,
               const clang::Token&          MacroNameTok,
               const clang::MacroDirective *MD)
    override;

    void Ifndef(clang::SourceLocation        Loc,
                const clang::Token&          MacroNameTok,
                const clang::MacroDirective *MD)
    override;

    void Else(clang::SourceLocation Loc, clang::SourceLocation IfLoc)
    override;

    void Endif(clang::SourceLocation Loc, clang::SourceLocation IfLoc)
    override;

    void Context();

    void HandleComment(clang::SourceRange);

private:
    PPObserver(PPObserver const&);
    void operator=(PPObserver const&);

    void do_include_file(clang::SourceLocation, bool, std::string const&);
    void do_open_file(clang::SourceLocation, std::string const&, std::string const&);
    void do_close_file(clang::SourceLocation, std::string const&, std::string const&);
    void do_skip_file(std::string const&, std::string const&);
    void do_file_not_found(std::string const&);
    void do_other_file(std::string const&, clang::PPCallbacks::FileChangeReason);
    void do_ident(clang::SourceLocation, std::string const&);
    void do_pragma(clang::SourceLocation, std::string const&);
    void do_macro_expands(clang::Token const&, clang::MacroDirective const*);
    void do_macro_defined(clang::Token const&, clang::MacroDirective const*);
    void do_macro_undefined(clang::Token const&, clang::MacroDirective const*);
    void do_if(clang::SourceLocation, clang::SourceRange);
    void do_elif(clang::SourceLocation, clang::SourceRange);
    void do_ifdef(clang::SourceLocation, clang::Token const&);
    void do_ifndef(clang::SourceLocation, clang::Token const&);
    void do_else(clang::SourceLocation, clang::SourceLocation);
    void do_endif(clang::SourceLocation, clang::SourceLocation);
    void do_comment(clang::SourceRange);
    void do_context();

    std::string get_file(clang::SourceLocation) const;
    clang::SourceManager const* source_manager_;
    std::stack<std::string>     files_;
    bool                        connected_;
    bde_verify::csabase::Config*      config_;
};

// -----------------------------------------------------------------------------

#endif
