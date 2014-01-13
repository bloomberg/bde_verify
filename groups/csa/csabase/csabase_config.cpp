// csabase_config.cpp                                                 -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include "groups/csa/csabase/csabase_config.h"
#include "groups/csa/csabase/csabase_filenames.h"
#include <llvm/Support/raw_ostream.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <istream>
#include <sstream>
#include <iterator>
#include <vector>
#ident "$Id$"

// ----------------------------------------------------------------------------

std::istream&
cool::csabase::operator>> (std::istream&                  in,
                           cool::csabase::Config::Status& value)
{
    std::string string;
    if (in >> string)
    {
        if (string == "on")
        {
            value = cool::csabase::Config::on;
        }
        else if (string == "off")
        {
            value = cool::csabase::Config::off;
        }
        else
        {
            in.setstate(std::ios_base::failbit);
        }
    }
    return in;
}

std::ostream&
cool::csabase::operator<< (std::ostream& out,
                           cool::csabase::Config::Status value)
{
    switch (value) {
    default:                         out << "<unknown>"; break;
    case cool::csabase::Config::on:  out << "on";        break;
    case cool::csabase::Config::off: out << "off";       break;
    }
    return out;
}

// ----------------------------------------------------------------------------
static void
set_status(std::map<std::string, cool::csabase::Config::Status>&   checks,
           std::map<std::string, std::vector<std::string> > const& groups,
           std::string const&                                      check,
           cool::csabase::Config::Status                           status,
           std::vector<std::string>&                               path)
{
    std::map<std::string, std::vector<std::string> >::const_iterator it(groups.find(check));
    if (it != groups.end()) {
        if (path.end() != std::find(path.begin(), path.end(), check)) {
            llvm::errs() << "WARNING: recursive group processing aborted for group '" << check << "'\n";
        }
        else {
            path.push_back(check);
            for (std::vector<std::string>::const_iterator cit(it->second.begin()), cend(it->second.end());
                 cit != cend; ++cit) {
                set_status(checks, groups, *cit, status, path);
            }
            path.pop_back();
        }
    }
    else {
        checks[check] = status;
    }
}

// ----------------------------------------------------------------------------

cool::csabase::Config::Config(std::string const& name)
: d_toplevel_namespace("BloombergLP")
, d_all(on)
{
    //-dk:TODO load global and user configuration?
    load(name);
}

void
cool::csabase::Config::process(std::string const& line)
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
            llvm::errs() << "WARNING: couldn't read namespace name from '" << line << "'\n";
        }
    }
    else if (command == "all") {
        cool::csabase::Config::Status status;
        if (args >> status) {
            d_all = status;
        }
        else {
            llvm::errs() << "WARNING: couldn't read 'all' configuration from '" << line << "'\n";
        }
    }
    else if (command == "check") {
        std::string                   check;
        cool::csabase::Config::Status status;
        if (args >> check >> status) {
            std::vector<std::string> path;
            set_status(d_checks, d_groups, check, status, path);
        }
        else {
            llvm::errs() << "WARNING: couldn't read check configuration from '" << line << "'\n";
        }
    }
    else if (command == "group") {
        std::string name;
        if (args >> name) {
            d_groups[name].assign(std::istream_iterator<std::string>(args),
                    std::istream_iterator<std::string>());
        }
        else {
            llvm::errs() << "WARNING: a group needs at least a name on line '" << line << "'\n";
        }
    }
    else if (command == "load") {
        std::string name;
        if (args >> name) {
            load(name);
        }
        else {
            llvm::errs() << "WARNING: no file name given on line '" << line << "'\n";
        }
    }
    else if (command == "set") {
        std::string key;
        if (args >> key) {
            std::string value;
            if (std::getline(args, value)) {
                d_values[key] = value;
            }
            else {
                llvm::errs() << "WARNING: set could not read value on line '" << line << "'\n";
            }
        }
        else {
            llvm::errs() << "WARNING: set needs name and value on line '" << line << "'\n";
        }
    }
    else if (command == "suppress") {
        std::string tag;
        if (args >> tag) {
            std::string file;
            while (args >> file) {
                d_suppressions.insert(std::make_pair(tag, file));
            }
        }
        else {
            llvm::errs() << "WARNING: suppress needs tag and files on line '" << line << "'\n";
        }
    }
    else if (command.empty() || command[0] != '#') {
        std::cout << "unknown configuration command='" << command << "' arguments='" << line << "'\n";
    }
}

void
cool::csabase::Config::load(std::string const& original)
{
    std::string file(original);
    if (!file.empty() && file[0] == '$') {
        std::string::size_type slash(file.find('/'));
        std::string variable(file.substr(1, slash - 1));
        if (char const* value = getenv(variable.c_str())) {
            file = value + file.substr(slash);
        }
        else {
            llvm::errs() << "WARNING: environment variable "
                         << "'" << variable << "' not set "
                         << "(file '" << file << "' is not loaded)\n";
            return;
        }
    }
    if (d_loadpath.end() != std::find(d_loadpath.begin(),
                                      d_loadpath.end(), file)) {
        llvm::errs() << "WARNING: recursive loading aborted for file '" << file << "'\n";
        return;
    }

    d_loadpath.push_back(file);
    std::ifstream in(file.c_str());
    std::string line;
    while (std::getline(in, line)) {
        process(line);
    }
    d_loadpath.pop_back();
}

// ----------------------------------------------------------------------------

std::string const&
cool::csabase::Config::toplevel_namespace() const
{
    return d_toplevel_namespace;
}

std::map<std::string, cool::csabase::Config::Status> const&
cool::csabase::Config::checks() const
{
    return d_checks;
}

const std::string& cool::csabase::Config::value(const std::string& key) const
{
    static std::string empty;
    if (d_values.find(key) != d_values.end()) {
        return d_values.find(key)->second;
    }
    return empty;
}

bool cool::csabase::Config::all() const
{
    return d_all;
}

bool
cool::csabase::Config::suppressed(const std::string& tag,
                                  const std::string& file) const
{
    cool::csabase::FileName fn(file);
    return d_suppressions.count(std::make_pair(tag, fn.name())) +
           d_suppressions.count(std::make_pair("*", fn.name())) +
           d_suppressions.count(std::make_pair(tag, "*"));
}
