// csabase_diagnosticfilter.cpp                                       -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csabase_diagnosticfilter.h>
#include <csabase_analyser.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/FileManager.h>
#include <llvm/Support/raw_ostream.h>
#ident "$Id$"

namespace CB = cool::csabase;

// ----------------------------------------------------------------------------

CB::DiagnosticFilter::DiagnosticFilter(CB::Analyser const& analyser,
                                       clang::DiagnosticOptions const& options)
    : d_options(&options)
    , d_client(new clang::TextDiagnosticPrinter(llvm::errs(), options))
    , d_analyser(&analyser)
{
}

CB::DiagnosticFilter::~DiagnosticFilter()
{
}

// ----------------------------------------------------------------------------

void
CB::DiagnosticFilter::BeginSourceFile(clang::LangOptions const&  opts,
                                      clang::Preprocessor const* pp)
{
    this->d_client->BeginSourceFile(opts, pp);
}

void
CB::DiagnosticFilter::EndSourceFile()
{
    this->d_client->EndSourceFile();
}

bool
CB::DiagnosticFilter::IncludeInDiagnosticCount() const
{
    return true;
}

static std::string
#if !defined(CLANG_29)
get_filename(clang::Diagnostic const& d)
#else
get_filename(clang::DiagnosticInfo const& d)
#endif
{
    if (!d.getLocation().isFileID()) {
        return std::string();
    }
    clang::FileID           f(d.getSourceManager().getFileID(d.getLocation()));
    clang::FileEntry const* fentry(d.getSourceManager().getFileEntryForID(f));
    return fentry->getName();
}

void
#if !defined(CLANG_29)
CB::DiagnosticFilter::HandleDiagnostic(clang::DiagnosticsEngine::Level level,
                                       clang::Diagnostic const&        info)
{
    if (clang::DiagnosticsEngine::Note != level
        && (clang::DiagnosticsEngine::Warning < level
            || !info.getLocation().isFileID()
            || this->d_analyser->is_component(::get_filename(info)))
        )
    {
        this->DiagnosticConsumer::HandleDiagnostic(level, info);
        this->d_client->HandleDiagnostic(level, info);
    }
}
#else
CB::DiagnosticFilter::HandleDiagnostic(clang::Diagnostic::Level     level,
                                       clang::DiagnosticInfo const& info)
{
    if (clang::Diagnostic::Note != level
        && (clang::Diagnostic::Warning < level
            || !info.getLocation().isFileID()
            || this->d_analyser->is_component(::get_filename(info)))
        )
    {
        this->DiagnosticClient::HandleDiagnostic(level, info);
        this->d_client->HandleDiagnostic(level, info);
    }
}
#endif

#if !defined(CLANG_29)
clang::DiagnosticConsumer*
CB::DiagnosticFilter::clone(clang::DiagnosticsEngine&) const
{
    return new CB::DiagnosticFilter(*this->d_analyser, *this->d_options);
}
#endif
