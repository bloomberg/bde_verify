// csabase_config.cpp                                                 -*-C++-*-

#include <csabase_config.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/CompilerInstance.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>
#include <stddef.h>
#include <stdlib.h>
#include <algorithm>
#include <fstream>   // IWYU pragma: keep
#include <iostream>  // IWYU pragma: keep
#include <iterator>
#include <set>
#include <sstream>   // IWYU pragma: keep
#include <vector>
#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_filenames.h>
#include <csabase_location.h>

using namespace csabase;
using namespace clang;
using namespace llvm;

// ----------------------------------------------------------------------------

static std::string const check_name("base");

// ----------------------------------------------------------------------------

std::istream& csabase::operator>>(std::istream& in, Config::Status& value)
{
    std::string string;
    if (in >> string)
    {
        if (string == "on")
        {
            value = Config::on;
        }
        else if (string == "off")
        {
            value = Config::off;
        }
        else
        {
            in.setstate(std::ios_base::failbit);
        }
    }
    return in;
}

std::ostream& csabase::operator<<(std::ostream& out, Config::Status value)
{
    switch (value) {
    default:          out << "<unknown>"; break;
    case Config::on:  out << "on";        break;
    case Config::off: out << "off";       break;
    }
    return out;
}

// ----------------------------------------------------------------------------

static void
set_status(std::map<std::string, Config::Status>& checks,
           std::map<std::string, std::vector<std::string>> const& groups,
           std::string const& check,
           Config::Status status,
           std::vector<std::string>& path)
{
    std::map<std::string, std::vector<std::string>>::const_iterator it =
        groups.find(check);
    if (it == groups.end()) {
        checks[check] = status;
    } else if (path.end() == std::find(path.begin(), path.end(), check)) {
        path.push_back(check);
        for (std::vector<std::string>::const_iterator cit(it->second.begin()),
             cend(it->second.end());
             cit != cend;
             ++cit) {
            set_status(checks, groups, *cit, status, path);
        }
        path.pop_back();
    }
}

// ----------------------------------------------------------------------------

csabase::Config::Config(std::vector<std::string> const& config,
                        CompilerInstance&               compiler)
: d_toplevel_namespace("BloombergLP")
, d_all(on)
, d_manager(compiler.getSourceManager())
{
    d_load_dirs.emplace_back(".");
    for (const auto &f : compiler.getFrontendOpts().Inputs) {
        if (f.isFile()) {
            StringRef file = f.getFile();
            file = sys::path::remove_leading_dotslash(file);
            SmallVector<char, 1024> v(file.begin(), file.end());
            sys::fs::make_absolute(v);
            sys::path::remove_dots(v, true);
            sys::path::remove_filename(v);
            StringRef path(v.begin(), v.size());
            while (!path.empty()) {
                d_load_dirs.emplace_back(path);
                path = sys::path::parent_path(path);
            }
        }
    }

    for (size_t i = 0; i < config.size(); ++i) {
        process(config[i]);
    }
}

