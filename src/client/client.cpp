#include <iostream>
#include <ctime>  
#include <cstdlib>

#include "localgame.h"
#include "onlinegame.h"
#include "guigame.h"

using namespace std;

int main(){
    srand(time(NULL));

    cout << "=== STATKI ===" << endl;
    cout << "1. Gra lokalna (2 graczy na zmianę)" << endl;
    cout << "2. Gra przez sieć (multiplayer - konsola)" << endl;
    cout << "3. GUI Mode (Okienka - Local & Online)" << endl;
    cout << "4. Wyjście" << endl;
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
            cin >> nazwa;

            string ip;
            cout << "Podaj IP serwera (np. 127.0.0.1): ";
            cin >> ip;
            
            int port;
            cout << "Podaj Port (np. 8080): ";
            cin >> port;
            
            OnlineGame gra;
            gra.uruchom(ip, port, nazwa);
            break;
        }
        case '3': {
            GuiGame gui;
            gui.run();
            break;
        }
        case '4':
            cout << "Do zobaczenia!" << endl;
            break;
        default:
            cout << "Nieprawidłowa opcja!" << endl;
    }
    
    return 0;
}