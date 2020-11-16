// csabase_ppobserver.h                                               -*-C++-*-

#ifndef INCLUDED_CSABASE_PPOSERVER
#define INCLUDED_CSABASE_PPOSERVER

#include <clang/Basic/DiagnosticIDs.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/ModuleLoader.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Pragma.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <stack>
#include <string>
#include <utils/event.hpp>
namespace clang { class CommentHandler; }
namespace clang { class FileEntry; }
namespace clang { class IdentifierInfo; }
namespace clang { class MacroArgs; }
namespace clang { class MacroDirective; }
namespace clang { class Module; }
namespace clang { class Token; }
namespace csabase { class Config; }
namespace llvm { template <typename T> class SmallVectorImpl; }

// -----------------------------------------------------------------------------

namespace csabase
{
class PPObserver : public clang::PPCallbacks
{
    typedef clang::PPCallbacks Base;

public:
    PPObserver(clang::SourceManager const*, Config*);
    ~PPObserver();
    void detach();
    clang::CommentHandler* get_comment_handler();

    utils::event<void(clang::SourceLocation, bool, std::string const&)>               onInclude;
    utils::event<
        void(clang::SourceLocation, std::string const &, std::string const &)>
    onOpenFile;
    utils::event<
        void(clang::SourceLocation, std::string const &, std::string const &)>
    onCloseFile;
    utils::event<void(std::string const&, std::string const&)>                        onSkipFile;
    utils::event<void(std::string const&)>                                            onFileNotFound;
    utils::event<void(std::string const &,
                      clang::PPCallbacks::FileChangeReason)> onOtherFile;
    utils::event<void(clang::SourceLocation, std::string const&)>                     onIdent;
    utils::event<void(clang::SourceLocation, std::string const&)>                     onPragma;
    utils::event<void(clang::Token const &,
                      clang::MacroDirective const *,
                      clang::SourceRange,
                      clang::MacroArgs const*)> onMacroExpands;
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

    enum CallbackType {
        e_None,
        e_FileChanged,
        e_FileSkipped,
        e_FileNotFound,
        e_InclusionDirective,
        e_moduleImport,
        e_EndOfMainFile,
        e_Ident,
        e_PragmaDirective,
        e_PragmaComment,
        e_PragmaDetectMismatch,
        e_PragmaDebug,
        e_PragmaMessage,
        e_PragmaDiagnosticPush,
        e_PragmaDiagnosticPop,
        e_PragmaDiagnostic,
        e_PragmaOpenCLExtension,
        e_PragmaWarning,
        e_PragmaWarningPush,
        e_PragmaWarningPop,
        e_MacroExpands,
        e_MacroDefined,
        e_MacroUndefined,
        e_Defined,
        e_SourceRangeSkipped,
        e_If,
        e_Elif,
        e_Ifdef,
        e_Ifndef,
        e_Else,
        e_Endif,

        e_Comment,
    };

    void FileChanged(clang::SourceLocation             Loc,
                     FileChangeReason                  Reason,
                     clang::SrcMgr::CharacteristicKind FileType,
                     clang::FileID                     PrevFID)
    override;

    utils::event<decltype(&Base::FileChanged)> onPPFileChanged;

    void FileSkipped(const clang::FileEntryRef         &ParentFile,
                     const clang::Token                &FilenameTok,
                     clang::SrcMgr::CharacteristicKind  FileType)
    override;

    utils::event<decltype(&Base::FileSkipped)> onPPFileSkipped;

    bool FileNotFound(llvm::StringRef              FileName,
                      llvm::SmallVectorImpl<char> &RecoveryPath)
    override;

    utils::event<decltype(&Base::FileNotFound)> onPPFileNotFound;

    void InclusionDirective(
                         clang::SourceLocation              HashLoc,
                         const clang::Token&                IncludeTok,
                         llvm::StringRef                    FileName,
                         bool                               IsAngled,
                         clang::CharSourceRange             FilenameRange,
                         const clang::FileEntry            *File,
                         llvm::StringRef                    SearchPath,
                         llvm::StringRef                    RelativePath,
                         const clang::Module               *Imported,
                         clang::SrcMgr::CharacteristicKind  FileType) override;

    utils::event<decltype(&Base::InclusionDirective)> onPPInclusionDirective;

    void moduleImport(clang::SourceLocation  ImportLoc,
                      clang::ModuleIdPath    Path,
                      const clang::Module   *Imported)
    override;

    utils::event<decltype(&Base::moduleImport)> onPPmoduleImport;

    void EndOfMainFile()
    override;

    utils::event<decltype(&Base::EndOfMainFile)> onPPEndOfMainFile;

    void Ident(clang::SourceLocation Loc, llvm::StringRef Str)
    override;

    utils::event<decltype(&Base::Ident)> onPPIdent;

    virtual void PragmaDirective(clang::SourceLocation       Loc,
                                 clang::PragmaIntroducerKind Introducer)
    override;

    utils::event<decltype(&Base::PragmaDirective)> onPPPragmaDirective;

    void PragmaComment(clang::SourceLocation         Loc,
                       const clang::IdentifierInfo  *Kind,
                       llvm::StringRef               Str)
    override;

    utils::event<decltype(&Base::PragmaComment)> onPPPragmaComment;

    void PragmaDetectMismatch(clang::SourceLocation Loc,
                              llvm::StringRef       Name,
                              llvm::StringRef       Value)
    override;

    utils::event<decltype(&Base::PragmaDetectMismatch)>
                                                      onPPPragmaDetectMismatch;

    void PragmaDebug(clang::SourceLocation Loc,
                     llvm::StringRef       DebugType)
    override;