void
csabase::Config::process(std::string const& line)
{
    std::string command;
    std::istringstream args(line);
    if (!(args >> command >> std::ws)) {
    }
    else if ("namespace" == command) {
        std::string name;
        if (args >> name) {
            d_toplevel_namespace = name;
        }
        else {
            errs()
                << "WARNING: couldn't read namespace name from '"
                << line << "'\n";
        }
    }
    else if ("all" == command) {
        Config::Status status;
        if (args >> status) {
            d_all = status;
        }
        else {
            errs()
                << "WARNING: couldn't read 'all' configuration from '"
                << line << "'\n";
        }
    }
    else if ("check" == command) {
        std::string    check;
        Config::Status status;
        if (args >> check >> status) {
            std::vector<std::string> path;
            set_status(d_checks, d_groups, check, status, path);
        }
        else {
            errs()
                << "WARNING: couldn't read check configuration from '"
                << line << "'\n";
        }
    }
    else if ("group" == command) {
        std::string name;
        if (args >> name) {
            d_groups[name].assign(std::istream_iterator<std::string>(args),
                    std::istream_iterator<std::string>());
        }
        else {
            errs()
                << "WARNING: a group needs at least a name on line '"
                << line << "'\n";
        }
    }
    else if ("load" == command) {
        std::string name;
        if (std::getline(args, name)) {
            load(name);
        }
        else {
            errs()
                << "WARNING: no file name given on line '"
                << line << "'\n";
        }
    }
    else if ("set" == command || "append" == command || "prepend" == command) {
        std::string key;
        if (args >> key) {
            std::string rest;
            if (std::getline(args, rest)) {
                if ("append" == command) {
                    rest = value(key) + " " + rest;
                } else if ("prepend" == command) {
                    rest = rest + " " + value(key);
                }
                set_value(key, StringRef(rest).trim());
            }
            else {
                errs() << "WARNING: " << command
                             << " could not read value on line '" << line
                             << "'\n";
            }
        }
        else {
            errs()
                << "WARNING: " << command << " needs name and value on line '"
                << line << "'\n";
        }
    }
    else if ("suppress" == command) {
        std::string tag;
        if (args >> tag) {
            std::string file;
            while (args >> file) {
                FileName fn(file);
                if (d_suppressions.insert(std::make_pair(tag, fn.name()))
                        .second &&
                    d_groups.count(tag)) {
                    for (const auto& group_item : d_groups.find(tag)->second) {
                        process("suppress " + group_item + " " + file);
                    }
                }
            }
        }
        else {
            errs()
                << "WARNING: suppress needs tag and files on line '"
                << line << "'\n";
        }
    }
    else if ("unsuppress" == command) {
        std::string tag;
        if (args >> tag) {
            std::string file;
            while (args >> file) {
                FileName fn(file);
                auto p = std::make_pair(tag, fn.name());
                if (d_suppressions.erase(p) && d_groups.count(tag)) {
                    for (const auto& group_item : d_groups.find(tag)->second) {
                        process("unsuppress " + group_item + " " + file);
                    }
                }
            }
        }
        else {
            errs()
                << "WARNING: unsuppress needs tag and files on line '"
                << line << "'\n";
        }
    }
    else if (command.empty() || command[0] != '#') {
        std::cout << "unknown configuration command='" << command
                  << "' arguments='" << line << "'\n";
    }
}

bool
csabase::Config::load(std::string const& original)
{
    std::string file(original);
    if (file[0] == '~' && file[1] == '/') {
        // Annoyingly, shells are not expanding the ~ in --config=~/...
        file = "$HOME" + file.substr(1);
    }
    if (file[0] == '$') {
        std::string variable;
        std::string::size_type end;
        if (file[1] == '{') {
            end = file.find('}');
            variable = file.substr(2, end - 1);
        }
        else {
            end = file.find('/');
            variable = file.substr(1, end - 1);
        }
        if (char const* value = getenv(variable.c_str())) {
            file = value + file.substr(end);
        }
        else {
            errs()
                << "WARNING: environment variable '" << variable
                << "' not set (file '" << file
                << "' is not loaded)\n";
            return false;
        }
    }
    if (sys::path::filename(file) != file) {
        // File name contains path components; use as-is
        std::ifstream in(file.c_str());
        if (!in) {
            return false;
        }
        if (!d_loadpath.insert(file).second) {
            errs()
                << "WARNING: recursive loading aborted for file '"
                << file << "'\n";
            return false;
        }
        std::string line;
        while (std::getline(in, line)) {
            while (!line.empty() && line.back() == '\\') {
                line.pop_back();
                std::string next;
                if (!std::getline(in, next)) {
                    break;
                }
                line += next;
            }
            process(line);
        }
        d_loadpath.erase(file);
        return true;
    }

    // File name is a plain name, try to find it up from input files.
    for (auto &p : d_load_dirs) {
        SmallVector<char, 1024> v(p.begin(), p.end());
        sys::path::append(v, file);
        if (load(std::string(v.begin(), v.end()))) {
            return true;
        }
    }
    return false;
}

// ----------------------------------------------------------------------------

std::string const& csabase::Config::toplevel_namespace() const
{
    return d_toplevel_namespace;
}

std::map<std::string, csabase::Config::Status> const&
csabase::Config::checks() const
{
    return d_checks;
}

void csabase::Config::set_value(const std::string& key,
                                const std::string& value)
{
    d_values[key] = value;
}

void csabase::Config::bv_stack_level(
    std::vector<SourceLocation>* stack,
    SourceLocation where) const
{
    Location location(d_manager, where);
    FileName fn(location.file());
    stack->clear();
    stack->push_back(where);
    if (d_local_bv_pragmas.find(fn.name()) != d_local_bv_pragmas.end()) {
        const std::vector<BVData>& ls =
            d_local_bv_pragmas.find(fn.name())->second;
        for (size_t i = 0; i < ls.size(); ++i) {
            if (d_manager.isBeforeInTranslationUnit(where, ls[i].where)) {
                break;
            }
            if (ls[i].type == '>') {
                stack->push_back(ls[i].where);
            } else if (stack->size() > 1 && ls[i].type == '<') {
                stack->pop_back();
            }
        }
    }
}

