// csabase_format.cpp                                                 -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_format.h>
#include <clang/AST/Type.h>
#include <clang/AST/DeclBase.h>
#ident "$Id$"

// -----------------------------------------------------------------------------

template <typename Target>
static void
print(Target& out, clang::Type::TypeClass value)
{
    switch (value)
    {
#define TYPE(Class, Base) case clang::Type::Class: out << #Class; break;
#define LAST_TYPE(Class)
#define ABSTRACT_TYPE(Class, Base)
#include "clang/AST/TypeNodes.def"
    default: out << "unknown-type-class=" << value; break;
    }
}

// -----------------------------------------------------------------------------

template <typename Target>
static void
print(Target& out, clang::Decl::Kind value)
{
    switch (value)
    {
#define DECL(Derived, Base) case clang::Decl::Derived: out << #Derived; break;
#define ABSTRACT_DECL(Abstract)
#include "clang/AST/DeclNodes.inc"
    default: out << "unknown-decl-kind=" << value; break;
    }
}

// -----------------------------------------------------------------------------

template <typename T>
void
bde_verify::csabase::Formatter<T>::print(llvm::raw_ostream& out) const
{
    ::print(out, value_);
}

template <typename T>
void
bde_verify::csabase::Formatter<T>::print(clang::DiagnosticBuilder& out) const
{
    ::print(out, value_);
}

// -----------------------------------------------------------------------------

template class bde_verify::csabase::Formatter<clang::Type::TypeClass>;
template class bde_verify::csabase::Formatter<clang::Decl::Kind>;
