#define A(X) { aSsErT(!(X), #X, __LINE__); }
#define AV(...) LOOPN_A(NA(__VA_ARGS__), __VA_ARGS__)
#define BSLS_A(X) BSLS_A_A(X)
#define BSLS_A_A(X) \
do { if ((!(X))) { invoke(#X, __FILE__, __LINE__); } } while (false)
#define BBTU_A(X) { aSsErT(!(X), #X, __LINE__); }
#define BBTU_AV(...) BBTU_LOOPN_A(BBTU_NA(__VA_ARGS__), __VA_ARGS__)
#define BBTU_EX(X) X
#define BBTU_LOOP0_A BBTU_A
#define BBTU_LOOP3_A(I,J,K,X) { aSsErT(!(X), #X, __LINE__); }
#define BBTU_LOOPN_A(N, ...) BBTU_LOOPN_A_IMPL(N, __VA_ARGS__)
#define BBTU_LOOPN_A_IMPL(N, ...) BBTU_EX(BBTU_LOOP##N##_A(__VA_ARGS__))
#define BBTU_NA(...) BBTU_EX(NA_IMPL(__VA_ARGS__, 5, 4, 3, 2, 1, 0, ""))
#define LOOP0_A(X) { if (!(X)) { aSsErT(1, #X, __LINE__); } }
#define LOOP3_A(I,J,K,X) { if (!(X)) { aSsErT(1, #X, __LINE__); } }
#define LOOPN_A(N, ...) LOOPN_A_IMPL(N, __VA_ARGS__)
#define LOOPN_A_IMPL(N, ...) LOOP ## N ## _A(__VA_ARGS__)
#define LOOP_A(I,X) { if (!(X)) { aSsErT(1, #X, __LINE__); }}
#define NA(...) NA_IMPL(__VA_ARGS__, 5, 4, 3, 2, 1, 0, "")
#define NA_IMPL(X5, X4, X3, X2, X1, X0, N, ...) N

void invoke(const char *, const char *, int);
void aSsErT(bool, const char *, int);

int main(int c, char **v)
{
    BSLS_A(c = 1);
    BBTU_AV(c = 1);
    BBTU_AV(1, 2, 3, c = 1);
    AV(c = 1);
    AV(1, 2, 3, c = 1);
}
