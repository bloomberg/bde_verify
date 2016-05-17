// csabde_tool.cpp                                                    -*-C++-*-

#include <clang/Frontend/FrontendPluginRegistry.h>
#include <csabase_analyse.h>
#include <csabase_tool.h>

using namespace csabase;
using namespace clang;

int main(int argc, const char **argv)
{
    return run(argc, argv);
}

// ----------------------------------------------------------------------------

FrontendPluginRegistry::Add<PluginAction>
registerPlugin("bde_verify", "analyse source");

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
