// csabase_format.cpp                                                 -*-C++-*-

#include <csabase_format.h>
#include <clang/AST/Type.h>
#include <clang/AST/Decl.h>
#include <clang/AST/PrettyPrinter.h>
#include <clang/Basic/Diagnostic.h>    // IWYU pragma: keep
#include <clang/Basic/LangOptions.h>
#include <llvm/Support/raw_ostream.h>  // IWYU pragma: keep

using namespace csabase;
using namespace clang;

// -----------------------------------------------------------------------------

template <typename Target>
static void printer(Target& out, Type::TypeClass value)
{
    switch (value)
    {
#define TYPE(Class, Base) case Type::Class: out << #Class; break;
#define LAST_TYPE(Class)
#define ABSTRACT_TYPE(Class, Base)
#include "clang/AST/TypeNodes.def"
    default: out << "unknown-type-class=" << value; break;
    }
}

// -----------------------------------------------------------------------------

template <typename Target>
static void printer(Target& out, Decl::Kind value)
{
    switch (value)
    {
#define DECL(Derived, Base) case Decl::Derived: out << #Derived; break;
#define ABSTRACT_DECL(Abstract)
#include "clang/AST/DeclNodes.inc"
    default: out << "unknown-decl-kind=" << value; break;
    }
}

// -----------------------------------------------------------------------------

template <typename Target>
static void printer(Target& out, const NamedDecl *decl)
{
    PrintingPolicy pp{LangOptions()};
    pp.Indentation = 4;
    pp.SuppressSpecifiers = false;
    pp.SuppressTagKeyword = false;
    pp.SuppressTag = false;
    pp.SuppressScope = false;
    pp.SuppressUnwrittenScope = false;
    pp.SuppressInitializers = false;
    pp.ConstantArraySizeAsWritten = true;
    pp.AnonymousTagLocations = true;
    pp.Bool = true;
    pp.TerseOutput = false;
    pp.PolishForDeclaration = true;
    pp.IncludeNewlines = false;

    std::string buf;
    llvm::raw_string_ostream s(buf);
    decl->print(s, pp, 0, true);
    out << s.str();
}

// -----------------------------------------------------------------------------

template <typename T>
void csabase::Formatter<T>::print(llvm::raw_ostream& out) const
{
    printer(out, value_);
}

template <typename T>
void csabase::Formatter<T>::print(DiagnosticBuilder& out) const
{
    printer(out, value_);
}

// -----------------------------------------------------------------------------

template class csabase::Formatter<Type::TypeClass>;
template class csabase::Formatter<Decl::Kind>;
template class csabase::Formatter<const NamedDecl *>;

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
