// csastil_implicitctor.cpp                                           -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_registercheck.h>
#include <csabase_ppobserver.h>
#include <set>

// -----------------------------------------------------------------------------

static std::string const check_name("implicit-ctor");

// -----------------------------------------------------------------------------

namespace
{
    struct suppressions
    {
        std::set<cool::csabase::Location> entries_;
        std::vector<clang::Decl const*>   reports_;
    };
}

// -----------------------------------------------------------------------------

static void
check(cool::csabase::Analyser& analyser, clang::CXXConstructorDecl const* decl)
{
    if (decl->isConvertingConstructor(false)
        && !decl->isCopyOrMoveConstructor()
        && decl->isFirstDecl()
        && !llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(decl->getParent())
        ) {
        analyser.attachment<suppressions>().reports_.push_back(decl);
    }
}

// -----------------------------------------------------------------------------
        
namespace
{
    struct report
    {
        report(cool::csabase::Analyser& analyser): analyser_(&analyser) {}
        cool::csabase::Analyser* analyser_;
        void
        operator()()
        {
            cool::csabase::Analyser* analyser(analyser_);
            //-dk:TODO the suppression handling should be in a shared place!
            suppressions const& attachment(analyser->attachment<suppressions>());
            for (std::vector<clang::Decl const*>::const_iterator rit(attachment.reports_.begin()), rend(attachment.reports_.end());
                 rit != rend; ++rit) {
                clang::Decl const* decl(*rit);
                typedef cool::csabase::Location            Location;
                typedef std::set<Location>::const_iterator const_iterator;
                clang::SourceLocation end(decl->getLocStart());
                Location loc(analyser->get_location(end));
                const_iterator it(attachment.entries_.lower_bound(loc));
                clang::Decl const* next(decl->getNextDeclInContext());

#if 0
                llvm::errs() << "loc=" << loc.file() << ":" << loc.line() << ":" << loc.column() << "\n";
                if (it != attachment.entries_.end()) {
                    llvm::errs() << "*it=" << it->file() << ":" << it->line() << ":" << it->column() << "\n";
                }
                for (const_iterator it(attachment.entries_.begin()), end(attachment.entries_.end()); it != end; ++it) {
                    llvm::errs() << "all=" << it->file() << ":" << it->line() << ":" << it->column() << "\n";
                }
#endif

                if (it == attachment.entries_.end()
                    || (next
                        && analyser->get_location(next->getLocStart()) < *it
                        && loc < analyser->get_location(next->getLocation()) 
                        )
                    || it->file() != loc.file()
                    || (it->line() != loc.line() && it->line() != loc.line() + 1)
                    || it->column() != 69
                    ) {
                    analyser->report(decl, check_name, "constructor suitable for implicit conversions")
                        << decl->getSourceRange();
                }
            }
        }
    };
}

// -----------------------------------------------------------------------------

namespace
{
    struct comments
    {
        comments(cool::csabase::Analyser& analyser): analyser_(&analyser) {}
        cool::csabase::Analyser* analyser_;
        void
        operator()(clang::SourceRange range)
        {
            cool::csabase::Location location(analyser_->get_location(range.getBegin()));
            if (analyser_->is_component(location.file())) {
                std::string comment(analyser_->get_source(range));
                if (comment == "// IMPLICIT") {
                    analyser_->attachment<suppressions>().entries_.insert(location);
                }
            }
        }
    };
}

static void
subscribe(cool::csabase::Analyser& analyser, cool::csabase::Visitor&, cool::csabase::PPObserver& observer)
{
    analyser.onTranslationUnitDone += report(analyser);
    observer.onComment += comments(analyser);
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_check(check_name, &check);
static cool::csabase::RegisterCheck register_observer(check_name, &subscribe);
