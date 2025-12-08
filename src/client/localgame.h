#ifndef LOCAL_GAME_H
#define LOCAL_GAME_H


#include "../common/gra.h"
#include "../common/plansza.h"

class LocalGame {
private:
    Plansza planszaGracza1;
    Plansza planszaGracza2;
    Gra gra;
    
    void inicjalizujNowaGre();
    bool sprawdzKoniec();
    
public:
    void uruchom();
};

#endif