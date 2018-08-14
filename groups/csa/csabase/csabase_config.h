// csabase_config.h                                                   -*-C++-*-

#ifndef INCLUDED_CSABASE_CONFIG
#define INCLUDED_CSABASE_CONFIG

#include <clang/Basic/SourceLocation.h>
#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace clang { class SourceManager; }
namespace clang { class CompilerInstance; }

// -----------------------------------------------------------------------------

namespace csabase { class Analyser; }
namespace csabase
{
class Config
{
public:
    enum Status
    {
        off,
        on
    };

    Config(std::vector<std::string> const& config,
           clang::CompilerInstance &compiler);
        // Create a 'Config' object initialized with the set of specified
        // 'config' lines, using the specified 'compiler'.

    bool load(std::string const& file);
        // Read a set of configuration lines from the specified 'file'.
        // Return 'true' iff the 'file' could be read.

    void process(std::string const& line);
        // Append the specifed 'line' to the configuration.

    std::string const& toplevel_namespace() const;

    std::map<std::string, Status> const& checks() const;

    void set_value(const std::string& key, const std::string& value);

    std::string const&
    value(const std::string& key,
          clang::SourceLocation where = clang::SourceLocation()) const;

    bool all() const;

    void bv_stack_level(std::vector<clang::SourceLocation> *stack,
                        clang::SourceLocation where) const;
        // Populate the specified 'stack' with the pragma bdeverify stack
        // locations for the specified location 'where'.

    bool suppressed(const std::string& tag, clang::SourceLocation where) const;
        // Return 'true' iff a diagnostic with the specified 'tag' should be
        // suppressed at the specified location 'where'.

    void push_suppress(clang::SourceLocation where);
        // Push a level onto the local diagnostics suppressions stack for the
        // specified location 'where'.

    void pop_suppress(clang::SourceLocation where);
        // Pop a level from the local diagnostics suppressions stack for the
        // specified location 'where', if the stack is not empty..

    void suppress(const std::string& tag,
                  clang::SourceLocation where,
                  bool on,
                  std::set<std::string> in_progress = std::set<std::string>());
        // Mark the specified 'tag' as suppressed or not at the current stack
        // level and the specified location 'where', depending on the state of
        // the 'on' flag.  If 'tag' is the name of a group, recursively process
        // the members of the group unles 'tag' is present in the optionally
        // specified 'in_progress' set.

    void set_bv_value(clang::SourceLocation where,
                      const std::string& variable,
                      const std::string& value);
        // Record, for the specified location 'where', the appearance of a
        // specified '#pragma bdeverify set variable value'.

    void check_bv_stack(Analyser& analyser) const;
        // Verify that the csabase pragmas form a proper stack.

    static std::vector<std::string> brace_expand(const std::string& s);
        // Brace-expand the specified string 's' (as done by ksh) and return
        // the vector of expanded strings.  E.g.,
        // 'bde_comp_{version,foo{,util}}.{h,{,t.}cpp}' becomes
        // bde_comp_version.h bde_comp_version.cpp bde_comp_version.t.cpp
        // bde_comp_foo.h     bde_comp_foo.cpp     bde_comp_foo.t.cpp
        // bde_comp_fooutil.h bde_comp_fooutil.cpp bde_comp_fooutil.t.cpp

    void set_reexports(const std::string& including_file,
                       const std::string& exported_file);
        // Record that the specified 'including_file' reexports the specified
        // 'exported_file' for purposes of transitive inclusion.

    bool reexports(const std::string& included_file,
                   const std::string& needed_file) const;
        // Report whether an include of the specified 'included_file' satisfies
        // a need for the inclusion of the specified 'needed_file'.

  private:
    std::string                                     d_toplevel_namespace;
    std::set<std::string>                           d_loadpath;
    std::map<std::string, Status>                   d_checks;
    std::map<std::string, std::vector<std::string>> d_groups;
    std::map<std::string, std::string>              d_values;
    std::set<std::pair<std::string, std::string>>   d_suppressions;
    std::vector<std::string>                        d_load_dirs;
    std::map<std::string, std::set<std::string>>    d_reexported_includes;

    struct BVData
    {
        clang::SourceLocation where;
        char type;
        std::string s1;
        std::string s2;

        BVData(clang::SourceLocation where,
               char type,
               const std::string& s1 = std::string(),
               const std::string& s2 = std::string())
            : where(where), type(type), s1(s1), s2(s2)
        {
        }
    };

    std::map<std::string, std::vector<BVData> >      d_local_bv_pragmas;
    Status                                           d_all;
    clang::SourceManager&                            d_manager;
};

std::istream& operator>>(std::istream&, Config::Status&);
std::ostream& operator<<(std::ostream&, Config::Status);
}

// -----------------------------------------------------------------------------

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
