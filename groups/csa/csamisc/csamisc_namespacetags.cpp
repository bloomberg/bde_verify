// csamisc_namespacetags.cpp                                          -*-C++-*-

#include <clang/AST/Decl.h>
#include <csabase_analyser.h>
#include <csabase_config.h>
#include <csabase_registercheck.h>
#include <string>

using namespace csabase;
using namespace clang;

// -----------------------------------------------------------------------------

static std::string const check_name("namespace-tags");

static void namespace_tags(Analyser& analyser, NamespaceDecl const *decl)
{
    SourceLocation rbr = decl->getRBraceLoc();
    if (   analyser.is_component(decl)
        && rbr.isValid()
        && analyser.manager().getPresumedLineNumber(decl->getLocation()) !=
           analyser.manager().getPresumedLineNumber(rbr)) {
        SourceRange line_range = analyser.get_line_range(rbr);
        line_range.setBegin(rbr);
        llvm::StringRef line = analyser.get_source(line_range, true);
        std::string nsname = decl->getNameAsString();
        std::string tag;
        if (decl->isAnonymousNamespace()) {
            tag = "unnamed";
        } else if (nsname == analyser.package()) {
            tag = "package";
        } else if (nsname == analyser.config()->toplevel_namespace()) {
            tag = "enterprise";
        }
        std::string s = tag.size() ? "}  // close " + tag + " namespace" :
                                     "}  // close namespace " + nsname;
        if (line != s) {
            auto report = analyser.report(rbr, check_name, "NT01",
                            "End of %0 namespace should be marked with \"%1\"");
            report << (nsname.size() ? nsname : "anonymous") << s;
            analyser.ReplaceText(line_range, s);
        }
    }
}

// -----------------------------------------------------------------------------

static RegisterCheck check(check_name, &namespace_tags);

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
