#include "guigame.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <SFML/Config.hpp>

using namespace std;

#define SFML_3_0 (SFML_VERSION_MAJOR >= 3)

static string boardToString(int T[12][12]) {
    string s = "";
    for (int i = 1; i <= 10; i++) {
        for (int j = 1; j <= 10; j++) {
            s += to_string(T[i][j]);
        }
    }
    return s;
}

GuiGame::GuiGame() : 
    state(MENU), 
    cellSize(35), 
    gridOffsetX(50), 
    gridOffsetY(180),
    isPlayer1Turn(true), 
    waitingForSwitch(false), 
    sock(-1), 
    connected(false), 
    lastShotX(0), 
    lastShotY(0),
    myTurnOnline(false) 
{
    unsigned int style = sf::Style::Titlebar | sf::Style::Close;
#if SFML_3_0
    window.create(sf::VideoMode({900, 800}), "BATTLESHIPS", style);
    if (!font.openFromFile("resources/fonts/font.ttf")) {}
#else
    window.create(sf::VideoMode(900, 800), "BATTLESHIPS", style);
    if (!font.loadFromFile("resources/fonts/font.ttf")) {}
#endif
    window.setFramerateLimit(60);
    
    inputIP = "127.0.0.1";
    inputPort = "8080";
    gameOverMsg = "";
}

GuiGame::~GuiGame() {
    connected = false;
    if (sock != -1) close(sock);
    if (networkThread.joinable()) networkThread.join();
}

void GuiGame::setStatus(string msg) {
    topStatusMsg = msg;
    statusClock.restart();
}

void GuiGame::setupShipsRandomly(Plansza& p) {
    p.ZERUJ();
    p.los4();
    for(int i=0; i<2; i++) p.los3();
    for(int i=0; i<3; i++) p.los2();
    for(int i=0; i<4; i++) p.los1();
}

void GuiGame::run() {
    while (window.isOpen()) {
#if SFML_3_0
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            else handleInput(*event);
        }
#else
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            else handleInput(event);
        }
#endif

        if (state == PLAY_ONLINE) processNetworkMessages();

        window.clear(sf::Color(30, 30, 40)); 
        
        if (state == MENU) drawMenu();
        else if (state == INPUT_IP || state == INPUT_PORT) drawConnectMenu();
        else if (state == PLAY_LOCAL) drawGame();
        else if (state == PLAY_ONLINE) {
            drawGame();
            drawChat();
        }
        else if (state == GAME_OVER) {
#if SFML_3_0
             sf::Text res(font, gameOverMsg, 60); res.setPosition({150.f, 300.f});
             sf::Text hint(font, "Wcisnij ESC aby wrocic do menu", 30); hint.setPosition({220.f, 400.f});
#else
             sf::Text res(gameOverMsg, font, 60); res.setPosition(150, 300);
             sf::Text hint("Wcisnij ESC aby wrocic do menu", font, 30); hint.setPosition(220, 400);
#endif
             if (gameOverMsg.find("WYGRA") != string::npos) res.setFillColor(sf::Color::Green);
             else if (gameOverMsg.find("PRZEGRA") != string::npos) res.setFillColor(sf::Color::Red);
             else res.setFillColor(sf::Color::Yellow);

             hint.setFillColor(sf::Color::White);
             window.draw(res);
             window.draw(hint);
        }
        window.display();
    }
}

void GuiGame::drawMenu() {
#if SFML_3_0
    sf::Text title(font, "BATTLESHIPS", 80); title.setPosition({200.f, 100.f});
    sf::Text opt1(font, "[1] Gra Lokalna (Hot-Seat)", 30); opt1.setPosition({250.f, 350.f});
    sf::Text opt2(font, "[2] Gra Online (Multiplayer)", 30); opt2.setPosition({250.f, 420.f});
#else
    sf::Text title("BATTLESHIPS", font, 80); title.setPosition(200, 100);
    sf::Text opt1("[1] Gra Lokalna (Hot-Seat)", font, 30); opt1.setPosition(250, 350);
    sf::Text opt2("[2] Gra Online (Multiplayer)", font, 30); opt2.setPosition(250, 420);
#endif
    title.setFillColor(sf::Color(100, 200, 255));
    opt1.setFillColor(sf::Color::White);
    opt2.setFillColor(sf::Color::White);
    window.draw(title); window.draw(opt1); window.draw(opt2);
}

