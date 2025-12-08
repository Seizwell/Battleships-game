#ifndef GRA_H
#define GRA_H

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


class Gra
{
    private:
    string gracz1; //nazwa gracza 1
    string gracz2; //nazwa gracza 2

    public:
    
    void set_nazwag1(string t);
    void set_nazwag2(string t);

    string get_nazwag1() const;
    string get_nazwag2() const;


    void Strzal(int T[12][12], int P[12][12],int x,int y);//strzał ręczny

    void zapiszT(int T1[12][12], int T2[12][12],int P1[12][12], int P2[12][12]);//zapisuje grę do pliku

    void wczytajT(int T1[12][12], int T2[12][12],int P1[12][12], int P2[12][12]);//wczytuje grę z pliu
};
#endif