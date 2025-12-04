#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cmath>
#include <string>
#include <ctime>
#include <fstream>
#include "gra.h"

using namespace std;

void Gra::set_g1(string t)
{
    gracz1=t;
}

void Gra::set_g2(string t)
{
    gracz2=t;
}

string Gra::Nazwa1()
{
    return gracz1;
}

string Gra::Nazwa2()
{
    return gracz2;
}

void Gra::Strzal(int T[12][12], int P[12][12], int x, int y)
{
    if(T[x][y]==0||T[x][y]==5||T[x][y]==7){
        T[x][y]=7;
        P[x][y]=7;
    }
    else if(T[x][y]==1||T[x][y]==2||T[x][y]==3||T[x][y]==4||T[x][y]==8){
        T[x][y]=8;
        P[x][y]=8;
    }
}

void Gra::zapiszT(int T1[12][12], int T2[12][12],int P1[12][12], int P2[12][12])
{
    ofstream plik;
    plik.open("../bin/statki.txt");
    plik<<gracz1<<endl;
    plik<<gracz2<<endl;
    for(int i=0; i<12; i++){
        for(int j=0; j<12; j++){
            plik<<T1[i][j]<<endl;
        }
    }
    for(int i=0; i<12; i++){
        for(int j=0; j<12; j++){
            plik<<T2[i][j]<<endl;
        }
    }
    for(int i=0; i<12; i++){
        for(int j=0; j<12; j++){
            plik<<P1[i][j]<<endl;
        }
    }
    for(int i=0; i<12; i++){
        for(int j=0; j<12; j++){
            plik<<P2[i][j]<<endl;
        }
    }
    plik.close();
}


void Gra::wczytajT(int T1[12][12], int T2[12][12],int P1[12][12], int P2[12][12])
{
    ifstream plik;
    plik.open("../bin/statki.txt");
    if(plik.is_open()){
        plik>>gracz1;
        plik>>gracz2;
        for(int i=0; i<12; i++){
            for(int j=0; j<12; j++){
                plik>>T1[i][j];
            }
        }
        for(int i=0; i<12; i++){
            for(int j=0; j<12; j++){
                plik>>T2[i][j];
            }
        }
        for(int i=0; i<12; i++){
            for(int j=0; j<12; j++){
                plik>>P1[i][j];
            }
        }
        for(int i=0; i<12; i++){
            for(int j=0; j<12; j++){
                plik>>P2[i][j];
            }
        }

        plik.close();
    }
    else{
        exit(0);
    }
}
