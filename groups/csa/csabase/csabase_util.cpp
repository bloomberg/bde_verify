// csabase_util.cpp                                                   -*-C++-*-
#include <csabase_util.h>
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

bool areConsecutive(clang::SourceManager& manager,
                    clang::SourceRange    first,
                    clang::SourceRange    second) {
    bool result = false;

    clang::FileID fidf = manager.getFileID(first.getEnd());
    clang::FileID fids = manager.getFileID(second.getBegin());

    if (fidf == fids) {
        unsigned offf = manager.getFileOffset(first.getEnd());
        unsigned offs = manager.getFileOffset(second.getBegin());

        if (offf <= offs) {
            result =
                llvm::StringRef::npos == manager.getBufferData(fidf)
                                             .substr(offf, offs - offf)
                                             .find_first_not_of(" \t\n\r\f\v");
        }
    }
    return result;
}

}
}
