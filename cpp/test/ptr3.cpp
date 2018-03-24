#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
    char* s = "AAA";

    cout << s << endl;

    s[0] = 'B';

    cout << s << endl;
    return 0;
}