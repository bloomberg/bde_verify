// csabase_checkregistry.h                                            -*-C++-*-

#ifndef INCLUDED_CSABASE_CHECKREGISTRY
#define INCLUDED_CSABASE_CHECKREGISTRY

#include <utils/function.hpp>
#include <string>

// -----------------------------------------------------------------------------

namespace csabase { class Analyser; }
namespace csabase { class Visitor; }
namespace csabase { class PPObserver; }
namespace csabase
{
class CheckRegistry
    // This class maintains a list of all the registered checks.  Essentially,
    // it is a map of (name, function) pairs where the function does the
    // necessary operations to subscribe to the suitable events on the visitor
    // object it gets passed.
{
  public:
    typedef utils::function<void(Analyser&, Visitor&, PPObserver&)> Subscriber;
    static void add_check(std::string const&, Subscriber);
    static void attach(Analyser&, Visitor&, PPObserver&);
};
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
