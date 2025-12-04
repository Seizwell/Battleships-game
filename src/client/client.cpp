#include <iostream>

#include "localgame.h"


int main(){
    cout << "=== STATKI ===" << endl;
    cout << "1. Gra lokalna (2 graczy na zmianę)" << endl;
    cout << "2. Gra przez sieć (multiplayer)" << endl;
    cout << "3. Wyjście" << endl;
    cout << "Wybierz opcję: ";
    
    char wybor = cin.get();
    
    switch(wybor) {
        case '1': {
            LocalGame gra;
            gra.uruchom();
            break;
        }
        case '2': {
            // TODO: Później
            // NetworkGame gra("localhost", 8080);
            // gra.uruchom();
            cout << "Wkrótce!" << endl;
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