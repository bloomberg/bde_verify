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

using namespace csabase;

// -----------------------------------------------------------------------------

namespace
{

typedef std::multimap<std::string, csabase::CheckRegistry::Subscriber>
map_type;

map_type& checks()
{
    static map_type rc;
    return rc;
}

}

// -----------------------------------------------------------------------------

void
csabase::CheckRegistry::add_check(std::string const& name,
                                  CheckRegistry::Subscriber check)
{
    checks().insert(std::make_pair(name, check));
}

// -----------------------------------------------------------------------------

void
csabase::CheckRegistry::attach(Analyser& analyser,
                               Visitor& visitor,
                               PPObserver& observer)
{
    typedef map_type::const_iterator const_iterator;
    typedef std::map<std::string, Config::Status> checks_type;
    checks_type const& config(analyser.config()->checks());
    for (const auto &cfg : config) {
        if (checks().find(cfg.first) == checks().end()) {
            llvm::errs() << "unknown check '" << cfg.first << "'; "
                         << "existing checks:\n";
            for (const_iterator cit(checks().begin()), cend(checks().end());
                 cit != cend; cit = checks().equal_range(cit->first).second) {
                llvm::errs() << "  check " << cit->first << " on\n";
            }
            break;
        }
    }

    for (const auto& check : checks()) {
        checks_type::const_iterator cit(config.find(check.first));
        if ((config.end() != cit && cit->second == csabase::Config::on) ||
            (config.end() == cit && analyser.config()->all())) {
            check.second(analyser, visitor, observer);
        }
    }
}
