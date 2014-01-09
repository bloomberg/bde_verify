// csadep_dependencies.cpp                                            -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csadep_dependencies.h>
#include <clang/AST/Decl.h>
#include <llvm/Support/raw_ostream.h>

// ----------------------------------------------------------------------------

cool::csadep::dependencies::dependency::dependency(
    clang::SourceLocation const& location,
    bool                         need_definition,
    clang::NamedDecl const*      decl)
    : d_location(location)
    , d_need_definition(need_definition)
    , d_decl(decl)
{
}

// ----------------------------------------------------------------------------

void
cool::csadep::dependencies::add(std::string const&           source,
                                clang::SourceLocation const& location,
                                bool                         need_definition,
                                clang::NamedDecl const*      decl)
{
    typedef container::iterator iterator;

    std::string name(decl->getQualifiedNameAsString());

    llvm::errs() << "add(" << source << ", loc, "
                 << (need_definition? "definition": "declaration") << ", "
                 << name << ")\n";
    container& decls = d_sources[source];
    iterator it(decls.insert(std::make_pair(std::string(name),
                                            dependency(location,
                                                       need_definition,
                                                       decl))).first);
    if (need_definition) {
        it->second.d_need_definition = true;
    }
}

// ----------------------------------------------------------------------------

static std::map<std::string, cool::csadep::dependencies::dependency> const
s_empty;

std::pair<cool::csadep::dependencies::const_iterator,
          cool::csadep::dependencies::const_iterator>
cool::csadep::dependencies::get_referenced(std::string const& source) const
{
    return std::make_pair(s_empty.begin(), s_empty.end());
}
