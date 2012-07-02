// -*-c++-*- groups/csa/csabase/csabase_config.cpp 
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include "groups/csa/csabase/csabase_config.h"
#include <llvm/Support/raw_ostream.h>
#include <fstream>
#include <iostream>
#include <istream>
#include <sstream>
#ident "$Id$"

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

cool::csabase::Config::Config(std::string const& name):
    d_toplevel_namespace("BloombergLP")
{
    //-dk:TODO load global and user configuration?
    this->load(name);
}

void
cool::csabase::Config::load(std::string const& file)
{
    std::ifstream in(file.c_str());
    for (std::string command, line;
         std::getline(in >> command >> std::ws, line); ) {
        std::istringstream args(line);
        if (command == "namespace") {
            std::string name;
            if (args >> name) {
                this->d_toplevel_namespace = name;
            }
            else {
                llvm::errs() << "WARNING: couldn't read namespace name from '" << line << "'\n";
            }
        }
        else if (command == "check") {
            std::string                   check;
            cool::csabase::Config::Status status;
            if (args >> check >> status) {
                this->d_checks[check] = status;
            }
            else {
                llvm::errs() << "WARNING: couldn't read check configuration from '" << line << "'\n";
            }
        }
        else if (command.empty() || command[0] != '#') {
            std::cout << "unknown configuration command='" << command << "' arguments='" << line << "'\n";
        }
    }
}

// -----------------------------------------------------------------------------

std::string const&
cool::csabase::Config::toplevel_namespace() const
{
    return this->d_toplevel_namespace;
}

std::map<std::string, cool::csabase::Config::Status> const&
cool::csabase::Config::checks() const
{
    return this->d_checks;
}
