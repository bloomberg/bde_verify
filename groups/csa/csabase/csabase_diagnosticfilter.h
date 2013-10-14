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
#if !defined(CLANG_29)
    : public clang::DiagnosticConsumer
#else
    : public clang::DiagnosticClient
#endif
{
public:
    DiagnosticFilter(cool::csabase::Analyser const& analyser,
                     clang::DiagnosticOptions & options);
    ~DiagnosticFilter();

    void BeginSourceFile(clang::LangOptions const&  opts,
                         clang::Preprocessor const* pp);
    void EndSourceFile();
    bool IncludeInDiagnosticCount() const;
#if !defined(CLANG_29)
    clang::DiagnosticConsumer* clone(clang::DiagnosticsEngine&) const;
    void HandleDiagnostic(clang::DiagnosticsEngine::Level level,
                          clang::Diagnostic const&        info);
#else
    void HandleDiagnostic(clang::Diagnostic::Level     level,
                          clang::DiagnosticInfo const& info);
#endif

private:
    DiagnosticFilter(DiagnosticFilter const&);
    void operator=(DiagnosticFilter const&);

    clang::DiagnosticOptions *               d_options;
#if !defined(CLANG_29)
    std::auto_ptr<clang::DiagnosticConsumer> d_client;
#else
    std::auto_ptr<clang::DiagnosticClient>   d_client;
#endif
    cool::csabase::Analyser const*           d_analyser;
};

// ----------------------------------------------------------------------------

#endif
