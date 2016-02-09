template <typename T> void g(T);

void f(long &long_val)
{
    g((long *)&long_val);
    g((float *)&long_val);
    g((char *)&long_val);
    g((long &)long_val);
    g((float &)long_val);
    g((char &)long_val);
    g((const long &)long_val);
    g((const float &)long_val);
    g((const char &)long_val);
    g((const long *)&long_val);
    g((const float *)&long_val);
    g((const char *)&long_val);
    g((long *)long_val);
    g((float *)long_val);
    g((char *)long_val);
}

void f(long *long_ptr)
{
    g((long *)&long_ptr);
    g((float *)&long_ptr);
    g((char *)&long_ptr);
    g((long &)long_ptr);
    g((float &)long_ptr);
    g((char &)long_ptr);
    g((const long &)long_ptr);
    g((const float &)long_ptr);
    g((const char &)long_ptr);
    g((const long *)&long_ptr);
    g((const float *)&long_ptr);
    g((const char *)&long_ptr);
    g((long *)long_ptr);
    g((float *)long_ptr);
    g((char *)long_ptr);
}

void f(long **long_ptr_ptr)
{
    g((long *)&long_ptr_ptr);
    g((float *)&long_ptr_ptr);
    g((char *)&long_ptr_ptr);
    g((long &)long_ptr_ptr);
    g((float &)long_ptr_ptr);
    g((char &)long_ptr_ptr);
    g((const long &)long_ptr_ptr);
    g((const float &)long_ptr_ptr);
    g((const char &)long_ptr_ptr);
    g((const long *)&long_ptr_ptr);
    g((const float *)&long_ptr_ptr);
    g((const char *)&long_ptr_ptr);
    g((long *)long_ptr_ptr);
    g((float *)long_ptr_ptr);
    g((char *)long_ptr_ptr);
}

void f(long &long_val, long *long_ptr, long **long_ptr_ptr)
{
    g((char (&)[1][1])long_val);
    g((char (*)[1][1])long_val);
    g((char (&)[1][1])long_ptr);
    g((char (*)[1][1])long_ptr);
    g((char (&)[1][1])long_ptr_ptr);
    g((char (*)[1][1])long_ptr_ptr);
    g((int (&)[1][1])long_val);
    g((int (*)[1][1])long_val);
    g((int (&)[1][1])long_ptr);
    g((int (*)[1][1])long_ptr);
    g((int (&)[1][1])long_ptr_ptr);
    g((int (*)[1][1])long_ptr_ptr);
    g((long (&)[1][1])long_val);
    g((long (*)[1][1])long_val);
    g((long (&)[1][1])long_ptr);
    g((long (*)[1][1])long_ptr);
    g((long (&)[1][1])long_ptr_ptr);
    g((long (*)[1][1])long_ptr_ptr);
    g((signed long *)&long_val);
    g((unsigned long *)&long_val);
    g((signed long &)long_val);
    g((unsigned long &)long_val);
    g((const signed long &)long_val);
    g((const unsigned long &)long_val);
    g((const signed long *)&long_val);
    g((const unsigned long *)&long_val);
    g((signed long *)long_val);
    g((unsigned long *)long_val);
    g((signed long *)&long_ptr);
    g((unsigned long *)&long_ptr);
    g((signed long &)long_ptr);
    g((unsigned long &)long_ptr);
    g((const signed long &)long_ptr);
    g((const unsigned long &)long_ptr);
    g((const signed long *)&long_ptr);
    g((const unsigned long *)&long_ptr);
    g((signed long *)long_ptr);
    g((unsigned long *)long_ptr);
    g((signed long *)&long_ptr_ptr);
    g((unsigned long *)&long_ptr_ptr);
    g((signed long &)long_ptr_ptr);
    g((unsigned long &)long_ptr_ptr);
    g((const signed long &)long_ptr_ptr);
    g((const unsigned long &)long_ptr_ptr);
    g((const signed long *)&long_ptr_ptr);
    g((const unsigned long *)&long_ptr_ptr);
    g((signed long *)long_ptr_ptr);
    g((unsigned long *)long_ptr_ptr);
}