void GuiGame::drawConnectMenu() {
#if SFML_3_0
    sf::Text title(font, "POLACZENIE", 50); title.setPosition({300.f, 100.f});
    sf::Text lblIP(font, "Adres IP:", 30); lblIP.setPosition({200.f, 300.f});
    sf::Text txtIP(font, inputIP + (state == INPUT_IP ? "_" : ""), 30); txtIP.setPosition({400.f, 300.f});
    sf::Text lblPort(font, "Port:", 30); lblPort.setPosition({200.f, 400.f});
    sf::Text txtPort(font, inputPort + (state == INPUT_PORT ? "_" : ""), 30); txtPort.setPosition({400.f, 400.f});
    sf::Text hint(font, "Wpisz dane i zatwierdz ENTER", 20); hint.setPosition({300.f, 600.f});
#else
    sf::Text title("POLACZENIE", font, 50); title.setPosition(300, 100);
    sf::Text lblIP("Adres IP:", font, 30); lblIP.setPosition(200, 300);
    sf::Text txtIP(inputIP + (state == INPUT_IP ? "_" : ""), font, 30); txtIP.setPosition(400, 300);
    sf::Text lblPort("Port:", font, 30); lblPort.setPosition(200, 400);
    sf::Text txtPort(inputPort + (state == INPUT_PORT ? "_" : ""), font, 30); txtPort.setPosition(400, 400);
    sf::Text hint("Wpisz dane i zatwierdz ENTER", font, 20); hint.setPosition(300, 600);
#endif
    
    if(state == INPUT_IP) txtIP.setFillColor(sf::Color::Yellow);
    if(state == INPUT_PORT) txtPort.setFillColor(sf::Color::Yellow);
    
    window.draw(title); window.draw(lblIP); window.draw(txtIP);
    window.draw(lblPort); window.draw(txtPort); window.draw(hint);
}

void GuiGame::drawGame() {
#if SFML_3_0
    sf::Text mainStatus(font, "", 30); mainStatus.setPosition({50.f, 20.f});
    sf::Text actionStatus(font, "", 40); actionStatus.setPosition({300.f, 70.f});
#else
    sf::Text mainStatus("", font, 30); mainStatus.setPosition(50, 20);
    sf::Text actionStatus("", font, 40); actionStatus.setPosition(300, 70);
#endif
    
    if (state == PLAY_LOCAL) {
        if (waitingForSwitch) {
            sf::RectangleShape cover(sf::Vector2f((float)window.getSize().x, (float)window.getSize().y));
            cover.setFillColor(sf::Color::Black);
            window.draw(cover);
            
            mainStatus.setString("ZMIANA GRACZA");
#if SFML_3_0
            mainStatus.setPosition({300.f, 300.f});
#else
            mainStatus.setPosition(300, 300);
#endif
            mainStatus.setFillColor(sf::Color::Red);
            window.draw(mainStatus);
            
            actionStatus.setString("Wcisnij SPACJE...");
#if SFML_3_0
            actionStatus.setPosition({280.f, 350.f});
#else
            actionStatus.setPosition(280, 350);
#endif
            window.draw(actionStatus);
            return;
        }
        string gracz = isPlayer1Turn ? gra.get_nazwag1() : gra.get_nazwag2();
        mainStatus.setString("Tura: " + gracz);
    } else {
        mainStatus.setString(myTurnOnline ? "TWOJA TURA" : "OCZEKIWANIE...");
        if(myTurnOnline) mainStatus.setFillColor(sf::Color::Green);
        else mainStatus.setFillColor(sf::Color(200, 200, 200));
    }
    window.draw(mainStatus);

    if (statusClock.getElapsedTime().asSeconds() < 2.0f) {
        actionStatus.setString(topStatusMsg);
        actionStatus.setFillColor(sf::Color::Yellow);
        window.draw(actionStatus);
    }

#if SFML_3_0
    sf::Text l1(font, "MOJA FLOTA", 20); l1.setPosition({(float)gridOffsetX, (float)gridOffsetY - 50}); 
    sf::Text l2(font, "RADAR", 20); l2.setPosition({(float)gridOffsetX + 400, (float)gridOffsetY - 50}); 
#else
    sf::Text l1("MOJA FLOTA", font, 20); l1.setPosition(gridOffsetX, gridOffsetY - 50); 
    sf::Text l2("RADAR", font, 20); l2.setPosition(gridOffsetX + 400, gridOffsetY - 50); 
#endif

    l1.setFillColor(sf::Color::Cyan); window.draw(l1);
    l2.setFillColor(sf::Color::Red); window.draw(l2);

    if (state == PLAY_LOCAL) {
        if (isPlayer1Turn) {
            drawGrid(gridOffsetX, gridOffsetY, planszaG1.T, planszaG1.P, false); 
            drawGrid(gridOffsetX + 400, gridOffsetY, planszaG2.T, planszaG1.P, true); 
        } else {
            drawGrid(gridOffsetX, gridOffsetY, planszaG2.T, planszaG2.P, false);
            drawGrid(gridOffsetX + 400, gridOffsetY, planszaG1.T, planszaG2.P, true);
        }
    } 
    else { 
        drawGrid(gridOffsetX, gridOffsetY, planszaG1.T, planszaG1.P, false);
        drawGrid(gridOffsetX + 400, gridOffsetY, planszaG2.T, planszaG1.P, true);
    }
}

