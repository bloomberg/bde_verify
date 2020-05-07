// csabase_checkregistry.cpp                                          -*-C++-*-

#include <csabase_checkregistry.h>
#include <csabase_analyser.h>
#include <csabase_config.h>
#include <csabase_debug.h>
#include <llvm/Support/raw_ostream.h>
#include <map>
#include <utility>

using namespace csabase;

// -----------------------------------------------------------------------------

namespace
{
typedef std::multimap<std::string, CheckRegistry::Subscriber> map_type;

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
    if (name.size() > 0 && name.front() == '.') {
        add_check(name.substr(1), check);
    }
    else {
        checks().insert(std::make_pair(name, check));
    }
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
        if (checks().find(cfg.first) == checks().end() &&
            checks().find("." + cfg.first) == checks().end()) {
            llvm::errs() << "unknown check '" << cfg.first << "'; "
                         << "existing checks:\n";
            for (const_iterator cit(checks().begin()), cend(checks().end());
                 cit != cend; cit = checks().equal_range(cit->first).second) {
                if (cit->first[0] != '\0' && cit->first[0] != '.') {
                    llvm::errs() << "  check " << cit->first << " on\n";
                }
            }
            break;
        }
    }

    for (const auto& check : checks()) {
        checks_type::const_iterator cit(config.find(check.first));
        if (cit == config.end()) {
            cit = config.find("." + check.first);
        }
        if (check.first.size() == 0 ||
            (config.end() != cit && cit->second == Config::on) ||
            (config.end() == cit && analyser.config()->all())) {
            check.second(analyser, visitor, observer);
        }
    }
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
