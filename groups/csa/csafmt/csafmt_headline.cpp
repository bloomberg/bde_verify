// csafmt_headline.cpp                                                -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_registercheck.h>
#include <csabase_ppobserver.h>
#include <fstream>
#include <string>

static std::string const check_name("headline");

// -----------------------------------------------------------------------------

namespace
{
    template <typename T>
    struct analyser_binder
    {
        analyser_binder(void (*function)(cool::csabase::Analyser&,
                                         clang::SourceLocation,
                                         T,
                                         std::string const&),
                        cool::csabase::Analyser& analyser):
            function_(function),
            analyser_(analyser)
        {
        }
        void operator()(clang::SourceLocation where,
                        T                     arg,
                        std::string const&    name) const
        {
            this->function_(this->analyser_, where, arg, name);
        }
        void          (*function_)(cool::csabase::Analyser&,
                                   clang::SourceLocation,
                                   T,
                                   std::string const&);
        cool::csabase::Analyser& analyser_;
    };
}

// -----------------------------------------------------------------------------

static void
open_file(cool::csabase::Analyser& analyser,
          clang::SourceLocation    where, 
          const std::string&       ,
          const std::string&       name)
{
    std::string::size_type slash(name.rfind('/'));
    std::string            filename(slash == name.npos
                                    ? name
                                    : name.substr(slash + 1));
    if (analyser.is_component_header(filename)
        || name == analyser.toplevel()) {
        std::string line;
        if (std::getline(std::ifstream(name.c_str()) >> std::noskipws, line)) {
            std::string expect("// " + filename);
            expect.resize(70, ' ');
            expect += "-*-C++-*-";
            if (line != expect && line.find("GENERATED") == line.npos) {
                analyser.report(where,
                                ::check_name,
                                "file headline incorrect",
                                true);
            }
        }
        else {
            analyser.report(where, ::check_name,
                            "failed to open file '" + name + "' for reading");
        }
    }
}

// -----------------------------------------------------------------------------

static void
subscribe(cool::csabase::Analyser& analyser,
          cool::csabase::Visitor&  ,
          cool::csabase::PPObserver& observer)
{
    observer.onOpenFile += ::analyser_binder<std::string const&>(::open_file,
                                                                 analyser);
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_observer(check_name, &::subscribe);

