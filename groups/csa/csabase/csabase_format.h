// csabase_format.h                                                   -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(INCLUDED_CSABASE_FORMAT)
#define INCLUDED_CSABASE_FORMAT 1
#ident "$Id$"

#include <llvm/Support/raw_ostream.h>
#include <clang/Basic/Diagnostic.h>

// -----------------------------------------------------------------------------

namespace bde_verify
{
    namespace csabase
    {
        template <typename T> class Formatter;
        template <typename T> Formatter<T> format(T const& value);
        template <typename T>
        llvm::raw_ostream&
        operator<< (llvm::raw_ostream&, Formatter<T> const&);
        template <typename T>
        clang::DiagnosticBuilder&
        operator<< (clang::DiagnosticBuilder&, Formatter<T> const&);
    }
}

// -----------------------------------------------------------------------------

template <typename T>
class bde_verify::csabase::Formatter
{
public:
    Formatter(T const& value);
    void print(llvm::raw_ostream&) const;
    void print(clang::DiagnosticBuilder&) const;

private:
    T const& value_;
};

template <typename T>
bde_verify::csabase::Formatter<T>::Formatter(T const& value):
    value_(value)
{
}

// -----------------------------------------------------------------------------

template <typename T>
bde_verify::csabase::Formatter<T>
bde_verify::csabase::format(T const& value)
{
    return bde_verify::csabase::Formatter<T>(value);
}

// -----------------------------------------------------------------------------

template <typename T>
llvm::raw_ostream&
bde_verify::csabase::operator<< (llvm::raw_ostream&                 out,
                           bde_verify::csabase::Formatter<T> const& value)
{
    value.print(out);
    return out;
}

template <typename T>
clang::DiagnosticBuilder&
bde_verify::csabase::operator<< (clang::DiagnosticBuilder&          builder,
                           bde_verify::csabase::Formatter<T> const& value)
{
    value.print(builder);
    return builder;
}

// -----------------------------------------------------------------------------

#endif /* INCLUDED_FRAMEWORK_FORMAT */
