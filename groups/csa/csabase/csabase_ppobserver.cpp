// csabase_ppobserver.cpp                                             -*-C++-*-

#include <csabase_ppobserver.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Preprocessor.h>
#include <csabase_debug.h>
#include <csabase_filenames.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/Regex.h>
#include <csabase_config.h>
#include <utils/event.hpp>

namespace clang
{
    class IdentifierInfo;
}
namespace clang
{
    class MacroArgs;
}
namespace clang
{
    class MacroDirective;
}
namespace clang
{
    class Module;
}
namespace clang
{
    class Token;
}

using namespace csabase;
using namespace clang;

// -----------------------------------------------------------------------------

csabase::PPObserver::PPObserver(SourceManager const *source_manager,
                                Config *config)
    : source_manager_(source_manager), connected_(true), config_(config)
{
}

// -----------------------------------------------------------------------------

csabase::PPObserver::~PPObserver()
{
}

// -----------------------------------------------------------------------------

void csabase::PPObserver::detach()
{
    connected_ = false;
}

// -----------------------------------------------------------------------------

namespace
{
    struct Handler : CommentHandler
    {
        Handler(PPObserver *observer)
            : observer_(observer)
        {
        }

        bool HandleComment(Preprocessor &, SourceRange range)
        {
            observer_->HandleComment(range);
            return false;
        }

        PPObserver *observer_;
    };
} // namespace

CommentHandler *csabase::PPObserver::get_comment_handler()
{
    return new Handler(this);
}

// -----------------------------------------------------------------------------

std::string csabase::PPObserver::get_file(SourceLocation location) const
{
    return source_manager_->getPresumedLoc(location).getFilename();
}

// -----------------------------------------------------------------------------

void csabase::PPObserver::do_include_file(SourceLocation location,
                                          bool is_angled,
                                          std::string const &file)
{
    std::string msg("do_include_file '" + file + "' angled=" +
                    (is_angled ? "true" : "false"));
    Debug d(msg.c_str());
    onInclude(location, is_angled, file);
}

void csabase::PPObserver::do_open_file(SourceLocation location,
                                       std::string const &from,
                                       std::string const &file)
{
    std::string msg("do_open_file '" + file + "'");
    Debug d(msg.c_str());
    onOpenFile(location, from, file);
}

void csabase::PPObserver::do_close_file(SourceLocation location,
                                        std::string const &from,
                                        std::string const &file)
{
    std::string msg("do_close_file '" + file + "'");
    Debug d(msg.c_str());
    onCloseFile(location, from, file);
}

void csabase::PPObserver::do_skip_file(std::string const &from,
                                       std::string const &file)
{
    std::string msg("do_skip_file(" + from + ", " + file + ")");
    Debug d(msg.c_str());
    onSkipFile(from, file);
}

void csabase::PPObserver::do_file_not_found(std::string const &file)
{
    std::string msg("do_file_not_found(" + file + ")");
    Debug d(msg.c_str());
    onFileNotFound(file);
}

void csabase::PPObserver::do_other_file(std::string const &file,
                                        PPCallbacks::FileChangeReason reason)
{
    std::string msg("do_other_file '" + file + "'");
    Debug d(msg.c_str());
    onOtherFile(file, reason);
}

void csabase::PPObserver::do_ident(SourceLocation location,
                                   std::string const &ident)
{
    Debug d("do_ident");
    onIdent(location, ident);
}

void csabase::PPObserver::do_pragma(SourceLocation location,
                                    std::string const &value)
{
    Debug d("do_pragma");
    onPragma(location, value);
}

void csabase::PPObserver::do_macro_expands(Token const &token,
                                           const MacroDirective *macro,
                                           SourceRange range,
                                           MacroArgs const *args)
{
    Debug d("do_macro_expands");
    onMacroExpands(token, macro, range, args);
}

void csabase::PPObserver::do_macro_defined(Token const &token,
                                           const MacroDirective *macro)
{
    Debug d("do_macro_defined");
    onMacroDefined(token, macro);
}

void csabase::PPObserver::do_macro_undefined(Token const &token,
                                             const MacroDirective *macro)
{
    Debug d("do_macro_undefined");
    onMacroUndefined(token, macro);
}

void csabase::PPObserver::do_if(SourceLocation where, SourceRange range)
{
    Debug d("do_if");
    onIf(where, range);
}

