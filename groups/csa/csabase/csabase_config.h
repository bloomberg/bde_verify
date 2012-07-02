// -*-c++-*- groups/csa/csabase/csabase_config.h 
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(INCLUDED_GROUPS_CSA_CSABASE_CSABASE_CONFIG_H)
#define INCLUDED_GROUPS_CSA_CSABASE_CSABASE_CONFIG_H 1
#ident "$Id$"

#include <iosfwd>
#include <map>
#include <string>

// -----------------------------------------------------------------------------

namespace cool
{
    namespace csabase
    {
        class Config;
    }
}

// -----------------------------------------------------------------------------

class cool::csabase::Config
{
public:
    enum Status
    {
        off,
        on
    };

    Config(std::string const& name);
    void load(std::string const& file);
    
    std::string const&                   toplevel_namespace() const;
    std::map<std::string, Status> const& checks() const;

private:
    std::string                   d_toplevel_namespace;
    std::map<std::string, Status> d_checks;
};

// -----------------------------------------------------------------------------

namespace cool
{
    namespace csabase
    {
        std::istream& operator>> (std::istream&, cool::csabase::Config::Status&);
        std::ostream& operator<< (std::ostream&, cool::csabase::Config::Status);
    }
}

// -----------------------------------------------------------------------------

#endif /* INCLUDED_GROUPS_CSA_CSABASE_CSABASE_CONFIG_H */
