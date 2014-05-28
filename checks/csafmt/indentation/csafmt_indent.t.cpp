#include <assert.h>

#define F() \
    do { \
        extern volatile int x; \
        ++x; \
    } while (false);

#define G()

int f()
    {
  F(); F();
      assert(
        2
      );
        G();
    F(
  );
        assert(7);
        return 0;
}

void g(int a, int b,
       int c)
{
}

void h(int a,
       int b, int c)
{
}

void i(int a, int b, int c)
{
}

void j(int a,
       int b,
      int c)
{
}