void GuiGame::drawGrid(int startX, int startY, int T[12][12], int P[12][12], bool isRadar) {
    sf::RectangleShape bg(sf::Vector2f((float)(10*cellSize), (float)(10*cellSize)));
#if SFML_3_0
    bg.setPosition({(float)startX, (float)startY});
#else
    bg.setPosition(startX, startY);
#endif
    bg.setFillColor(sf::Color(0, 20, 50)); 
    bg.setOutlineThickness(2);
    bg.setOutlineColor(sf::Color::White);
    window.draw(bg);

    for (int i = 1; i <= 10; i++) {
        for (int j = 1; j <= 10; j++) {
            float px = startX + (j-1)*cellSize;
            float py = startY + (i-1)*cellSize;

            sf::RectangleShape cell(sf::Vector2f((float)cellSize, (float)cellSize));
#if SFML_3_0
            cell.setPosition({px, py});
#else
            cell.setPosition(px, py);
#endif
            cell.setFillColor(sf::Color::Transparent);
            cell.setOutlineThickness(1);
            cell.setOutlineColor(sf::Color(0, 60, 100)); 
            
            int val = (isRadar) ? P[i][j] : T[i][j];

            if (!isRadar && val >= 1 && val <= 4) {
                sf::RectangleShape ship(sf::Vector2f((float)cellSize - 4, (float)cellSize - 4));
#if SFML_3_0
                ship.setPosition({px + 2, py + 2});
#else
                ship.setPosition(px + 2, py + 2);
#endif
                ship.setFillColor(sf::Color(100, 100, 100));
                window.draw(ship);
            }
            window.draw(cell); 

            if (val == 8) { // TRAFIENIE
                sf::RectangleShape line1(sf::Vector2f((float)cellSize * 1.2f, 4.f));
                sf::RectangleShape line2(sf::Vector2f((float)cellSize * 1.2f, 4.f));
                line1.setFillColor(sf::Color::Red);
                line2.setFillColor(sf::Color::Red);
#if SFML_3_0
                line1.setOrigin(line1.getSize() / 2.f);
                line2.setOrigin(line2.getSize() / 2.f);
                line1.setPosition({px + cellSize/2, py + cellSize/2});
                line2.setPosition({px + cellSize/2, py + cellSize/2});
                line1.setRotation(sf::degrees(45));
                line2.setRotation(sf::degrees(-45));
#else
                line1.setOrigin(line1.getSize() / 2.f);
                line2.setOrigin(line2.getSize() / 2.f);
                line1.setPosition(px + cellSize/2, py + cellSize/2);
                line2.setPosition(px + cellSize/2, py + cellSize/2);
                line1.setRotation(45);
                line2.setRotation(-45);
#endif
                window.draw(line1);
                window.draw(line2);
            }
            else if (val == 7) { // PUDŁO
                float r = cellSize / 4.0f;
                sf::CircleShape dot(r);
#if SFML_3_0
                dot.setOrigin({r, r});
                dot.setPosition({px + cellSize/2, py + cellSize/2});
#else
                dot.setOrigin(r, r);
                dot.setPosition(px + cellSize/2, py + cellSize/2);
#endif
                dot.setFillColor(sf::Color(200, 200, 255));
                window.draw(dot);
            }
        }
    }
    
    for(int k=1; k<=10; k++) {
#if SFML_3_0
        sf::Text n(font, to_string(k), 15); n.setPosition({(float)startX + (k-1)*cellSize + 10, (float)startY - 20}); window.draw(n);
        sf::Text m(font, to_string(k), 15); m.setPosition({(float)startX - 20, (float)startY + (k-1)*cellSize + 8}); window.draw(m);
#else
        sf::Text n(to_string(k), font, 15); n.setPosition(startX + (k-1)*cellSize + 10, startY - 20); window.draw(n);
        sf::Text m(to_string(k), font, 15); m.setPosition(startX - 20, startY + (k-1)*cellSize + 8); window.draw(m);
#endif
    }
}

