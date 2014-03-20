// csabase_config.cpp                                                 -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include "csabase_analyser.h"
#include "csabase_config.h"
#include "csabase_debug.h"
#include "csabase_diagnostic_builder.h"
#include "csabase_filenames.h"
#include "csabase_location.h"
#include <llvm/Support/raw_ostream.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <istream>
#include <set>
#include <sstream>
#include <iterator>
#include <vector>
#ident "$Id$"

static std::string const check_name("base");

// ----------------------------------------------------------------------------

std::istream&
bde_verify::csabase::operator>> (std::istream&                  in,
                           bde_verify::csabase::Config::Status& value)
{
    std::string string;
    if (in >> string)
    {
        if (string == "on")
        {
            value = bde_verify::csabase::Config::on;
        }
        else if (string == "off")
        {
            value = bde_verify::csabase::Config::off;
        }
        else
        {
            in.setstate(std::ios_base::failbit);
        }
    }
    return in;
}

std::ostream&
bde_verify::csabase::operator<< (std::ostream& out,
                           bde_verify::csabase::Config::Status value)
{
    switch (value) {
    default:                         out << "<unknown>"; break;
    case bde_verify::csabase::Config::on:  out << "on";        break;
    case bde_verify::csabase::Config::off: out << "off";       break;
    }
    return out;
}

