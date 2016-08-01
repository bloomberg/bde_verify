bool checkErr(bool, int, int);

#define CHECK_ERR(HAS_ERR, CODE, STATUS)                                      \
    do {                                                                      \
        if (checkErr(HAS_ERR, CODE, STATUS)) {                                \
            return 0;                                                         \
        }                                                                     \
    } while (0)

int f(int x)
{
    if (x < 0) {
                                                    CHECK_ERR(x < 0, 17, 19);
                                                    CHECK_ERR(x < 0, 17, 19);
                                                                      // RETURN
                                                    CHECK_ERR(x < 0, 17, 19);
                                                                  // RETURN
        CHECK_ERR(x < 0, 17, 19);                 // RETURN
        CHECK_ERR(x < 0, 17, 19);                                     // RETURN
    } else {
        CHECK_ERR(x > 0, 17, 19);
        CHECK_ERR(x > 0, 17, 19);                 // RETURN
        CHECK_ERR(x > 0, 17, 19);                                     // RETURN
                                                    CHECK_ERR(x > 0, 17, 19);
                                                                      // RETURN
                                                    CHECK_ERR(x > 0, 17, 19);
                                                                  // RETURN
                                                    CHECK_ERR(x > 0, 17, 19);
    }
    // RETURN
    return x;
}