const std::string& csabase::Config::value(const std::string& key,
                                          SourceLocation where) const
{
    if (where.isValid()) {
        Location location(d_manager, where);
        FileName fn(location.file());
        if (d_local_bv_pragmas.find(fn.name()) != d_local_bv_pragmas.end()) {
            const std::vector<BVData>& ls =
                d_local_bv_pragmas.find(fn.name())->second;
            // Find the pragma stack of the diagnostic location.
            std::vector<SourceLocation> stack;
            bv_stack_level(&stack, where);
            const std::string *found = 0;
            // Look for a set pragma before the diagnostic location, only
            // within the pragma stack for that location.
            std::vector<SourceLocation> pragma_stack(1, where);
            for (size_t i = 0; i < ls.size(); ++i) {
                if (d_manager.isBeforeInTranslationUnit(where, ls[i].where)) {
                    break;
                }
                if (ls[i].type == '>') {
                    pragma_stack.push_back(ls[i].where);
                } else if (pragma_stack.size() > 1 && ls[i].type == '<') {
                    pragma_stack.pop_back();
                } else if (   ls[i].type == '='
                           && ls[i].s1 == key
                           && pragma_stack.size() <= stack.size()
                           && pragma_stack.back() ==
                              stack[pragma_stack.size() - 1]) {
                    found = &ls[i].s2;
                }
            }

            if (found) {
                return *found;                                        // RETURN
            }
        }
    }

    static std::string empty;
    if (d_values.find(key) != d_values.end()) {
        return d_values.find(key)->second;
    }
    return empty;
}

bool csabase::Config::all() const
{
    return d_all;
}

namespace {
bool glob_match(StringRef name, StringRef pattern)
{
    while (name.size() != 0 && pattern.size() != 0 && pattern[0] != '*') {
        if (name[0] != pattern[0] && pattern[0] != '?') { return false; }
        name = name.drop_front(1);
        pattern = pattern.drop_front(1);
    }
    if (pattern.size() == 1) { return true; }
    if (pattern.size() == 0) { return name.size() == 0; }
    for (pattern = pattern.drop_front(1);; name = name.drop_front(1)) {
        if (glob_match(name, pattern)) { return true; }
        if (name.size() == 0) { return false; }
    }
}
}

bool csabase::Config::suppressed(const std::string& tag,
                                 SourceLocation where) const
{
    Location location(d_manager, where);
    FileName fn(location.file());

    if (d_local_bv_pragmas.find(fn.name()) != d_local_bv_pragmas.end()) {
        const std::vector<BVData>& ls =
            d_local_bv_pragmas.find(fn.name())->second;
        std::vector<SourceLocation> stack;
        bv_stack_level(&stack, where);
        char found = 0;
        // Look for a tag pragma before the diagnostic location, only within
        // the pragma stack for that location.
        std::vector<SourceLocation> pragma_stack(1, where);
        for (size_t i = 0; i < ls.size(); ++i) {
            if (d_manager.isBeforeInTranslationUnit(where, ls[i].where)) {
                break;
            }
            if (ls[i].type == '>') {
                pragma_stack.push_back(ls[i].where);
            } else if (pragma_stack.size() > 1 && ls[i].type == '<') {
                pragma_stack.pop_back();
            } else if (   (ls[i].type == '-' || ls[i].type == '+')
                       && pragma_stack.size() <= stack.size()
                       && pragma_stack.back() == stack[pragma_stack.size() - 1]
                       && glob_match(tag, ls[i].s1)) {
                found = ls[i].type;
            }
        }
        if (found) {
            return found == '-';                                      // RETURN
        }
    }

    for (const auto &sup : d_suppressions) {
        if (glob_match(tag, sup.first) && glob_match(fn.name(), sup.second)) {
            return true;
        }
    }
    return false;
}

void csabase::Config::push_suppress(SourceLocation where)
{
    Location location(d_manager, where);
    FileName fn(location.file());
    d_local_bv_pragmas[fn.name()].push_back(BVData(where, '>'));
}

void csabase::Config::pop_suppress(SourceLocation where)
{
    Location location(d_manager, where);
    FileName fn(location.file());
    d_local_bv_pragmas[fn.name()].push_back(BVData(where, '<'));
}

