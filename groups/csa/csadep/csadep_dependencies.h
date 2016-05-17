// csadep_dependencies.h                                              -*-C++-*-

#ifndef INCLUDED_CSADEP_DEPENDENCIES
#define INCLUDED_CSADEP_DEPENDENCIES

#include <clang/Basic/SourceLocation.h>
#include <string>
#include <map>
#include <utility>

// ----------------------------------------------------------------------------

namespace clang { class NamedDecl; }

namespace bde_verify
{
namespace csadep
{
class dependencies
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
}
}

#endif

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
