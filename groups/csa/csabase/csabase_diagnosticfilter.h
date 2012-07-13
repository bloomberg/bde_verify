// csabase_diagnosticfilter.h                                         -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#if !defined(INCLUDED_CSABASE_DIAGNOSTICFILTER_H)
#define INCLUDED_CSABASE_DIAGNOSTICFILTER_H 1
#ident "$Id$"

#include <clang/Basic/Diagnostic.h>
#include <memory>

// ----------------------------------------------------------------------------

namespace clang
{
    class DiagnosticOptions;
}

namespace cool
{
    namespace csabase
    {
        class Analyser;
        class DiagnosticFilter;
    }
}

// ----------------------------------------------------------------------------

class cool::csabase::DiagnosticFilter
    : public clang::DiagnosticClient
{
public:
    DiagnosticFilter(cool::csabase::Analyser const& analyser,
                     clang::DiagnosticOptions const& options);
    ~DiagnosticFilter();

    void BeginSourceFile(clang::LangOptions const&  opts,
                         clang::Preprocessor const* pp);
    void EndSourceFile();
    bool IncludeInDiagnosticCount() const;
    void HandleDiagnostic(clang::Diagnostic::Level     level,
                          clang::DiagnosticInfo const& info);

private:
    DiagnosticFilter(DiagnosticFilter const&);
    void operator=(DiagnosticFilter const&);

    std::auto_ptr<clang::DiagnosticClient> d_client;
    cool::csabase::Analyser const*         d_analyser;
};

// ----------------------------------------------------------------------------

#endif
