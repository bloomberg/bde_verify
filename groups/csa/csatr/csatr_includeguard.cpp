// csatr_includeguard.cpp                                             -*-C++-*-

#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <cctype>

// ----------------------------------------------------------------------------

static std::string const check_name("include-guard");

// -----------------------------------------------------------------------------

namespace
{
    char my_toupper(unsigned char c)
    {
        return std::toupper(c);
    }
    bool my_isspace(unsigned char c)
    {
        return std::isspace(c);
    }

    struct include_guard
    {
        include_guard()
            : d_test()
            , d_define()
        {
        }
        bool isComplete() const {
            return d_test && d_define;
        }
        std::string const& get_expect(cool::csabase::Analyser* analyser)
        {
            if (d_expect.empty()) {
                d_expect = "INCLUDED_" + analyser->component();
                std::transform(d_expect.begin(), d_expect.end(),
                               d_expect.begin(),
                               my_toupper);
            }
            return d_expect;
        }
        std::string d_expect;
        bool        d_test;
        bool        d_define;
    };

    struct binder
    {
        binder(cool::csabase::Analyser* analyser)
            : d_analyser(analyser)
        {
        }

        void operator()(clang::SourceLocation,
                        clang::SourceRange range) const // onIf
        {
            if (!d_analyser->is_component_header(range.getBegin())) {
                return;
            }
            include_guard& data(d_analyser->attachment<include_guard>());
            if (data.d_test) {
                return;
            }
            char const* begin(d_analyser->manager().getCharacterData(range.getBegin()));
            char const* end(d_analyser->manager().getCharacterData(range.getEnd()));
            std::string value(begin, end);
            value.erase(std::remove_if(value.begin(), value.end(),
                                       &my_isspace), value.end());
            std::string const& expect(data.get_expect(d_analyser));
            if (value != "!defined(" + expect + ")"
                && value != "!defined" + expect) {
                d_analyser->report(range.getBegin(), check_name, "TR14",
                                         "Wrong include guard "
                                         "(expected '!defined(%0)')")
                    << expect;
            }
            data.d_test = true;
        }

        void operator()(clang::SourceLocation where,
                        clang::Token const& token) const  // onIfndef
        {
            if (!d_analyser->is_component_header(token.getLocation())) {
                return;
            }

            //-dk:TODO llvm::errs() << "  in header\n";
            include_guard& data(d_analyser->attachment<include_guard>());
            if (clang::IdentifierInfo const* id
                = data.d_test? 0: token.getIdentifierInfo())
            {
                std::string value(id->getNameStart());
                //-dk:TODO llvm::errs() << "  vlaue='" << value << "'\n";
                std::string const& expect(data.get_expect(d_analyser));
                if (value != expect) {
                    d_analyser->report(token.getLocation(), check_name, "TR14",
                                             "Wrong name for include guard "
                                             "(expected '%0')")
                        << expect;
                }
                data.d_test = true;
            }
        }

        void operator()(clang::Token const& token,
                        clang::MacroDirective const*) const
        {
            include_guard& data(d_analyser->attachment<include_guard>());
            if (d_analyser->is_component_header(token.getLocation())
                && !data.d_define
                && token.getIdentifierInfo()
                && (std::string(token.getIdentifierInfo()->getNameStart())
                    == data.get_expect(d_analyser))
                ) {
                data.d_define = true;
            }
        }

        void operator()(clang::SourceLocation location,
                        std::string const&,
                        std::string const& filename) const
        {
            if (d_analyser->is_component_header(filename) &&
                !d_analyser->attachment<include_guard>().isComplete()) {
                include_guard const& data(
                    d_analyser->attachment<include_guard>());
                d_analyser->report(location, check_name, "TR14",
                                   data.d_test ?
                                        "Missing define for include guard %0"
                                   : data.d_define ?
                                        "Missing test for include guard %0"
                                   :    "Missing include guard %0")
                    << d_analyser->attachment<include_guard>().
                                                        get_expect(d_analyser);
            }
        }

        cool::csabase::Analyser* d_analyser;
    };
}

// -----------------------------------------------------------------------------

static void
subscribe(cool::csabase::Analyser&   analyser,
          cool::csabase::Visitor&    ,
          cool::csabase::PPObserver& observer)
{
    observer.onIfndef       += binder(&analyser);
    observer.onIf           += binder(&analyser);
    observer.onMacroDefined += binder(&analyser);
    observer.onCloseFile    += binder(&analyser);
}

// ----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_observer(check_name, &subscribe);
