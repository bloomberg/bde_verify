// csabase_ppobserver.h                                               -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(INCLUDED_CSABASE_PPOSERVER)
#define INCLUDED_CSABASE_PPOSERVER 1
#ident "$Id$"

#include <cool/event.hpp>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Preprocessor.h>
#include <string>
#include <stack>

// -----------------------------------------------------------------------------

namespace cool
{
    namespace csabase
    {
        class PPObserver;
    }
}

// -----------------------------------------------------------------------------

class cool::csabase::PPObserver
    : public clang::PPCallbacks
{
public:
    PPObserver(clang::SourceManager const*);
    ~PPObserver();
    void detach();
    clang::CommentHandler* get_comment_handler();

    cool::event<void(clang::SourceLocation, bool, std::string const&)>               onInclude;
    cool::event<void(clang::SourceLocation, std::string const&, std::string const&)> onOpenFile;
    cool::event<void(clang::SourceLocation, std::string const&, std::string const&)> onCloseFile;
    cool::event<void(std::string const&, std::string const&)>                        onSkipFile;
    cool::event<void(std::string const&)>                                            onFileNotFound;
    cool::event<void(std::string const&, clang::PPCallbacks::FileChangeReason)>      onOtherFile;
    cool::event<void(clang::SourceLocation, std::string const&)>                     onIdent;
    cool::event<void(clang::SourceLocation, std::string const&)>                     onPragma;
    cool::event<void(clang::Token const&, clang::MacroDirective const*)>                  onMacroExpands;
    cool::event<void(clang::Token const&, clang::MacroDirective const*)>                  onMacroDefined;
    cool::event<void(clang::Token const&, clang::MacroDirective const*)>                  onMacroUndefined;
    cool::event<void(clang::SourceLocation, clang::SourceRange)>                     onIf;
    cool::event<void(clang::SourceLocation, clang::SourceRange)>                     onElif;
    cool::event<void(clang::SourceLocation, clang::Token const&)>                    onIfdef;
    cool::event<void(clang::SourceLocation, clang::Token const&)>                    onIfndef;
    cool::event<void(clang::SourceLocation, clang::SourceLocation)>                  onElse;
    cool::event<void(clang::SourceLocation, clang::SourceLocation)>                  onEndif;
    cool::event<void(clang::SourceRange)>                                            onComment;
    cool::event<void()>                                                              onContext;

#if 1
    void FileChanged(
                  clang::SourceLocation             Loc,
                  FileChangeReason                  Reason,
                  clang::SrcMgr::CharacteristicKind FileType,
                  clang::FileID                     PrevFID = clang::FileID());

    void FileSkipped(const clang::FileEntry            &ParentFile,
                     const clang::Token                &FilenameTok,
                     clang::SrcMgr::CharacteristicKind  FileType);

    bool FileNotFound(llvm::StringRef              FileName,
                      llvm::SmallVectorImpl<char> &RecoveryPath);

    void InclusionDirective(clang::SourceLocation   HashLoc,
                            const clang::Token&     IncludeTok,
                            llvm::StringRef         FileName,
                            bool                    IsAngled,
                            clang::CharSourceRange  FilenameRange,
                            const clang::FileEntry *File,
                            llvm::StringRef         SearchPath,
                            llvm::StringRef         RelativePath, 
                            const clang::Module    *Imported);

    void moduleImport(clang::SourceLocation  ImportLoc,
                      clang::ModuleIdPath    Path,
                      const clang::Module   *Imported);

    void EndOfMainFile();

    void Ident(clang::SourceLocation Loc, const std::string &str);

    void PragmaComment(clang::SourceLocation         Loc,
                       const clang::IdentifierInfo  *Kind,
                       const std::string&            Str);

    void PragmaDebug(clang::SourceLocation Loc,
                     llvm::StringRef       DebugType);

    enum PragmaMessageKind { PMK_Message, PMK_Warning, PMK_Error };

    void PragmaMessage(clang::SourceLocation Loc,
                       llvm::StringRef       Namespace,
                       PragmaMessageKind     Kind,
                       llvm::StringRef       Str); 

    void PragmaDiagnosticPush(clang::SourceLocation Loc,
                              llvm::StringRef       Namespace); 

    void PragmaDiagnosticPop(clang::SourceLocation Loc,
                             llvm::StringRef       Namespace); 

    void PragmaDiagnostic(clang::SourceLocation Loc,
                          llvm::StringRef       Namespace,
                          clang::diag::Mapping  Mapping,
                          llvm::StringRef       Str); 

    void MacroExpands(const clang::Token&          MacroNameTok,
                      const clang::MacroDirective *MD,
                      clang::SourceRange           Range,
                      const clang::MacroArgs      *Args); 

    void MacroDefined(const clang::Token&          MacroNameTok,
                      const clang::MacroDirective *MD); 

    void MacroUndefined(const clang::Token&          MacroNameTok,
                        const clang::MacroDirective *MD); 

    void Defined(const clang::Token&          MacroNameTok,
                 const clang::MacroDirective *MD); 

    void SourceRangeSkipped(clang::SourceRange Range); 

    void If(clang::SourceLocation Loc, clang::SourceRange ConditionRange); 

    void Elif(clang::SourceLocation Loc,
              clang::SourceRange    ConditionRange,
              clang::SourceLocation IfLoc); 

    void Ifdef(clang::SourceLocation        Loc,
               const clang::Token&          MacroNameTok,
               const clang::MacroDirective *MD); 

    void Ifndef(clang::SourceLocation        Loc,
                const clang::Token&          MacroNameTok,
                const clang::MacroDirective *MD);

    void Else(clang::SourceLocation Loc, clang::SourceLocation IfLoc);

    void Endif(clang::SourceLocation Loc, clang::SourceLocation IfLoc);
#else
    void FileChanged(clang::SourceLocation, clang::PPCallbacks::FileChangeReason,
                     clang::SrcMgr::CharacteristicKind,
                     clang::FileID);
    void FileChanged(clang::SourceLocation, clang::PPCallbacks::FileChangeReason,
                     clang::SrcMgr::CharacteristicKind);
    void FileSkipped(clang::FileEntry const&, clang::Token const&,
                     clang::SrcMgr::CharacteristicKind);
    void EndOfMainFile();

    void Ident(clang::SourceLocation, std::string const&);
    void PragmaComment(clang::SourceLocation, clang::IdentifierInfo const*, std::string const&);
    void PragmaMessage(clang::SourceLocation, llvm::StringRef);
    void MacroExpands(clang::Token const&, clang::MacroInfo const*);
    void MacroDefined(clang::Token const&, clang::MacroInfo const*);
    void MacroUndefined(clang::Token const&, clang::MacroInfo const*);
    void If(clang::SourceRange);
    void Elif(clang::SourceRange);
    void Ifdef(clang::Token const&);
    void Ifndef(clang::Token const&);
    void Else();
    void Endif();

    void If(clang::SourceLocation, clang::SourceRange);
    void Elif(clang::SourceLocation, clang::SourceRange);
    void Ifdef(clang::SourceLocation, clang::Token const&);
    void Ifndef(clang::SourceLocation, clang::Token const&);
    void Else(clang::SourceLocation, clang::SourceLocation);
    void Endif(clang::SourceLocation, clang::SourceLocation);

    void InclusionDirective(clang::SourceLocation HashLoc,
                            clang::Token const& IncludeTok,
                            llvm::StringRef FileName,
                            bool IsAngled,
                            clang::FileEntry const* File,
                            clang::SourceLocation EndLoc,
                            llvm::StringRef SearchPath,
                            llvm::StringRef RelativePath);
    void InclusionDirective(clang::SourceLocation HashLoc,
                            clang::Token const& IncludeTok,
                            llvm::StringRef FileName,
                            bool IsAngled,
                            clang::FileEntry const* File,
                            clang::SourceLocation EndLoc);
#endif
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
};

// -----------------------------------------------------------------------------

#endif
