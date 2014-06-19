void f()
{
    char aaaa;
    int bb;

    double ddddd;
    float ee;
    unsigned gggggg;

    int hhhhhhhhhh;
    int iii;

    int kkkkkkk, llllll;

    char *mmmm;
    int nnnnnn;
    double& ooooooooo = ddddd;

    char   *pppppppppp;
    int     qqqqqqqq;
    double& rrrrrrrrrr = ooooooooo;
}

struct x
{
    char aaaa;
    int bb;

    double ddddd;
    float ee;
    unsigned gggggg;

    int hhhhhhhhhh;
    int iii;

    int kkkkkkk, llllll;

    char *mmmm;
    int nnnnnn;
    double& ooooooooo = ddddd;

    char   *pppppppppp;
    int     qqqqqqqq;
    double& rrrrrrrrrr = ooooooooo;
};

char aaaa;
int bb;

double ddddd;
float ee;
unsigned gggggg;

int hhhhhhhhhh;
int iii;

int kkkkkkk, llllll;

char *mmmm;
int nnnnnn;
double& ooooooooo = ddddd;

char   *pppppppppp;
int     qqqqqqqq;
double& rrrrrrrrrr = ooooooooo;

void g()
{
    for (int i = 0; i < 10; ++i) {
        volatile int j = i * i;
    }

    struct xxx { };
    double d;
    class z { } a;
    int y;
}
