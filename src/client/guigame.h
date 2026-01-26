#ifndef GUI_GAME_H
#define GUI_GAME_H

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <thread>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

#include "../common/gra.h"
#include "../common/plansza.h"

enum GameState {
    MENU,
    INPUT_IP,
    INPUT_PORT,
    PLAY_LOCAL,
    PLAY_ONLINE,
    GAME_OVER
};

class GuiGame {
private:
    sf::RenderWindow window;
    sf::Font font;
    
    Plansza planszaG1; 
    Plansza planszaG2; 
    Gra gra;
    GameState state;

    int cellSize = 35;
    int gridOffsetX = 50;
    int gridOffsetY = 180; 
    
    bool isPlayer1Turn;
    bool waitingForSwitch;

    int sock;
    bool connected;
    std::thread networkThread;
    std::mutex queueMutex;
    
    std::string networkBuffer; 
    std::queue<std::string> msgQueue;
    
    int lastShotX;
    int lastShotY;

    std::vector<std::string> chatLog;
    std::string currentChatInput;
    bool myTurnOnline;
    
    std::string topStatusMsg;
    sf::Clock statusClock;

    std::string gameOverMsg;

    std::string inputIP;
    std::string inputPort;

    void drawMenu();
    void drawConnectMenu();
    void drawGame();
    void drawGrid(int startX, int startY, int T[12][12], int P[12][12], bool isRadar);
    void drawChat();
    
    void handleInput(const sf::Event& event);
    
    void handleLocalClick(int r, int c);
    void handleOnlineClick(int r, int c);
    
    bool connectToServer(std::string ip, int port, std::string name);
    void networkLoop();
    void processNetworkMessages();
    void sendMsg(std::string msg);
    void setupShipsRandomly(Plansza& p);
    void setStatus(std::string msg);

public:
    GuiGame();
    ~GuiGame();
    void run();
};

#endif