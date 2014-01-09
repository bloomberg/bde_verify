// csafmt_headline.cpp                                                -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_filenames.h>
#include <csabase_registercheck.h>
#include <csabase_ppobserver.h>
#include <fstream>
#include <string>

static std::string const check_name("headline");

// ----------------------------------------------------------------------------

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
            function_(analyser_, where, arg, name);
        }
        void          (*function_)(cool::csabase::Analyser&,
                                   clang::SourceLocation,
                                   T,
                                   std::string const&);
        cool::csabase::Analyser& analyser_;
    };
}

// ----------------------------------------------------------------------------

static std::pair<unsigned, unsigned>
mismatch(const std::string &have, const std::string &want)
{
    std::pair<unsigned, unsigned> result(0, 0);
    while (   result.first < have.size()
           && result.first < want.size()
           && have[result.first] == want[result.first]) {
        ++result.first;
    }
    while (   result.second < have.size()
           && result.second < want.size()
           && have.size() - result.second - 1 >= result.first
           && have[have.size() - result.second - 1] ==
              want[want.size() - result.second - 1]) {
        ++result.second;
    }
    return result;
}

// ----------------------------------------------------------------------------

static void
open_file(cool::csabase::Analyser& analyser,
          clang::SourceLocation    where, 
          const std::string&       ,
          const std::string&       name)
{
    cool::csabase::FileName fn(name);
    std::string filename = fn.name();
    if (analyser.is_component_header(filename) || name == analyser.toplevel()) {
        const clang::SourceManager &m = analyser.manager();
        llvm::StringRef buf = m.getBuffer(m.getFileID(where))->getBuffer();
        buf = buf.substr(0, buf.find('\n')).rtrim();
        std::string expectcpp("// " + filename);
        expectcpp.resize(70, ' ');
        expectcpp += "-*-C++-*-";
        std::string expectc("/* " + filename);
        expectc.resize(69, ' ');
        expectc += "-*-C-*- */";

        if (   !buf.equals(expectcpp)
            && !buf.equals(expectc)
            && buf.find("GENERATED") == buf.npos) {
            std::pair<unsigned, unsigned> mcpp = mismatch(buf, expectcpp);
            std::pair<unsigned, unsigned> mc   = mismatch(buf, expectc);
            std::pair<unsigned, unsigned> m;
            std::string expect;

            if (mcpp.first >= mc.first || mcpp.second <= mc.second) {
                m = mcpp;
                expect = expectcpp;
            } else {
                m = mc;
                expect = expectc;
            }
            analyser.report(where.getLocWithOffset(m.first),
                            check_name,
                            "HL01: file headline incorrect",
                            true)
                << clang::FixItHint::CreateReplacement(
                    clang::SourceRange(
                        where.getLocWithOffset(m.first),
                        where.getLocWithOffset(buf.size() - m.second)),
                    expect.substr(m.first,
                                  expect.size() - m.first - m.second));
        }
    }
}

// ----------------------------------------------------------------------------

static void
subscribe(cool::csabase::Analyser& analyser,
          cool::csabase::Visitor&  ,
          cool::csabase::PPObserver& observer)
{
    observer.onOpenFile += analyser_binder<std::string const&>(open_file,
                                                                 analyser);
}

// ----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_observer(check_name,&subscribe);

