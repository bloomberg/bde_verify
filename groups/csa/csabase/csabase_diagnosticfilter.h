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
    : public clang::DiagnosticConsumer
{
public:
    DiagnosticFilter(cool::csabase::Analyser const& analyser,
                     bool toplevel_only,
                     clang::DiagnosticOptions & options);
    ~DiagnosticFilter();

    void BeginSourceFile(clang::LangOptions const&  opts,
                         clang::Preprocessor const* pp);
    void EndSourceFile();
    bool IncludeInDiagnosticCount() const;
    clang::DiagnosticConsumer* clone(clang::DiagnosticsEngine&) const;
    void HandleDiagnostic(clang::DiagnosticsEngine::Level level,
                          clang::Diagnostic const&        info);

private:
    DiagnosticFilter(DiagnosticFilter const&);
    void operator=(DiagnosticFilter const&);

    clang::DiagnosticOptions *               d_options;
    std::auto_ptr<clang::DiagnosticConsumer> d_client;
    cool::csabase::Analyser const*           d_analyser;
    bool                                     d_toplevel_only;
};

// ----------------------------------------------------------------------------

#endif
