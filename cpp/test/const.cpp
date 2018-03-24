int main(int argc, char** argv)
{
    int x = 10;

    const int* p = &x;

    p = nullptr;

    return 0;
}