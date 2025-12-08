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

    string nazwa1, nazwa2;
    cout << "Podaj nazwę Gracza 1: ";
    cin >> nazwa1;
    gra.set_nazwag1(nazwa1);

    cout << "Podaj nazwę Gracza 2: ";
    cin >> nazwa2;
    gra.set_nazwag2(nazwa2);
    
}

bool LocalGame::sprawdzKoniec()
{
    if(planszaGracza1.SprawdzT())
    {
        cout<<"Gracz "<< gra.get_nazwag1()<<" nie ma statkow"<<"\n";
        cout<<"Gracz "<< gra.get_nazwag2()<<" WYGRAL"<<"\n";
        return true;
    }
    if(planszaGracza2.SprawdzT())
    {
        cout<<"Gracz "<< gra.get_nazwag1()<<" nie ma statkow"<<"\n";
        cout<<"Gracz "<< gra.get_nazwag2()<<" WYGRAL"<<"\n";
        return true;
    }
    return false;
}

void LocalGame::uruchom()
{
    cout << "Chcesz kontynuowac poprzednia gre czy zaczac nowa? Wpisz P dla poprzedniej lub N dla nowej: "<<"\n";
    char wybor;
    cin >> wybor;

    if(toupper(wybor) == 'P') {
        gra.wczytajT(planszaGracza1.T,planszaGracza2.T,planszaGracza1.P,planszaGracza2.P);

    } else {
        inicjalizujNowaGre();
    }
    system("clear");
    char c;
    do
    {
        int x1,y1,x2,y2;
        cout<<"Tura Gracza "<<gra.get_nazwag1()<<"Wciśnij przycisk gdy będziesz gotowy"<<"\n";
        cin.get();
        planszaGracza1.piszP();
        planszaGracza1.piszT();
        cout<<"Podaj X strzalu: "<<"\n";
        cin >> x1;
        cout<<"Podaj Y strzalu: "<<"\n";
        cin >> y1;
        gra.Strzal(planszaGracza2.T,planszaGracza1.P,x1,y1);
        system("clear");
        if(sprawdzKoniec())
        {
            break;
        }
        cout<<"Tura Gracza "<<gra.get_nazwag2()<<"Wciśnij przycisk gdy będziesz gotowy"<<"\n";
        cin.get();
        planszaGracza2.piszP();
        planszaGracza2.piszT();
        cout<<"Podaj X strzalu: "<<"\n";
        cin >> x2;
        cout<<"Podaj Y strzalu: "<<"\n";
        cin >> y2;
        gra.Strzal(planszaGracza1.T,planszaGracza2.P,x2,y2);
        system("clear");
        if(sprawdzKoniec())
        {
            break;
        }
        gra.zapiszT(planszaGracza1.T,planszaGracza2.T,planszaGracza1.P,planszaGracza2.P);
        cout<<"--- AUTOSAVE ---"<<"\n";
        cout<<"Czy chcesz kontynuować rozgrywkę T/N"<<"\n";
        cin >> c;
    } while (toupper(c) =='T');
    
 
}