void csabase::Config::suppress(const std::string& tag,
                               SourceLocation where,
                               bool on,
                               std::set<std::string> in_progress)
{
    if (!in_progress.count(tag)) {
        Location location(d_manager, where);
        FileName fn(location.file());
        d_local_bv_pragmas[fn.name()]
            .push_back(BVData(where, on ? '-' : '+', tag));
        if (d_groups.find(tag) != d_groups.end()) {
            in_progress.insert(tag);
            const std::vector<std::string>& group_items =
                d_groups.find(tag)->second;
            for (size_t i = 0; i < group_items.size(); ++i) {
                suppress(group_items[i], where, on, in_progress);
            }
        }
    }
}

void csabase::Config::set_bv_value(SourceLocation where,
                                   const std::string& variable,
                                   const std::string& value)
{
    Location location(d_manager, where);
    FileName fn(location.file());
    d_local_bv_pragmas[fn.name()]
        .push_back(BVData(where, '=', variable, value));
}

void csabase::Config::check_bv_stack(Analyser& analyser) const
{
    for (std::map<std::string, std::vector<BVData> >::const_iterator
             b = d_local_bv_pragmas.begin(),
             e = d_local_bv_pragmas.end();
         b != e;
         ++b) {
        const std::vector<BVData>& ls = b->second;
        std::vector<int> local_stack;
        for (size_t i = 0; i < ls.size(); ++i) {
            if (ls[i].type == '>') {
                local_stack.push_back(i);
            } else if (ls[i].type == '<') {
                if (local_stack.size() > 0) {
                    local_stack.pop_back();
                } else {
                    analyser.report(ls[i].where, check_name, "PR01",
                            "Pop of empty stack");
                }
            }
        }
        while (local_stack.size() > 0) {
            analyser.report(ls[local_stack.back()].where, check_name, "PR02",
                "Push is not popped in this file");
            local_stack.pop_back();
        }
    }
}

namespace {

typedef std::vector<std::string> VS_t;

VS_t cross(const VS_t &vs1, const VS_t &vs2)
    // Return the cross product of the specified 'vs1' and vs2' sets.
{
    VS_t rs;
    for (const auto& s1 : vs1) {
        for (const auto& s2 : vs2) {
            rs.push_back(s1 + s2);
        }
    }
    return rs;
}

void append(VS_t &vs1, const VS_t &vs2)
    // Append the contents of the specified 'vs2' to the specified 'vs1'.
{
    vs1.insert(vs1.end(), vs2.begin(), vs2.end());
}

VS_t comma_split(const std::string &s, size_t &pos)
    // Split the specified string 's' starting from the specified 'pos' at top-
    // level commas into a set of strings and return them.  A top-level comma
    // is not enclosed within balanced braces.  Processing stops at the end of
    // the string or at the first unbalanced '}'.  The next position to be
    // processed will be set in 'pos'.
{
    VS_t result;
    std::string current;
    size_t nesting = 0;
    while (char c = s[pos++]) {
        if (c == ',' && nesting == 0) {
            result.push_back(current);
            current.clear();
        }
        else if (c == '}' && nesting == 0) {
            result.push_back(current);
            break;
        }
        else {
            current += c;
            if (c == '{') {
                ++nesting;
            }
            else if (c == '}') {
                --nesting;
            }
        }
    }
    return result;
}

VS_t expand(const std::string &s, size_t &pos)
    // Brace-expand the specified string 's' starting from the specified 'pos'
    // and return the set of strings.
{
    size_t left = s.find('{', pos);
    if (left == s.npos) {
        VS_t result{s.substr(pos)};
        pos = s.size();
        return result;
    }
    VS_t result{s.substr(pos, left - pos)};
    pos = left + 1;
    VS_t x = comma_split(s, pos);
    VS_t w;
    for (const auto& sx : x) {
        size_t xpos = 0;
        append(w, expand(sx, xpos));
    }
    return cross(cross(result, w), expand(s, pos));
}

}

std::vector<std::string> csabase::Config::brace_expand(const std::string &s)
{
    size_t p = 0;
    return expand(s, p);
}

void csabase::Config::set_reexports(const std::string &including_file,
                                    const std::string& exported_file)
{
    if (d_reexported_includes[including_file].insert(exported_file).second) {
        for (auto &a : d_reexported_includes) {
            if (a.second.count(including_file)) {
                set_reexports(a.first, exported_file);
            }
        }
    }
}

bool csabase::Config::reexports(const std::string& included_file,
                                const std::string& needed_file) const
{
    auto i = d_reexported_includes.find(included_file);
    return i != d_reexported_includes.end() && i->second.count(needed_file);
}

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