void GuiGame::drawChat() {
    float chatY = 550.f;
    float chatH = 200.f;
    sf::RectangleShape chatBg(sf::Vector2f(800.f, chatH));
#if SFML_3_0
    chatBg.setPosition({50.f, chatY});
#else
    chatBg.setPosition(50, chatY);
#endif
    chatBg.setFillColor(sf::Color(10, 10, 20, 200));
    chatBg.setOutlineColor(sf::Color::White);
    chatBg.setOutlineThickness(1);
    window.draw(chatBg);

    int maxLines = 7;
    int start = 0;
    if (chatLog.size() > static_cast<size_t>(maxLines)) {
        start = static_cast<int>(chatLog.size()) - maxLines;
    }

    float lineY = chatY + 10;
    for (size_t i = static_cast<size_t>(start); i < chatLog.size(); i++) {
#if SFML_3_0
        sf::Text line(font, chatLog[i], 16); line.setPosition({60.f, lineY});
#else
        sf::Text line(chatLog[i], font, 16); line.setPosition(60, lineY);
#endif
        if (chatLog[i].rfind("[TY]", 0) == 0) line.setFillColor(sf::Color::Green);
        else if (chatLog[i].rfind("[PRZECIWNIK]", 0) == 0) line.setFillColor(sf::Color(255, 100, 100)); 
        else line.setFillColor(sf::Color::White);

        window.draw(line);
        lineY += 22;
    }

#if SFML_3_0
    sf::Text inputTxt(font, "> " + currentChatInput + "_", 16);
    inputTxt.setPosition({60.f, chatY + chatH - 30});
#else
    sf::Text inputTxt("> " + currentChatInput + "_", font, 16);
    inputTxt.setPosition(60, chatY + chatH - 30);
#endif
    inputTxt.setFillColor(sf::Color::Yellow);
    window.draw(inputTxt);
}

