// csafmt_headline.cpp                                                -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
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
            this->function_(this->analyser_, where, arg, name);
        }
        void          (*function_)(cool::csabase::Analyser&,
                                   clang::SourceLocation,
                                   T,
                                   std::string const&);
        cool::csabase::Analyser& analyser_;
    };
}

// ----------------------------------------------------------------------------

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
    if (analyser.is_component_header(filename) || name == analyser.toplevel()) {
        const clang::SourceManager &m = analyser.manager();
        llvm::StringRef buf = m.getBuffer(m.getFileID(where))->getBuffer();
        buf = buf.substr(0, buf.find('\n'));
        std::string expect("// " + filename);
        expect.resize(70, ' ');
        expect += "-*-C++-*-";

        if (!buf.equals(expect) && buf.find("GENERATED") == buf.npos) {
            int i = 0;
            while (buf[i] == expect[i]) {
                ++i;
            }
            int j = 0;
            while (j > i &&
                   buf[buf.size() - j - 1] == expect[expect.size() - j - 1]) {
                --j;
            }
            analyser.report(where.getLocWithOffset(i),
                            check_name,
                            "HL01: file headline incorrect",
                            true)
                << clang::FixItHint::CreateReplacement(
                    clang::SourceRange(
                        where.getLocWithOffset(i),
                        where.getLocWithOffset(buf.size() - j)),
                    llvm::StringRef(&expect[i], expect.size() - i - j));
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

