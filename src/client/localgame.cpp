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
//#include <term.h>
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
    cout << "Chcesz kontynuowac poprzednia gre czy zaczac nowa? (P - poprzednia, N - nowa): ";
    char wybor;
    cin >> wybor;
    cin.ignore(1000, '\n');

    if(toupper(wybor) == 'P') {
        gra.wczytajT(planszaGracza1.T,planszaGracza2.T,planszaGracza1.P,planszaGracza2.P);
    } else {
        inicjalizujNowaGre();
    }

    char c = 'T'; // Inicjalizacja, żeby wejść do pętli
    do
    {
        // === TURA GRACZA 1 ===
        system("clear"); 
        cout << "Tura Gracza: " << gra.get_nazwag1() << "\n";
        cout << "Przekaż komputer Graczowi " << gra.get_nazwag1() << " i wciśnij ENTER...";
        cin.get(); // Czeka na enter

        system("clear");
        cout << "--- PLANSZA STRZALOW GRACZA " << gra.get_nazwag1() << " ---\n";
        planszaGracza1.piszP();
        cout << "--- TWOJE STATKI ---\n";
        planszaGracza1.piszT();
        
        int x1, y1;
        do {
            cout << "Podaj Wiersz strzalu (1-10): ";
            cin >> x1;
            cout << "Podaj Kolumne strzalu (1-10): ";
            cin >> y1;
            
            if(cin.fail() || x1 < 1 || x1 > 10 || y1 < 1 || y1 > 10) {
                cout << "Błąd! Współrzędne muszą być liczbami od 1 do 10.\n";
                cin.clear(); // Czyści flagę błędu
                cin.ignore(1000, '\n'); // Usuwa błędne dane z bufora
            } else {
                break; // Dane poprawne
            }
        } while(true);

        gra.Strzal(planszaGracza2.T, planszaGracza1.P, x1, y1);
        cin.ignore(1000, '\n'); // Czyścimy bufor po cin >> int

        if(sprawdzKoniec()) break;

        // === TURA GRACZA 2 ===
        system("clear");
        cout << "Tura Gracza: " << gra.get_nazwag2() << "\n";
        cout << "Przekaż komputer Graczowi " << gra.get_nazwag2() << " i wciśnij ENTER...";
        cin.get();

        system("clear");
        cout << "--- PLANSZA STRZALOW GRACZA " << gra.get_nazwag2() << " ---\n";
        planszaGracza2.piszP();
        cout << "--- TWOJE STATKI ---\n";
        planszaGracza2.piszT();
        
        int x2, y2;
        do {
            cout << "Podaj Wiersz strzalu (1-10): ";
            cin >> x2;
            cout << "Podaj Kolumne strzalu (1-10): ";
            cin >> y2;
             if(cin.fail() || x2 < 1 || x2 > 10 || y2 < 1 || y2 > 10) {
                cout << "Błąd! Współrzędne muszą być liczbami od 1 do 10.\n";
                cin.clear();
                cin.ignore(1000, '\n');
            } else {
                break;
            }
        } while(true);

        gra.Strzal(planszaGracza1.T, planszaGracza2.P, x2, y2);
        cin.ignore(1000, '\n');

        if(sprawdzKoniec()) break;

        // === AUTOSAVE I PYTANIE ===
        gra.zapiszT(planszaGracza1.T, planszaGracza2.T, planszaGracza1.P, planszaGracza2.P);
        
        cout << "\nGra zapisana.\nCzy chcesz grać dalej? (T/N): ";
        cin >> c;
        cin.ignore(1000, '\n'); // Znowu czyszczenie po wczytaniu chara

    } while (toupper(c) == 'T');
}