void GuiGame::handleInput(const sf::Event& event) {
#if SFML_3_0
    if (state == MENU) {
        if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            if (keyEvent->code == sf::Keyboard::Key::Num1) { 
                state = PLAY_LOCAL;
                setupShipsRandomly(planszaG1); setupShipsRandomly(planszaG2);
                gra.set_nazwag1("Gracz 1"); gra.set_nazwag2("Gracz 2");
            }
            if (keyEvent->code == sf::Keyboard::Key::Num2) {
                state = INPUT_IP;
                while(const auto e = window.pollEvent()){}
            }
        }
    }
    else if (state == INPUT_IP || state == INPUT_PORT) {
        if (const auto* textEvent = event.getIf<sf::Event::TextEntered>()) {
            string* target = (state == INPUT_IP) ? &inputIP : &inputPort;
            
            if (textEvent->unicode == '\b') {
                if (!target->empty()) target->pop_back();
            }
            else if (textEvent->unicode == 13 || textEvent->unicode == 10) {
                if (state == INPUT_IP) state = INPUT_PORT;
                else {
                    int port = 8080;
                    try { port = stoi(inputPort); } catch(...) {}
                    if(connectToServer(inputIP, port, "GuiPlayer")) {
                        state = PLAY_ONLINE;
                        setupShipsRandomly(planszaG1); gra.set_nazwag1("JA");
                        while(const auto e = window.pollEvent()){}
                    } else {
                        state = MENU;
                        cout << "Blad polaczenia!\n";
                    }
                }
            }
            else if (textEvent->unicode < 128) {
                if (textEvent->unicode != 13 && textEvent->unicode != 10)
                    *target += static_cast<char>(textEvent->unicode);
            }
        }
    }
    else if (state == GAME_OVER) {
        if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            if (keyEvent->code == sf::Keyboard::Key::Escape) state = MENU;
        }
    }
    else if (state == PLAY_LOCAL) {
        if (waitingForSwitch) {
            if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
                if (keyEvent->code == sf::Keyboard::Key::Space) {
                    waitingForSwitch = false; isPlayer1Turn = !isPlayer1Turn;
                }
            }
            return;
        }
        if (const auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseEvent->button == sf::Mouse::Button::Left) {
                sf::Vector2f mouseWorld = window.mapPixelToCoords(mouseEvent->position);
                int mx = static_cast<int>(mouseWorld.x);
                int my = static_cast<int>(mouseWorld.y);
                int startX = gridOffsetX + 400; int startY = gridOffsetY;
                if (mx > startX && mx < startX + (10*cellSize) && my > startY && my < startY + (10*cellSize)) {
                    int col = (mx - startX) / cellSize + 1; int row = (my - startY) / cellSize + 1;
                    handleLocalClick(row, col);
                }
            }
        }
    }
    else if (state == PLAY_ONLINE) {
        if (const auto* textEvent = event.getIf<sf::Event::TextEntered>()) {
            if (textEvent->unicode == '\b') {
                if (!currentChatInput.empty()) currentChatInput.pop_back();
            }
            else if (textEvent->unicode == 13 || textEvent->unicode == 10) {
                if (!currentChatInput.empty()) {
                    sendMsg("CHAT " + currentChatInput);
                    chatLog.push_back("[TY]: " + currentChatInput);
                    currentChatInput = "";
                }
            }
            else if (textEvent->unicode < 128) {
                if (textEvent->unicode != 13 && textEvent->unicode != 10) {
                    currentChatInput += static_cast<char>(textEvent->unicode);
                }
            }
        }
        if (myTurnOnline) {
             if (const auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
                 if (mouseEvent->button == sf::Mouse::Button::Left) {
                     sf::Vector2f mouseWorld = window.mapPixelToCoords(mouseEvent->position);
                     int mx = static_cast<int>(mouseWorld.x);
                     int my = static_cast<int>(mouseWorld.y);
                     int startX = gridOffsetX + 400; int startY = gridOffsetY;
                     if (mx > startX && mx < startX + (10*cellSize) && my > startY && my < startY + (10*cellSize)) {
                        int col = (mx - startX) / cellSize + 1; int row = (my - startY) / cellSize + 1;
                        handleOnlineClick(row, col);
                     }
                 }
             }
        }
    }
