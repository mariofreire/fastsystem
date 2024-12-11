void delay(int x)
{
    int i = 10000;
    while (--i)
    {
        int j = 10 * x;
        while (--j);
    }
}

