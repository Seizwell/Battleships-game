#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cmath>
#include <string>
#include <ctime>
#include <fstream>
#include <unistd.h>
#include <term.h>
#include <time.h>

#include "../common/gra.h"
#include "../common/plansza.h"
#include "localgame.h"

void LocalGame::inicjalizujNowaGre()
{
    planszaGracza1.ZERUJ();
    planszaGracza1.los4();
    for(int i = 0; i < 2; i++) planszaGracza1.los3();
    for(int i = 0; i < 3; i++) planszaGracza1.los2();
    for(int i = 0; i < 4; i++) planszaGracza1.los1();

    planszaGracza2.ZERUJ();
    planszaGracza2.los4();
    for(int i = 0; i < 2; i++) planszaGracza2.los3();
    for(int i = 0; i < 3; i++) planszaGracza2.los2();
    for(int i = 0; i < 4; i++) planszaGracza2.los1();
}

void LocalGame::wczytajGre()
{
    gra.wczytajT(planszaGracza1.T,planszaGracza2.T,planszaGracza1.P,planszaGracza2.P);
}

void LocalGame::zapiszGre()
{
    gra.zapiszT(planszaGracza1.T,planszaGracza2.T,planszaGracza1.P,planszaGracza2.P);
}

bool LocalGame::wykonajTureGracza(int numerGracza)
{
    return false;
}

bool LocalGame::sprawdzKoniec()
{
    return false;
}

void LocalGame::uruchom()
{
    cout << "Chcesz kontynuowac poprzednia gre czy zaczac nowa? Wpisz P dla poprzedniej lub N dla nowej: ";
    char wybor = toupper(cin.get());
    cout << "\n";
    
    if(wybor == 'P') {
        wczytajGre();
    } else {
        inicjalizujNowaGre();
    }
    cout << "Podaj nazwę Gracza " << gracz1 << ": ";
    cin.get();

    cout << "Podaj nazwę Gracza " << gracz2 << ": ";
    cin.get();
}