#else
    // SFML 2.x
    if (state == MENU) {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Num1) { 
                state = PLAY_LOCAL;
                setupShipsRandomly(planszaG1); setupShipsRandomly(planszaG2);
                gra.set_nazwag1("Gracz 1"); gra.set_nazwag2("Gracz 2");
            }
            if (event.key.code == sf::Keyboard::Num2) {
                state = INPUT_IP;
                sf::Event e; while(window.pollEvent(e)){}
            }
        }
    }
    else if (state == INPUT_IP || state == INPUT_PORT) {
        if (event.type == sf::Event::TextEntered) {
            string* target = (state == INPUT_IP) ? &inputIP : &inputPort;
            
            if (event.text.unicode == '\b') {
                if (!target->empty()) target->pop_back();
            }
            else if (event.text.unicode == 13 || event.text.unicode == 10) {
                if (state == INPUT_IP) state = INPUT_PORT;
                else {
                    int port = 8080;
                    try { port = stoi(inputPort); } catch(...) {}
                    if(connectToServer(inputIP, port, "GuiPlayer")) {
                        state = PLAY_ONLINE;
                        setupShipsRandomly(planszaG1); gra.set_nazwag1("JA");
                        sf::Event e; while(window.pollEvent(e)){}
                    } else {
                        state = MENU;
                        cout << "Blad polaczenia!\n";
                    }
                }
            }
            else if (event.text.unicode < 128) {
                if (event.text.unicode != 13 && event.text.unicode != 10)
                    *target += static_cast<char>(event.text.unicode);
            }
        }
    }
    else if (state == GAME_OVER) {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) state = MENU;
        }
    }
    else if (state == PLAY_LOCAL) {
        if (waitingForSwitch) {
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
                waitingForSwitch = false; isPlayer1Turn = !isPlayer1Turn;
            }
            return;
        }
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2i pixelPos = { event.mouseButton.x, event.mouseButton.y };
            sf::Vector2f mouseWorld = window.mapPixelToCoords(pixelPos);
            int mx = static_cast<int>(mouseWorld.x);
            int my = static_cast<int>(mouseWorld.y);
            int startX = gridOffsetX + 400; int startY = gridOffsetY;
            if (mx > startX && mx < startX + (10*cellSize) && my > startY && my < startY + (10*cellSize)) {
                int col = (mx - startX) / cellSize + 1; int row = (my - startY) / cellSize + 1;
                handleLocalClick(row, col);
            }
        }
    }
    else if (state == PLAY_ONLINE) {
        if (event.type == sf::Event::TextEntered) {
            if (event.text.unicode == '\b') {
                if (!currentChatInput.empty()) currentChatInput.pop_back();
            }
            else if (event.text.unicode == 13 || event.text.unicode == 10) {
                if (!currentChatInput.empty()) {
                    sendMsg("CHAT " + currentChatInput);
                    chatLog.push_back("[TY]: " + currentChatInput);
                    currentChatInput = "";
                }
            }
            else if (event.text.unicode < 128) {
                if (event.text.unicode != 13 && event.text.unicode != 10) {
                    currentChatInput += static_cast<char>(event.text.unicode);
                }
            }
        }
        if (myTurnOnline) {
             if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                 sf::Vector2i pixelPos = { event.mouseButton.x, event.mouseButton.y };
                 sf::Vector2f mouseWorld = window.mapPixelToCoords(pixelPos);
                 int mx = static_cast<int>(mouseWorld.x);
                 int my = static_cast<int>(mouseWorld.y);
                 int startX = gridOffsetX + 400; int startY = gridOffsetY;
                 if (mx > startX && mx < startX + (10*cellSize) && my > startY && my < startY + (10*cellSize)) {
                    int col = (mx - startX) / cellSize + 1; int row = (my - startY) / cellSize + 1;
                    handleOnlineClick(row, col);
                 }
             }
        }
    }
#endif
}

void GuiGame::handleLocalClick(int r, int c) {
    if (r < 1 || r > 10 || c < 1 || c > 10) return;
    if (isPlayer1Turn) {
        if (planszaG1.P[r][c] != 0) return; 
        gra.Strzal(planszaG2.T, planszaG1.P, r, c);
        if (planszaG2.SprawdzT()) {
            state = GAME_OVER;
            gameOverMsg = "GRACZ 1 WYGRAL!";
        }
    } else {
        if (planszaG2.P[r][c] != 0) return;
        gra.Strzal(planszaG1.T, planszaG2.P, r, c);
        if (planszaG1.SprawdzT()) {
            state = GAME_OVER;
            gameOverMsg = "GRACZ 2 WYGRAL!";
        }
    }
    waitingForSwitch = true;
}

