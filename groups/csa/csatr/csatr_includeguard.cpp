// csatr_includeguard.cpp                                             -*-C++-*-

#include <csabase_analyser.h>
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
            return this->d_test && this->d_define;
        }
        std::string const& get_expect(cool::csabase::Analyser* analyser)
        {
            if (this->d_expect.empty()) {
                d_expect = "INCLUDED_"
                    + analyser->package() + "_"
                    + analyser->component();
                std::transform(this->d_expect.begin(), this->d_expect.end(),
                               this->d_expect.begin(),
                               my_toupper);
            }
            return this->d_expect;
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
            if (!this->d_analyser->is_component_header(range.getBegin())) {
                return;
            }
            include_guard& data(this->d_analyser->attachment<include_guard>());
            if (data.d_test) {
                return;
            }
            char const* begin(this->d_analyser->manager().getCharacterData(range.getBegin()));
            char const* end(this->d_analyser->manager().getCharacterData(range.getEnd()));
            std::string value(begin, end);
            value.erase(std::remove_if(value.begin(), value.end(),
                                       &my_isspace), value.end());
            std::string const& expect(data.get_expect(this->d_analyser));
            if (value != "!defined(" + expect + ")"
                && value != "!defined" + expect) {
                this->d_analyser->report(range.getBegin(), check_name,
                                         "TR14: wrong include guard "
                                         "(expected '!defined(%0)')")
                    << expect;
            }
            data.d_test = true;
        }
        void operator()(clang::SourceLocation where,
                        clang::Token const& token) const // onIfndef
        {
#if 0
            //-dk:TODO
            llvm::errs() << "ifndef: "
                         << "where=" << this->d_analyser->get_location(where) << " "
                         << "token=" << this->d_analyser->get_location(token.getLocation()) << " "
                         << "\n";
            if (clang::IdentifierInfo const* id = token.getIdentifierInfo())
            {
                llvm::errs() << "  value1='" << id->getNameStart() << "'\n";
            }
#endif
            if (!this->d_analyser->is_component_header(token.getLocation())) {
                return;
            }

            //-dk:TODO llvm::errs() << "  in header\n";
            include_guard& data(this->d_analyser->attachment<include_guard>());
            if (clang::IdentifierInfo const* id
                = data.d_test? 0: token.getIdentifierInfo())
            {
                std::string value(id->getNameStart());
                //-dk:TODO llvm::errs() << "  vlaue='" << value << "'\n";
                std::string const& expect(data.get_expect(this->d_analyser));
                if (value != expect) {
                    this->d_analyser->report(token.getLocation(), check_name,
                                             "TR14: wrong name for include guard "
                                             "(expected '%0')")
                        << expect;
                }
                data.d_test = true;
            }
        }
        void operator()(clang::Token const& token, clang::MacroDirective const*) const
        {
            include_guard& data(this->d_analyser->attachment<include_guard>());
            if (this->d_analyser->is_component_header(token.getLocation())
                && !data.d_define
                && token.getIdentifierInfo()
                && (std::string(token.getIdentifierInfo()->getNameStart())
                    == data.get_expect(this->d_analyser))
                ) {
                data.d_define = true;
            }
        }
        void operator()(clang::SourceLocation location,
                        std::string const&,
                        std::string const& filename) const
        {
            if (this->d_analyser->is_component_header(filename)
                && !this->d_analyser->attachment<include_guard>().isComplete())
            {
                include_guard const& data(this->d_analyser->attachment<include_guard>());
                this->d_analyser->report(location, check_name,
                                         data.d_test
                                         ? "TR14: missing define for include guard"
                                         : data.d_define
                                         ? "TR14: missing test for include guard"
                                         : "TR14: missing include guard");
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
