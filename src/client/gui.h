#ifndef GUI_H
#define GUI_H

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <thread>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

#include "../common/gra.h"
#include "../common/plansza.h"

// Stany gry w GUI
enum GameState {
    MENU,
    SETUP_LOCAL,
    SETUP_ONLINE,
    PLAY_LOCAL,
    PLAY_ONLINE,
    GAME_OVER
};

class GuiGame {
private:
    sf::RenderWindow window;
    sf::Font font;
    
    // Logika gry
    Plansza planszaG1; // Dla Local: P1, Dla Online: Ja
    Plansza planszaG2; // Dla Local: P2, Dla Online: Bufor (niewidoczna)
    Gra gra;
    GameState state;

    // GUI Elementy
    int cellSize = 30;
    int gridOffsetX = 50;
    int gridOffsetY = 100;
    
    // Zmienne dla Local Game
    bool isPlayer1Turn;
    bool waitingForSwitch; // Ekran "Przekaż komputer..."

    // Zmienne dla Online Game
    int sock;
    bool connected;
    std::thread networkThread;
    std::mutex queueMutex;
    std::queue<std::string> msgQueue;
    std::vector<std::string> chatLog;
    std::string currentChatInput;
    bool myTurnOnline;
    
    // Metody Rysowania
    void drawMenu();
    void drawGame(bool showEnemyShips); // showEnemyShips=false (norma), true (debug/koniec)
    void drawGrid(int startX, int startY, int T[12][12], int P[12][12], bool isEnemyBoard);
    void drawChat();
    
    // Metody Logiki
    void handleInput(sf::Event& event);
    void handleLocalClick(int x, int y, bool leftSide);
    void handleOnlineClick(int x, int y);
    
    // Metody Sieciowe (zapożyczone z OnlineGame ale dostosowane do GUI)
    bool connectToServer(std::string ip, int port, std::string name);
    void networkLoop();
    void processNetworkMessages();
    void sendMsg(std::string msg);
    void setupShipsRandomly(Plansza& p);

public:
    GuiGame();
    ~GuiGame();
    void run();
};

#endif