bool GuiGame::connectToServer(string ip, int port, string name) {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) return false;
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if(inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0) return false;
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) return false;
    connected = true;
    sendMsg(name);
    networkThread = thread(&GuiGame::networkLoop, this);
    return true;
}

void GuiGame::sendMsg(string msg) {
    msg += "\n";
    send(sock, msg.c_str(), msg.length(), 0);
}

void GuiGame::networkLoop() {
    char buffer[1024];
    while (connected) {
        int valread = read(sock, buffer, 1024);
        if (valread <= 0) { connected = false; break; }
        
        lock_guard<mutex> lock(queueMutex);
        networkBuffer.append(buffer, valread);
        
        size_t pos = 0;
        while ((pos = networkBuffer.find('\n')) != string::npos) {
            string msg = networkBuffer.substr(0, pos);
            msgQueue.push(msg);
            networkBuffer.erase(0, pos + 1);
        }
    }
}

void GuiGame::processNetworkMessages() {
    lock_guard<mutex> lock(queueMutex);
    while (!msgQueue.empty()) {
        string cmd = msgQueue.front();
        msgQueue.pop();
        while (!cmd.empty() && isspace(cmd.back())) cmd.pop_back();

        if (cmd == "SEND_BOARD") {
            string flatBoard = boardToString(planszaG1.T);
            sendMsg(flatBoard);
            setStatus("Wysyłanie planszy...");
        }
        else if (cmd.rfind("START", 0) == 0) {
            setStatus("Gra z: " + cmd.substr(6));
        }
        else if (cmd.rfind("CHAT", 0) == 0) {
            chatLog.push_back(cmd.substr(5));
        }
        else if (cmd == "TURN") {
            myTurnOnline = true;
            setStatus("TWOJA TURA");
        }
        else if (cmd == "WAIT") {
            myTurnOnline = false;
            setStatus("Ruch przeciwnika...");
        }
        else if (cmd.rfind("RESULT", 0) == 0) {
            char type[10];
            int x, y;
            sscanf(cmd.c_str(), "RESULT %s %d %d", type, &x, &y);
            
            string res(type);
            if (res == "HIT") {
                chatLog.push_back(">> Trafiles w (" + to_string(x) + "," + to_string(y) + ")");
                setStatus("TRAFIENIE!");
                planszaG1.P[x][y] = 8;
            } else {
                chatLog.push_back(">> Pudlo w (" + to_string(x) + "," + to_string(y) + ")");
                setStatus("PUDLO");
                planszaG1.P[x][y] = 7; 
            }
        }
        else if (cmd == "OPPONENT_LEFT") {
            state = GAME_OVER;
            gameOverMsg = "Przeciwnik uciekl Wygrales";
        }
        else if (cmd.rfind("OPPONENT_SHOT", 0) == 0) {
            int x, y;
            sscanf(cmd.c_str(), "OPPONENT_SHOT %d %d", &x, &y);
            if (planszaG1.T[x][y] >= 1 && planszaG1.T[x][y] <= 4) {
                planszaG1.T[x][y] = 8; 
                setStatus("ZOSTALES TRAFIONY!");
            }
            else if(planszaG1.T[x][y])
            {
                planszaG1.T[x][y] = 7;
                setStatus("Przeciwnik spudlowal.");
            }
        }
        else if (cmd == "WIN") { state = GAME_OVER; gameOverMsg = "WYGRALES!"; }
        else if (cmd == "LOSE") { state = GAME_OVER; gameOverMsg = "PRZEGRALES..."; }
    }
}

void GuiGame::handleOnlineClick(int r, int c) {
    if (planszaG1.P[r][c] != 0) return;
    lastShotX = r;
    lastShotY = c;
    
    sendMsg("SHOT " + to_string(r) + " " + to_string(c));
    planszaG1.P[r][c] = 7;
    myTurnOnline = false; 
    setStatus("Wysylanie strzalu...");
}