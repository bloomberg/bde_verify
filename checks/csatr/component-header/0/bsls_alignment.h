#include <bsls_ident.h>
namespace BloombergLP {
namespace bsls {
struct Alignment {
    enum Strategy {
        BSLS_MAXIMUM = 0,
        BSLS_NATURAL = 1,
        BSLS_BYTEALIGNED = 2
    };
    static const char *toAscii(Alignment::Strategy value);
};
}  
}  
