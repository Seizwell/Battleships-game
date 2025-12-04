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

    void piszT(int T[maks][maks]);//wypisuje plansze

    void los4(int T[maks][maks]);

    void los3(int T[maks][maks]);

    void los2(int T[maks][maks]);

    void los1(int T[maks][maks]);

    int SprawdzT(int T[maks][maks]);

};