// ----------------------------------------------------------------------------
static void
set_status(std::map<std::string, bde_verify::csabase::Config::Status>&   checks,
           std::map<std::string, std::vector<std::string> > const& groups,
           std::string const&                                      check,
           bde_verify::csabase::Config::Status                           status,
           std::vector<std::string>&                               path)
{
    std::map<std::string, std::vector<std::string> >::const_iterator it =
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

bde_verify::csabase::Config::Config(std::vector<std::string> const& config,
                              clang::SourceManager& manager)
: d_toplevel_namespace("BloombergLP")
, d_all(on)
, d_manager(manager)
{
    //-dk:TODO load global and user configuration?
    for (size_t i = 0; i < config.size(); ++i) {
        process(config[i]);
    }
}

void
bde_verify::csabase::Config::process(std::string const& line)
{
    std::string command;
    std::istringstream args(line);
    if (!(args >> command >> std::ws)) {
    }
    else if (command == "namespace") {
        std::string name;
        if (args >> name) {
            d_toplevel_namespace = name;
        }
        else {
            llvm::errs()
                << "WARNING: couldn't read namespace name from '"
                << line << "'\n";
        }
    }
    else if (command == "all") {
        bde_verify::csabase::Config::Status status;
        if (args >> status) {
            d_all = status;
        }
        else {
            llvm::errs()
                << "WARNING: couldn't read 'all' configuration from '"
                << line << "'\n";
        }
    }
    else if (command == "check") {
        std::string                   check;
        bde_verify::csabase::Config::Status status;
        if (args >> check >> status) {
            std::vector<std::string> path;
            set_status(d_checks, d_groups, check, status, path);
        }
        else {
            llvm::errs()
                << "WARNING: couldn't read check configuration from '"
                << line << "'\n";
        }
    }
    else if (command == "group") {
        std::string name;
        if (args >> name) {
            d_groups[name].assign(std::istream_iterator<std::string>(args),
                    std::istream_iterator<std::string>());
        }
        else {
            llvm::errs()
                << "WARNING: a group needs at least a name on line '"
                << line << "'\n";
        }
    }
    else if (command == "load") {
        std::string name;
        if (args >> name) {
            load(name);
        }
        else {
            llvm::errs()
                << "WARNING: no file name given on line '"
                << line << "'\n";
        }
    }
    else if (command == "set") {
        std::string key;
        if (args >> key) {
            std::string value;
            if (std::getline(args, value)) {
                set_value(key, llvm::StringRef(value).trim());
            }
            else {
                llvm::errs()
                    << "WARNING: set could not read value on line '"
                    << line << "'\n";
            }
        }
        else {
            llvm::errs()
                << "WARNING: set needs name and value on line '"
                << line << "'\n";
        }
    }
    else if (command == "suppress") {
        std::string tag;
        if (args >> tag) {
            std::string file;
            while (args >> file) {
                bde_verify::csabase::FileName fn(file);
                d_suppressions.insert(std::make_pair(tag, fn.name()));
            }
        }
        else {
            llvm::errs()
                << "WARNING: suppress needs tag and files on line '"
                << line << "'\n";
        }
    }
    else if (command.empty() || command[0] != '#') {
        std::cout << "unknown configuration command='" << command
                  << "' arguments='" << line << "'\n";
    }
}

void
bde_verify::csabase::Config::load(std::string const& original)
{
    std::string file(original);
    if (!file.empty() && file[0] == '$') {
        std::string::size_type slash(file.find('/'));
        std::string variable(file.substr(1, slash - 1));
        if (char const* value = getenv(variable.c_str())) {
            file = value + file.substr(slash);
        }
        else {
            llvm::errs()
                << "WARNING: environment variable '" << variable
                << "' not set (file '" << file
                << "' is not loaded)\n";
            return;
        }
    }
    if (d_loadpath.end() != std::find(d_loadpath.begin(),
                                      d_loadpath.end(), file)) {
        llvm::errs()
            << "WARNING: recursive loading aborted for file '"
            << file << "'\n";
        return;
    }

    d_loadpath.push_back(file);
    std::ifstream in(file.c_str());
    std::string line;
    while (std::getline(in, line)) {
        while (!line.empty() && line[line.size() - 1] == '\\') {
            std::string next;
            if (std::getline(in, next)) {
                line.resize(line.size() - 1);
                line += next;
            }
        }
        process(line);
    }
    d_loadpath.pop_back();
}

// ----------------------------------------------------------------------------

std::string const&
bde_verify::csabase::Config::toplevel_namespace() const
{
    return d_toplevel_namespace;
}

std::map<std::string, bde_verify::csabase::Config::Status> const&
bde_verify::csabase::Config::checks() const
{
    return d_checks;
}

void bde_verify::csabase::Config::set_value(const std::string& key,
                                      const std::string& value)
{
    d_values[key] = value;
}

void bde_verify::csabase::Config::bv_stack_level(
    std::vector<clang::SourceLocation>* stack,
    clang::SourceLocation where) const
{
    bde_verify::csabase::Location location(d_manager, where);
    bde_verify::csabase::FileName fn(location.file());
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

const std::string&
bde_verify::csabase::Config::value(const std::string& key,
                             clang::SourceLocation where) const
{
    if (where.isValid()) {
        bde_verify::csabase::Location location(d_manager, where);
        bde_verify::csabase::FileName fn(location.file());
        if (d_local_bv_pragmas.find(fn.name()) != d_local_bv_pragmas.end()) {
            const std::vector<BVData>& ls =
                d_local_bv_pragmas.find(fn.name())->second;
            // Find the pragma stack of the diagnostic location.
            std::vector<clang::SourceLocation> stack;
            bv_stack_level(&stack, where);
            const std::string *found = 0;
            // Look for a set pragma before the diagnostic location, only
            // within the pragma stack for that location.
            std::vector<clang::SourceLocation> pragma_stack(1, where);
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

bool bde_verify::csabase::Config::all() const
{
    return d_all;
}

bool
bde_verify::csabase::Config::suppressed(const std::string& tag,
                                  clang::SourceLocation where) const
{
    bde_verify::csabase::Location location(d_manager, where);
    bde_verify::csabase::FileName fn(location.file());

    if (d_local_bv_pragmas.find(fn.name()) != d_local_bv_pragmas.end()) {
        const std::vector<BVData>& ls =
            d_local_bv_pragmas.find(fn.name())->second;
        std::vector<clang::SourceLocation> stack;
        bv_stack_level(&stack, where);
        char found = 0;
        // Look for a tag pragma before the diagnostic location, only within
        // the pragma stack for that location.
        std::vector<clang::SourceLocation> pragma_stack(1, where);
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
                       && (   ls[i].s1 == tag
                           || ls[i].s1 == "*")) {
                found = ls[i].type;
            }
        }
        if (found) {
            return found == '-';                                      // RETURN
        }
    }

    return d_suppressions.count(std::make_pair(tag, fn.name())) +
           d_suppressions.count(std::make_pair("*", fn.name())) +
           d_suppressions.count(std::make_pair(tag, "*"));
}

void
bde_verify::csabase::Config::push_suppress(clang::SourceLocation where)
{
    bde_verify::csabase::Location location(d_manager, where);
    bde_verify::csabase::FileName fn(location.file());
    d_local_bv_pragmas[fn.name()].push_back(BVData(where, '>'));
}

void
bde_verify::csabase::Config::pop_suppress(clang::SourceLocation where)
{
    bde_verify::csabase::Location location(d_manager, where);
    bde_verify::csabase::FileName fn(location.file());
    d_local_bv_pragmas[fn.name()].push_back(BVData(where, '<'));
}

void bde_verify::csabase::Config::suppress(const std::string& tag,
                                     clang::SourceLocation where,
                                     bool on,
                                     std::set<std::string> in_progress)
{
    if (!in_progress.count(tag)) {
        bde_verify::csabase::Location location(d_manager, where);
        bde_verify::csabase::FileName fn(location.file());
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

void bde_verify::csabase::Config::set_bv_value(clang::SourceLocation where,
                                         const std::string& variable,
                                         const std::string& value)
{
    bde_verify::csabase::Location location(d_manager, where);
    bde_verify::csabase::FileName fn(location.file());
    d_local_bv_pragmas[fn.name()]
        .push_back(BVData(where, '=', variable, value));
}

void bde_verify::csabase::Config::check_bv_stack(
    bde_verify::csabase::Analyser& analyser) const
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
