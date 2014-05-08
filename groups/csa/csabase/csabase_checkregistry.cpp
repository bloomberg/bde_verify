// csabase_checkregistry.cpp                                          -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_checkregistry.h>
#include <csabase_analyser.h>
#include <csabase_config.h>
#include <llvm/Support/raw_ostream.h>
#include <fstream>
#include <sstream>
#include <map>
#ident "$Id$"

// -----------------------------------------------------------------------------

namespace
{
    typedef std::multimap<std::string, csabase::CheckRegistry::Subscriber> map_type;

    map_type&
    checks()
    {
        static map_type rc;
        return rc;
    }
}

// -----------------------------------------------------------------------------

void
csabase::CheckRegistry::add_check(std::string const& name,
                                        csabase::CheckRegistry::Subscriber check)
{
    checks().insert(std::make_pair(name, check));
}

// -----------------------------------------------------------------------------

void
csabase::CheckRegistry::attach(csabase::Analyser& analyser,
                                     csabase::Visitor& visitor,
                                     csabase::PPObserver& observer)
{
    typedef map_type::const_iterator const_iterator;
    typedef std::map<std::string, csabase::Config::Status> checks_type;
    checks_type const& config(analyser.config()->checks());
    for (checks_type::const_iterator it(config.begin()), end(config.end());
         it != end; ++it) {
        if (checks().find(it->first) == checks().end()) {
            llvm::errs() << "unknown check '" << it->first << "'; "
                         << "existing checks:\n";
            for (const_iterator cit(checks().begin()), cend(checks().end());
                 cit != cend; cit = checks().equal_range(cit->first).second) {
                llvm::errs() << "  check " << cit->first << " on\n";
            }
            break;
        }
    }

    typedef map_type::const_iterator const_iterator;
    for (const_iterator it(checks().begin()), end(checks().end());
         it != end; ++it)
    {
        checks_type::const_iterator cit(config.find(it->first));
        if ((config.end() == cit && analyser.config()->all()) ||
            cit->second == csabase::Config::on)
        {
            it->second(analyser, visitor, observer);
        }
    }
}
