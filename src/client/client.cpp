#include <iostream>
#include <ctime>  
#include <cstdlib>

#include "localgame.h"

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