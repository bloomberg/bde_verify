// csamisc_auto.t.cpp                                                 -*-C++-*-

#if 0
template <typename T>
struct general_iterator
{
    void operator++();
    bool operator!= (general_iterator const&) const;
    T& operator*();
};

template <typename T>
struct vector
{
#if 0
    struct iterator
    {
        void operator++();
        bool operator!= (iterator const&) const;
        T& operator*();
    };
#else
    typedef general_iterator<T> iterator;
#endif
    iterator begin();
    iterator end();
};

int main()
{
    int  i = 17;
    auto var = 17;
    vector<int> v;
    for (auto it = v.begin(); it != v.end(); ++it) {
        auto& r = *it;
    }
}
#endif