    utils::event<decltype(&Base::PragmaDebug)> onPPPragmaDebug;

    void PragmaMessage(clang::SourceLocation Loc,
                       llvm::StringRef       Namespace,
                       PragmaMessageKind     Kind,
                       llvm::StringRef       Str)
    override;

    utils::event<decltype(&Base::PragmaMessage)> onPPPragmaMessage;

    void PragmaDiagnosticPush(clang::SourceLocation Loc,
                              llvm::StringRef       Namespace)
    override;

    utils::event<decltype(&Base::PragmaDiagnosticPush)>
                                                      onPPPragmaDiagnosticPush;

    void PragmaDiagnosticPop(clang::SourceLocation Loc,
                             llvm::StringRef       Namespace)
    override;

    utils::event<decltype(&Base::PragmaDiagnosticPop)> onPPPragmaDiagnosticPop;

    void PragmaDiagnostic(clang::SourceLocation  Loc,
                          llvm::StringRef        Namespace,
                          clang::diag::Severity  Mapping,
                          llvm::StringRef        Str)
    override;

    utils::event<decltype(&Base::PragmaDiagnostic)> onPPPragmaDiagnostic;

    void PragmaOpenCLExtension(clang::SourceLocation        NameLoc,
                               const clang::IdentifierInfo *Name,
                               clang::SourceLocation        StateLoc,
                               unsigned                     State)
    override;

    utils::event<decltype(&Base::PragmaOpenCLExtension)>
                                                     onPPPragmaOpenCLExtension;

    void PragmaWarning(clang::SourceLocation Loc,
                       llvm::StringRef       WarningSpec,
                       llvm::ArrayRef<int>   Ids)
    override;

    utils::event<decltype(&Base::PragmaWarning)> onPPPragmaWarning;

    void PragmaWarningPush(clang::SourceLocation Loc,
                           int                   Level)
    override;

    utils::event<decltype(&Base::PragmaWarningPush)> onPPPragmaWarningPush;

    void PragmaWarningPop(clang::SourceLocation Loc)
    override;

    utils::event<decltype(&Base::PragmaWarningPop)> onPPPragmaWarningPop;

    void MacroExpands(const clang::Token&            MacroNameTok,
                      const clang::MacroDefinition&  MD,
                      clang::SourceRange             Range,
                      const clang::MacroArgs        *Args)
    override;

    utils::event<decltype(&Base::MacroExpands)> onPPMacroExpands;

    void MacroDefined(const clang::Token&          MacroNameTok,
                      const clang::MacroDirective *MD)
    override;

    utils::event<decltype(&Base::MacroDefined)> onPPMacroDefined;

    void MacroUndefined(const clang::Token&            MacroNameTok,
                        const clang::MacroDefinition&  MD,
                        const clang::MacroDirective   *Undef)
    override;

    utils::event<decltype(&Base::MacroUndefined)> onPPMacroUndefined;

    void Defined(const clang::Token&           MacroNameTok,
                 const clang::MacroDefinition& MD,
                 clang::SourceRange            Range)
    override;

    utils::event<decltype(&Base::Defined)> onPPDefined;

    void SourceRangeSkipped(clang::SourceRange    Range,
                            clang::SourceLocation EndifLoc) override;

    utils::event<decltype(&Base::SourceRangeSkipped)> onPPSourceRangeSkipped;

    void If(clang::SourceLocation Loc,
            clang::SourceRange    ConditionRange,
            ConditionValueKind    ConditionValue)
    override;

    utils::event<decltype(&Base::If)> onPPIf;

    void Elif(clang::SourceLocation Loc,
              clang::SourceRange    ConditionRange,
              ConditionValueKind    ConditionValue,
              clang::SourceLocation IfLoc)
    override;

    utils::event<decltype(&Base::Elif)> onPPElif;

    void Ifdef(clang::SourceLocation         Loc,
               const clang::Token&           MacroNameTok,
               const clang::MacroDefinition& MD)
    override;

    utils::event<decltype(&Base::Ifdef)> onPPIfdef;

    void Ifndef(clang::SourceLocation         Loc,
                const clang::Token&           MacroNameTok,
                const clang::MacroDefinition& MD)
    override;

    utils::event<decltype(&Base::Ifndef)> onPPIfndef;

    void Else(clang::SourceLocation Loc, clang::SourceLocation IfLoc)
    override;

    utils::event<decltype(&Base::Else)> onPPElse;

    void Endif(clang::SourceLocation Loc, clang::SourceLocation IfLoc)
    override;

    utils::event<decltype(&Base::Endif)> onPPEndif;

    void Context();

    void HandleComment(clang::SourceRange);

private:
    PPObserver(PPObserver const&);
    void operator=(PPObserver const&);

    void do_include_file(clang::SourceLocation, bool, std::string const&);
    void do_open_file(clang::SourceLocation,
                      std::string const &,
                      std::string const &);
    void do_close_file(clang::SourceLocation,
                       std::string const &,
                       std::string const &);
    void do_skip_file(std::string const&, std::string const&);
    void do_file_not_found(std::string const&);
    void
    do_other_file(std::string const &, clang::PPCallbacks::FileChangeReason);
    void do_ident(clang::SourceLocation, std::string const&);
    void do_pragma(clang::SourceLocation, std::string const&);
    void do_macro_expands(clang::Token const &,
                          clang::MacroDirective const *,
                          clang::SourceRange,
                          clang::MacroArgs const *);
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
    std::stack<std::string> files_;
    bool                    connected_;
    Config*                 config_;
};
}

#endif

// ----------------------------------------------------------------------------
// Copyright (C) 2014 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
