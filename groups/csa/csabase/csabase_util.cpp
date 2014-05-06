// csabase_util.cpp                                                   -*-C++-*-
#include <csabase_util.h>
#include <csabase_location.h>
#include <csabase_debug.h>
#include <llvm/Support/Regex.h>

namespace csabase {

std::pair<size_t, size_t>
mid_mismatch(const std::string &have, const std::string &want)
{
    std::pair<size_t, size_t> result(0, 0);
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

std::pair<size_t, size_t>
mid_match(const std::string &have, const std::string &want)
{
    std::pair<size_t, size_t> result(have.find(want), have.npos);
    if (result.first != have.npos) {
        result.second = have.size() - want.size() - result.first;
    }
    return result;
}

static llvm::Regex between_comments(
    "^[[:blank:]]*[[:space:]]?[[:blank:]]*$",
    llvm::Regex::NoFlags);

bool areConsecutive(clang::SourceManager& manager,
                    clang::SourceRange    first,
                    clang::SourceRange    second)
{
    clang::FileID fidf = manager.getFileID(first.getEnd());
    clang::FileID fids = manager.getFileID(second.getBegin());
    size_t colf = manager.getPresumedColumnNumber(first.getBegin());
    size_t cols = manager.getPresumedColumnNumber(second.getBegin());
    size_t offf = manager.getFileOffset(first.getEnd());
    size_t offs = manager.getFileOffset(second.getBegin());

    return fidf == fids && colf == cols && offf <= offs &&
           between_comments.match(
               manager.getBufferData(fidf).substr(offf, offs - offf));
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

} // close package namespace