void csabase::PPObserver::do_elif(SourceLocation where, SourceRange range)
{
    Debug d("do_elif");
    onElif(where, range);
}

void csabase::PPObserver::do_ifdef(SourceLocation where, Token const &token)
{
    Debug d("do_ifdef");
    onIfdef(where, token);
}

void csabase::PPObserver::do_ifndef(SourceLocation where, Token const &token)
{
    Debug d("do_ifndef");
    onIfndef(where, token);
}

void csabase::PPObserver::do_else(SourceLocation where, SourceLocation what)
{
    Debug d("do_else");
    onElse(where, what);
}

void csabase::PPObserver::do_endif(SourceLocation where, SourceLocation what)
{
    Debug d("do_endif");
    onEndif(where, what);
}

void csabase::PPObserver::do_context()
{
    Debug d("do_context");
    onContext();
}

// -----------------------------------------------------------------------------

void csabase::PPObserver::FileChanged(SourceLocation location,
                                      PPCallbacks::FileChangeReason reason,
                                      SrcMgr::CharacteristicKind kind,
                                      FileID prev)
{
    if (connected_)
    {
        onPPFileChanged(location, reason, kind, prev);

        switch (reason)
        {
        case PPCallbacks::EnterFile:
        {
            std::string file(get_file(location));
            do_open_file(location,
                         files_.empty() ? std::string() : files_.top(),
                         file);
            files_.push(file);
        }
        break;
        case PPCallbacks::ExitFile:
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

void csabase::PPObserver::EndOfMainFile()
{
    if (connected_)
    {
        onPPEndOfMainFile();
    }
}

// -----------------------------------------------------------------------------

void csabase::PPObserver::FileSkipped(FileEntryRef const &file,
                                      Token const &token,
                                      SrcMgr::CharacteristicKind kind)
{
    onPPFileSkipped(file, token, kind);

    do_skip_file(files_.empty() ? std::string() : files_.top(), file.getName().str());
}

// -----------------------------------------------------------------------------

bool csabase::PPObserver::FileNotFound(llvm::StringRef name,
                                       llvm::SmallVectorImpl<char> &path)
{
    onPPFileNotFound(name, path);

    do_file_not_found(name.str());
    return false;
}

// -----------------------------------------------------------------------------

void csabase::PPObserver::Ident(SourceLocation location, llvm::StringRef ident)
{
    onPPIdent(location, ident);

    do_ident(location, ident.str());
}

// -----------------------------------------------------------------------------

static llvm::Regex pragma_bdeverify(
    "^ *# *pragma +b[bd]e?_?verify +"
    "(" // 1
    "("
    "push"
    ")|" // 2
    "("
    "pop"
    ")|" // 3
    "("
    "[-] *([[:alnum:]]+|[*])"
    ")|" // 4 5
    "("
    "[+] *([[:alnum:]]+|[*])"
    ")|" // 6 7
    "("
    "set +([_[:alnum:]]+) *(.*[^ ])"
    ")|" // 8 9 10
    "("
    "append +([_[:alnum:]]+) *(.*[^ ])"
    ")|" // 11 12 13
    "("
    "prepend +([_[:alnum:]]+) *(.*[^ ])"
    ")|" // 14 15 16
    "("
    "re-exports? +[<\"]?([_./[:alnum:]]+)[\">]?"
    ")|" // 17 18
    "\r*$"
    ")",
    llvm::Regex::NoFlags);

static llvm::Regex comment_bdeverify(
    "^ *// *B[BD]E?_?VERIFY +pragma *: *"
    "(" // 1
    "("
    "push"
    ")|" // 2
    "("
    "pop"
    ")|" // 3
    "("
    "[-] *([[:alnum:]]+|[*])"
    ")|" // 4 5
    "("
    "[+] *([[:alnum:]]+|[*])"
    ")|" // 6 7
    "("
    "set +([_[:alnum:]]+) *(.*[^ ])"
    ")|" // 8 9 10
    "("
    "append +([_[:alnum:]]+) *(.*[^ ])"
    ")|" // 11 12 13
    "("
    "prepend +([_[:alnum:]]+) *(.*[^ ])"
    ")|" // 14 15 16
    "("
    "re-exports? +[<\"]?([_./[:alnum:]]+)[\">]?"
    ")|" // 17 18
    "\r*$"
    ")",
    llvm::Regex::NoFlags);

static bool handle_bv_pragma(const SourceManager &m,
                             Config *c,
                             SourceLocation l,
                             llvm::Regex &r)
{
    FileID fid = m.getFileID(l);
    unsigned line = m.getPresumedLineNumber(l);
    llvm::StringRef directive =
        m.getBufferData(fid).slice(
            m.getFileOffset(m.translateLineCol(fid, line, 1u)),
            m.getFileOffset(m.translateLineCol(fid, line, ~0u)));
    llvm::SmallVector<llvm::StringRef, 20> matches;
    if (r.match(directive, &matches))
    {
        if (!matches[2].empty())
        { // push
            c->push_suppress(l);
        }
        else if (!matches[3].empty())
        { // pop
            c->pop_suppress(l);
        }
        else if (!matches[5].empty())
        { // suppress
            c->suppress(matches[5].str(), l, true);
        }
        else if (!matches[7].empty())
        { // unsuppress
            c->suppress(matches[7].str(), l, false);
        }
        else if (!matches[8].empty())
        { // set
            c->set_bv_value(l, matches[9].str(), matches[10].str());
        }
        else if (!matches[11].empty())
        { // append
            std::string old = c->value(matches[12].str(), l);
            std::string sp = old.size() > 0 ? " " : "";
            c->set_bv_value(l, matches[12].str(), old + sp + matches[13].str());
        }
        else if (!matches[14].empty())
        { // prepend
            std::string old = c->value(matches[15].str(), l);
            std::string sp = old.size() > 0 ? " " : "";
            c->set_bv_value(l, matches[15].str(), matches[16].str() + old + sp);
        }
        else if (!matches[17].empty())
        {
            FileName including_file(m.getFilename(l));
            FileName reexported_file(matches[18]);
            c->set_reexports(including_file.name().str(), reexported_file.name().str());
        }
        return true; // RETURN
    }
    return false;
}

void csabase::PPObserver::do_comment(SourceRange range)
{
    Debug d("do_comment");

    if (!handle_bv_pragma(
            *source_manager_, config_, range.getBegin(), comment_bdeverify))
    {
        onComment(range);
    }
}

void csabase::PPObserver::PragmaDirective(SourceLocation location,
                                          PragmaIntroducerKind introducer)
{
    if (!handle_bv_pragma(
            *source_manager_, config_, location, pragma_bdeverify))
    {
        onPPPragmaDirective(location, introducer);
    }
}

void csabase::PPObserver::PragmaComment(SourceLocation location,
                                        IdentifierInfo const *id,
                                        llvm::StringRef value)
{
    onPPPragmaComment(location, id, value);

    do_pragma(location, value.str());
}

void csabase::PPObserver::PragmaDetectMismatch(SourceLocation loc,
                                               llvm::StringRef name,
                                               llvm::StringRef value)
{
    onPPPragmaDetectMismatch(loc, name, value);
}

void csabase::PPObserver::PragmaDebug(SourceLocation loc, llvm::StringRef debugtype)
{
    onPPPragmaDebug(loc, debugtype);
}

void csabase::PPObserver::PragmaDiagnosticPush(SourceLocation loc,
                                               llvm::StringRef nmspc)
{
    onPPPragmaDiagnosticPush(loc, nmspc);
}

void csabase::PPObserver::PragmaDiagnosticPop(SourceLocation loc,
                                              llvm::StringRef nmspc)
{
    onPPPragmaDiagnosticPop(loc, nmspc);
}

void csabase::PPObserver::PragmaDiagnostic(SourceLocation loc,
                                           llvm::StringRef nmspc,
                                           diag::Severity mapping,
                                           llvm::StringRef str)
{
    onPPPragmaDiagnostic(loc, nmspc, mapping, str);
}

void csabase::PPObserver::PragmaOpenCLExtension(SourceLocation nameloc,
                                                const IdentifierInfo *name,
                                                SourceLocation stateloc,
                                                unsigned state)
{
    onPPPragmaOpenCLExtension(nameloc, name, stateloc, state);
}

void csabase::PPObserver::PragmaWarning(SourceLocation loc,
                                        llvm::StringRef warningspec,
                                        llvm::ArrayRef<int> ids)
{
    onPPPragmaWarning(loc, warningspec, ids);
}

void csabase::PPObserver::PragmaWarningPush(SourceLocation loc, int level)
{
    onPPPragmaWarningPush(loc, level);
}

void csabase::PPObserver::PragmaWarningPop(SourceLocation loc)
{
    onPPPragmaWarningPop(loc);
}

void csabase::PPObserver::PragmaMessage(SourceLocation location,
                                        llvm::StringRef nmspc,
                                        PragmaMessageKind kind,
                                        llvm::StringRef value)
{
    onPPPragmaMessage(location, nmspc, kind, value);

    do_pragma(location, value.str());
}

// -----------------------------------------------------------------------------

void csabase::PPObserver::MacroExpands(Token const &token,
                                       const MacroDefinition &macro,
                                       SourceRange range,
                                       const MacroArgs *args)
{
    onPPMacroExpands(token, macro, range, args);

    do_macro_expands(token, macro.getLocalDirective(), range, args);
}

void csabase::PPObserver::MacroDefined(Token const &token,
                                       const MacroDirective *macro)
{
    onPPMacroDefined(token, macro);

    do_macro_defined(token, macro);
}

void csabase::PPObserver::MacroUndefined(Token const &token,
                                         const MacroDefinition &macro,
                                         const MacroDirective *undef)
{
    onPPMacroUndefined(token, macro, undef);

    do_macro_undefined(token, undef);
}

void csabase::PPObserver::Defined(const Token &token,
                                  const MacroDefinition &macro,
                                  SourceRange range)
{
    onPPDefined(token, macro, range);
}

void csabase::PPObserver::SourceRangeSkipped(SourceRange range,
                                             SourceLocation endifLoc)
{
    onPPSourceRangeSkipped(range, endifLoc);
}

// ----------------------------------------------------------------------------

void csabase::PPObserver::If(SourceLocation loc,
                             SourceRange range,
                             ConditionValueKind conditionvalue)
{
    onPPIf(loc, range, conditionvalue);

    do_if(loc, range);
}

void csabase::PPObserver::Elif(SourceLocation loc,
                               SourceRange range,
                               ConditionValueKind conditionvalue,
                               SourceLocation ifloc)
{
    onPPElif(loc, range, conditionvalue, ifloc);

    do_elif(loc, range);
}

void csabase::PPObserver::Ifdef(SourceLocation loc,
                                Token const &token,
                                const MacroDefinition &md)
{
    onPPIfdef(loc, token, md);

    do_ifdef(loc, token);
}

void csabase::PPObserver::Ifndef(SourceLocation loc,
                                 Token const &token,
                                 const MacroDefinition &md)
{
    onPPIfndef(loc, token, md);

    do_ifndef(loc, token);
}

void csabase::PPObserver::Else(SourceLocation loc, SourceLocation ifloc)
{
    onPPElse(loc, ifloc);

    do_else(loc, ifloc);
}

void csabase::PPObserver::Endif(SourceLocation loc, SourceLocation ifloc)
{
    onPPEndif(loc, ifloc);

    do_endif(loc, ifloc);
}

// ----------------------------------------------------------------------------

void csabase::PPObserver::HandleComment(SourceRange range)
{
    do_comment(range);
}

void csabase::PPObserver::Context()
{
    do_context();
}

void csabase::PPObserver::InclusionDirective(
    SourceLocation HashLoc,
    const Token &IncludeTok,
    llvm::StringRef FileName,
    bool IsAngled,
    CharSourceRange FilenameRange,
    const FileEntry *File,
    llvm::StringRef SearchPath,
    llvm::StringRef RelativePath,
    const Module *Imported,
    SrcMgr::CharacteristicKind FileType)
{
    onPPInclusionDirective(HashLoc, IncludeTok, FileName, IsAngled,
                           FilenameRange, File, SearchPath, RelativePath,
                           Imported, FileType);

    do_include_file(HashLoc, IsAngled, FileName.str());
    //-dk:TODO make constructive use of this...
}

void csabase::PPObserver::moduleImport(SourceLocation ImportLoc,
                                       ModuleIdPath Path,
                                       const Module *Imported)
{
    onPPmoduleImport(ImportLoc, Path, Imported);
}

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
