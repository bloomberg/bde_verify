// csabase_util.cpp                                                   -*-C++-*-
#include <csabase_util.h>
#include <csabase_location.h>
#include <csabase_debug.h>

namespace cool {
namespace csabase {

std::pair<unsigned, unsigned>
mid_mismatch(const std::string &have, const std::string &want)
{
    std::pair<unsigned, unsigned> result(0, 0);
    while (   result.first < have.size()
           && result.first < want.size()
           && have[result.first] == want[result.first]) {
        ++result.first;
    }
    while (   result.second < have.size()
           && result.second < want.size()
           && have.size() > result.first + result.second
           && have[have.size() - result.second - 1] ==
              want[want.size() - result.second - 1]) {
        ++result.second;
    }
    return result;
}

std::pair<unsigned, unsigned>
mid_match(const std::string &have, const std::string &want)
{
    std::pair<unsigned, unsigned> result(have.find(want), have.npos);
    if (result.first != have.npos) {
        result.second = have.size() - want.size() - result.first;
    }
    return result;
}

bool areConsecutive(clang::SourceManager& manager,
                    clang::SourceRange    first,
                    clang::SourceRange    second)
{
    clang::FileID fidf = manager.getFileID(first.getEnd());
    clang::FileID fids = manager.getFileID(second.getBegin());
    unsigned colf = manager.getPresumedColumnNumber(first.getBegin());
    unsigned cols = manager.getPresumedColumnNumber(second.getBegin());
    unsigned offf = manager.getFileOffset(first.getEnd());
    unsigned offs = manager.getFileOffset(second.getBegin());

    return fidf == fids && colf == cols && offf <= offs &&
           llvm::StringRef::npos == manager.getBufferData(fidf)
                                        .substr(offf, offs - offf)
                                        .find_first_not_of(" \t\n\r\f\v");
}

std::string to_lower(std::string s)
{
    std::string::iterator b = s.begin();
    std::string::iterator e = s.end();
    for (std::string::iterator i = b; i != e; ++i) {
        *i = std::tolower(static_cast<unsigned char>(*i));
    }
    return s;
}

}
}
