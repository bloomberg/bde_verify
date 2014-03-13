// csadep_dependencies.h                                              -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#if !defined(INCLUDED_CSADEP_DEPENDENCIES)
#define INCLUDED_CSADEP_DEPENDENCIES 1

#include <clang/Basic/SourceLocation.h>
#include <string>
#include <map>
#include <utility>

// ----------------------------------------------------------------------------

namespace clang
{
    class NamedDecl;
}

namespace bde_verify
{
    namespace csadep
    {
        class dependencies;
    }
}

// ----------------------------------------------------------------------------

class bde_verify::csadep::dependencies
{
public:
    struct dependency
    {
        dependency(clang::SourceLocation const& location,
                   bool                         need_definition,
                   clang::NamedDecl const*      decl);

        clang::SourceLocation   d_location;
        bool                    d_need_definition;
        clang::NamedDecl const* d_decl;
    };
    typedef std::map<std::string, dependency> container;
    typedef container::const_iterator         const_iterator;

private:
    std::map<std::string, container> d_sources;

public:

    void add(std::string const&           source,
             clang::SourceLocation const& location,
             bool                         need_definition,
             clang::NamedDecl const*      decl);

    std::pair<const_iterator, const_iterator>
        get_referenced(std::string const& source) const;
};

// ----------------------------------------------------------------------------

#endif
