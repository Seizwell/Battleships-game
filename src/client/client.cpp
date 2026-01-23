#include <iostream>
#include <ctime>  
#include <cstdlib>

#include "localgame.h"
#include "onlinegame.h"

using namespace std;

int main(){
    srand(time(NULL));

    cout << "=== STATKI ===" << endl;
    cout << "1. Gra lokalna (2 graczy na zmianę)" << endl;
    cout << "2. Gra przez sieć (multiplayer)" << endl;
    cout << "3. Wyjście" << endl;
    cout << "Wybierz opcję: ";
    
    char wybor = cin.get();
    if(wybor != '\n') cin.ignore(1000, '\n');
    
    switch(wybor) {
        case '1': {
            LocalGame gra;
            gra.uruchom();
            break;
        }
        case '2': {
            string nazwa;
            cout << "Podaj swoją nazwę gracza: ";
            // Używamy getline, żeby nazwa mogła mieć spacje (opcjonalnie)
            // Ale cin >> nazwa jest bezpieczniejsze na start, zostańmy przy tym:
            cin >> nazwa;

            string ip;
            cout << "Podaj IP serwera (np. 127.0.0.1): ";
            cin >> ip;
            
            OnlineGame gra;
            // Przekazujemy nazwę do funkcji
            gra.uruchom(ip, 8080, nazwa);
            break;
        }
        case '3':
            cout << "Do zobaczenia!" << endl;
            break;
        default:
            cout << "Nieprawidłowa opcja!" << endl;
    }
    
    return 0;
}