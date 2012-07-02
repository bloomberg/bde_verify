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
    : d_client(new clang::TextDiagnosticPrinter(llvm::errs(), options))
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
get_filename(clang::DiagnosticInfo const& d)
{
    clang::FileID           f(d.getSourceManager().getFileID(d.getLocation()));
    clang::FileEntry const* fentry(d.getSourceManager().getFileEntryForID(f));
    return fentry->getName();
}

void
CB::DiagnosticFilter::HandleDiagnostic(clang::Diagnostic::Level     level,
                                       clang::DiagnosticInfo const& info)
{
    if (clang::Diagnostic::Warning < level
        || this->d_analyser->is_component(::get_filename(info))
        )
    {
        this->DiagnosticClient::HandleDiagnostic(level, info);
        this->d_client->HandleDiagnostic(level, info);
    }
}
