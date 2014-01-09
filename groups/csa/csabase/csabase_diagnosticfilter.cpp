// csabase_diagnosticfilter.cpp                                       -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csabase_diagnosticfilter.h>
#include <csabase_analyser.h>
#include <csabase_registercheck.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/FileManager.h>
#include <llvm/Support/raw_ostream.h>
#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("diagnostic-filter");

// ----------------------------------------------------------------------------

namespace CB = cool::csabase;

// ----------------------------------------------------------------------------

CB::DiagnosticFilter::DiagnosticFilter(CB::Analyser const& analyser,
                                       clang::DiagnosticOptions & options)
    : d_options(&options)
    , d_client(new clang::TextDiagnosticPrinter(llvm::errs(), d_options))
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
    d_client->BeginSourceFile(opts, pp);
}

void
CB::DiagnosticFilter::EndSourceFile()
{
    d_client->EndSourceFile();
}

bool
CB::DiagnosticFilter::IncludeInDiagnosticCount() const
{
    return true;
}

static std::string
get_filename(clang::Diagnostic const& d)
{
    if (!d.getLocation().isFileID()) {
        return std::string();
    }
    clang::FileID           f(d.getSourceManager().getFileID(d.getLocation()));
    clang::FileEntry const* fentry(d.getSourceManager().getFileEntryForID(f));
    return fentry->getName();
}

void
CB::DiagnosticFilter::HandleDiagnostic(clang::DiagnosticsEngine::Level level,
                                       clang::Diagnostic const&        info)
{
    if (!info.getLocation().isFileID() ||
        d_analyser->is_component(get_filename(info)))
    /*
    if (clang::DiagnosticsEngine::Note != level
        && (clang::DiagnosticsEngine::Warning < level
            || !info.getLocation().isFileID()
            || d_analyser->is_component(get_filename(info)))
        )
     */
    {
        DiagnosticConsumer::HandleDiagnostic(level, info);
        d_client->HandleDiagnostic(level, info);
    }
}

clang::DiagnosticConsumer*
CB::DiagnosticFilter::clone(clang::DiagnosticsEngine&) const
{
    return new CB::DiagnosticFilter(*d_analyser, *d_options);
}

// ----------------------------------------------------------------------------

static void check(CB::Analyser& analyser, const clang::TranslationUnitDecl*)
{
}

// ----------------------------------------------------------------------------

static CB::RegisterCheck register_check(check_name, &check);
