#ifndef ONLINE_GAME_H
#define ONLINE_GAME_H

#include <string>
#include <netinet/in.h>
#include <thread>          
#include <mutex>             
#include <queue>              
#include <condition_variable> 
#include "../common/gra.h"
#include "../common/plansza.h"

class OnlineGame {
private:
    Plansza mojaPlansza;
    Gra gra;
    
    int sock; // Deskryptor gniazda
    struct sockaddr_in server_addr;
    bool connected;
    
    std::thread networkThread;       // Wątek odbierający pakiety
    std::queue<std::string> gameMsgQueue; // Kolejka tylko na komendy gry (TURN, SHOT)
    std::mutex queueMutex;           // Ochrona kolejki
    std::condition_variable queueCV; // Powiadamianie, że coś przyszło
    bool running;                    // Flaga do zatrzymania wątku

    // Metody pomocnicze
    void inicjalizujStatki();
    bool polacz(std::string ip, int port);
    void wyslijWiadomosc(std::string msg);
    std::string pobierzKomendeGry();
    
    void watekNasluchiwania(); 
    
    void obslugaTury();

public:
    OnlineGame();
    ~OnlineGame(); // Destruktor do zamknięcia socketu
    void uruchom(std::string ip, int port, std::string nazwaGracza);
};

#endif