// csastil_implicitctor.cpp                                           -*-C++-*-

#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/Basic/SourceLocation.h>
#include <csabase_analyser.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Casting.h>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <set>
#include <string>
#include <vector>

namespace csabase { class Visitor; }

using namespace csabase;
using namespace clang;

// -----------------------------------------------------------------------------

static std::string const check_name("implicit-ctor");

// -----------------------------------------------------------------------------

namespace
{
    struct suppressions
    {
        std::set<Location>       entries_;
        std::vector<Decl const*> reports_;
    };
}

// -----------------------------------------------------------------------------

static void check(Analyser& analyser, CXXConstructorDecl const* decl)
{
    if (analyser.is_component(decl)
        && decl->isConvertingConstructor(false)
        && !decl->isCopyOrMoveConstructor()
        && decl->isFirstDecl()
        && !llvm::dyn_cast<ClassTemplateSpecializationDecl>(decl->getParent())
        ) {
        analyser.attachment<suppressions>().reports_.push_back(decl);
    }
}

// -----------------------------------------------------------------------------
        
namespace
{
    struct report
    {
        report(Analyser& analyser): analyser_(&analyser) {}

        void operator()()
        {
            //-dk:TODO the suppression handling should be in a shared place!
            suppressions const& attachment(
                analyser_->attachment<suppressions>());
            for (Decl const* decl : attachment.reports_) {
                Location declB(analyser_->get_location(decl->getLocStart()));
                Location declE(analyser_->get_location(decl->getLocEnd()));
                auto it(attachment.entries_.lower_bound(declB));
                Decl const* next(decl->getNextDeclInContext());

                if (it == attachment.entries_.end() ||
                    (next &&
                     analyser_->get_location(next->getLocStart()) < *it &&
                     declB < analyser_->get_location(next->getLocation())) ||
                    it->file() != declB.file() ||
                    it->line() < declB.line() ||
                    it->line() > declE.line() + 1 ||
                    it->column() != 69) {
                    analyser_->report(decl, check_name, "IEC01",
                            "Constructor suitable for implicit conversions")
                        << decl->getSourceRange();
                }
            }
        }

        Analyser* analyser_;
    };
}

// -----------------------------------------------------------------------------

namespace
{
    struct tags
    {
        tags(Analyser& analyser) : analyser_(&analyser)
        {
        }

        void operator()(SourceRange range)
        {
            Location location(analyser_->get_location(range.getBegin()));
            if (analyser_->is_component(location.file())) {
                std::string comment(analyser_->get_source(range));
                if (comment == "// IMPLICIT") {
                    analyser_->attachment<suppressions>().entries_.insert(
                        location);
                }
            }
        }

        Analyser* analyser_;
    };
}

static void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
{
    analyser.onTranslationUnitDone += report(analyser);
    observer.onComment += tags(analyser);
}

// -----------------------------------------------------------------------------

static RegisterCheck register_check(check_name, &check);
static RegisterCheck register_observer(check_name, &subscribe);

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
