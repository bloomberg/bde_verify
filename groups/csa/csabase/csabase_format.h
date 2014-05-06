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

namespace csabase {
    template <typename T> class Formatter;
    template <typename T> Formatter<T> format(T const& value);
    template <typename T>
    llvm::raw_ostream&
    operator<< (llvm::raw_ostream&, Formatter<T> const&);
    template <typename T>
    clang::DiagnosticBuilder&
    operator<< (clang::DiagnosticBuilder&, Formatter<T> const&);
} // close package namespace

// -----------------------------------------------------------------------------

template <typename T>
class csabase::Formatter
{
public:
    Formatter(T const& value);
    void print(llvm::raw_ostream&) const;
    void print(clang::DiagnosticBuilder&) const;

private:
    T const& value_;
};

template <typename T>
csabase::Formatter<T>::Formatter(T const& value):
    value_(value)
{
}

// -----------------------------------------------------------------------------

template <typename T>
csabase::Formatter<T>
csabase::format(T const& value)
{
    return csabase::Formatter<T>(value);
}

// -----------------------------------------------------------------------------

template <typename T>
llvm::raw_ostream&
csabase::operator<< (llvm::raw_ostream&                 out,
                           csabase::Formatter<T> const& value)
{
    value.print(out);
    return out;
}

template <typename T>
clang::DiagnosticBuilder&
csabase::operator<< (clang::DiagnosticBuilder&          builder,
                           csabase::Formatter<T> const& value)
{
    value.print(builder);
    return builder;
}

// -----------------------------------------------------------------------------

#endif /* INCLUDED_FRAMEWORK_FORMAT */
