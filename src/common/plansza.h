#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cmath>
#include <string>
#include <ctime>
#include <fstream>

using namespace std;

int const maks = 12;

class Plansza
{
    public:


    int T[maks][maks];

    int P[maks][maks];

    void ZERUJ();//zerujue plansze

    void piszT();//wypisuje plansze

    void los4();

    void los3();

    void los2();

    void los1();

    int SprawdzT();

};
