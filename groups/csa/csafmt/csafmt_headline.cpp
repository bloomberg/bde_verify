// csafmt_headline.cpp                                                -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_filenames.h>
#include <csabase_registercheck.h>
#include <csabase_ppobserver.h>
#include <csabase_util.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <fstream>
#include <string>

static std::string const check_name("headline");

// ----------------------------------------------------------------------------

namespace
{
    template <typename T>
    struct analyser_binder
    {
        analyser_binder(void (*function)(csabase::Analyser&,
                                         clang::SourceLocation,
                                         T,
                                         std::string const&),
                        csabase::Analyser& analyser):
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
        void          (*function_)(csabase::Analyser&,
                                   clang::SourceLocation,
                                   T,
                                   std::string const&);
        csabase::Analyser& analyser_;
    };
}

// ----------------------------------------------------------------------------

static void
open_file(csabase::Analyser& analyser,
          clang::SourceLocation    where, 
          const std::string&       ,
          const std::string&       name)
{
    csabase::FileName fn(name);
    std::string filename = fn.name();
    if (analyser.is_component_header(filename) ||
        name == analyser.toplevel()) {
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
            std::pair<size_t, size_t> mcpp =
                csabase::mid_mismatch(buf, expectcpp);
            std::pair<size_t, size_t> mc =
                csabase::mid_mismatch(buf, expectc);
            std::pair<size_t, size_t> m;
            std::string expect;

            if (mcpp.first >= mc.first || mcpp.second >= mc.second) {
                m = mcpp;
                expect = expectcpp;
            } else {
                m = mc;
                expect = expectc;
            }
            analyser.report(where.getLocWithOffset(m.first),
                            check_name, "HL01",
                            "File headline incorrect", true);
            analyser.report(where.getLocWithOffset(m.first),
                            check_name, "HL01",
                            "Correct format is\n%0",
                            true, clang::DiagnosticsEngine::Note)
                << expect;
            if (buf.size() > 0) {
                analyser.rewriter().ReplaceText(
                    where.getLocWithOffset(m.first),
                    buf.size() - m.first - m.second + 1,
                    llvm::StringRef(expect).
                        drop_front(m.first).
                        drop_back(m.second - 1));
            }
        }
    }
}

// ----------------------------------------------------------------------------

static void
subscribe(csabase::Analyser& analyser,
          csabase::Visitor&  ,
          csabase::PPObserver& observer)
{
    observer.onOpenFile += analyser_binder<std::string const&>(open_file,
                                                                 analyser);
}

// ----------------------------------------------------------------------------

static csabase::RegisterCheck register_observer(check_name,&subscribe);

