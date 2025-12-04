#ifndef LOCAL_GAME_H
#define LOCAL_GAME_H


#include "../common/gra.h"
#include "../common/plansza.h"

class LocalGame {
private:
    int gracz1=1;
    int gracz2=2;
    Plansza planszaGracza1;
    Plansza planszaGracza2;
    Gra gra;
    
    void inicjalizujNowaGre();
    void wczytajGre();
    void zapiszGre();
    bool wykonajTureGracza(int numerGracza);
    bool sprawdzKoniec();
    
public:
    LocalGame();
    void uruchom();
};

#endif