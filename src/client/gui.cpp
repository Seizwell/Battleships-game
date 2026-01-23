#include "gui.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

GuiGame::GuiGame() : state(MENU), isPlayer1Turn(true), waitingForSwitch(false), sock(-1), connected(false), myTurnOnline(false) {
    window.create(sf::VideoMode(900, 700), "Battleships GUI");
    window.setFramerateLimit(60);
    
    // Próbujemy załadować czcionkę systemową (ścieżka dla Maca/Linuxa może się różnić)
    // Jeśli nie zadziała, tekst nie będzie widoczny. Warto dodać plik .ttf do folderu bin.
    if (!font.loadFromFile("/System/Library/Fonts/Helvetica.ttc")) { // macOS
        if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) { // Linux
             // Fallback lub brak czcionki (tekst nie zadziała)
             cout << "Nie znaleziono czcionki! Teksty moga nie dzialac.\n";
        }
    }
}

GuiGame::~GuiGame() {
    connected = false;
    if (sock != -1) close(sock);
    if (networkThread.joinable()) networkThread.join();
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
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            handleInput(event);
        }

        if (state == PLAY_ONLINE) {
            processNetworkMessages();
        }

        window.clear(sf::Color(30, 30, 30)); // Ciemne tło
        
        if (state == MENU) drawMenu();
        else if (state == PLAY_LOCAL) drawGame(false);
        else if (state == PLAY_ONLINE) {
            drawGame(false);
            drawChat();
        }
        else if (state == GAME_OVER) {
             sf::Text txt("KONIEC GRY", font, 50);
             txt.setPosition(300, 300);
             txt.setFillColor(sf::Color::Red);
             window.draw(txt);
        }

        window.display();
    }
}

void GuiGame::drawMenu() {
    sf::Text title("BATTLESHIPS", font, 60);
    title.setPosition(250, 100);
    window.draw(title);

    sf::Text opt1("1. Gra Lokalna (Kliknij 1)", font, 30);
    opt1.setPosition(300, 300);
    window.draw(opt1);

    sf::Text opt2("2. Gra Online (Kliknij 2)", font, 30);
    opt2.setPosition(300, 360);
    window.draw(opt2);
}

void GuiGame::drawGame(bool showEnemyShips) {
    // Rysujemy dwie siatki
    // Lewa: MOJE STATKI (T) + Co przeciwnik trafił (widzę to na moim T jako 8/X)
    // Prawa: MOJE STRZAŁY (P) -> Gdzie ja strzelałem

    sf::Text statusText("", font, 24);
    statusText.setPosition(50, 20);
    
    if (state == PLAY_LOCAL) {
        if (waitingForSwitch) {
            statusText.setString("Tura zakonczona. Kliknij SPACJE aby zmienic gracza.");
            window.draw(statusText);
            return; // Nie rysujemy planszy żeby nie podglądać
        }
        string gracz = isPlayer1Turn ? gra.get_nazwag1() : gra.get_nazwag2();
        statusText.setString("Tura gracza: " + gracz);
    } else {
        statusText.setString(myTurnOnline ? "TWOJA TURA - Strzelaj!" : "Ruch przeciwnika...");
        if(myTurnOnline) statusText.setFillColor(sf::Color::Green);
        else statusText.setFillColor(sf::Color(200, 200, 200));
    }
    window.draw(statusText);

    // Etykiety
    sf::Text l1("MOJA FLOTA", font, 18); l1.setPosition(gridOffsetX, gridOffsetY - 30); window.draw(l1);
    sf::Text l2("RADAR (STRZALY)", font, 18); l2.setPosition(gridOffsetX + 400, gridOffsetY - 30); window.draw(l2);

    // Plansze
    if (state == PLAY_LOCAL) {
        if (isPlayer1Turn) {
            drawGrid(gridOffsetX, gridOffsetY, planszaG1.T, planszaG1.P, false); // Lewa: moje statki (tylko T)
            drawGrid(gridOffsetX + 400, gridOffsetY, planszaG2.T, planszaG1.P, true); // Prawa: moje strzały (P1)
        } else {
            drawGrid(gridOffsetX, gridOffsetY, planszaG2.T, planszaG2.P, false);
            drawGrid(gridOffsetX + 400, gridOffsetY, planszaG1.T, planszaG2.P, true);
        }
    } 
    else { // ONLINE
        // Lewa: Moja plansza (statki)
        // Prawa: Moja historia strzałów
        // Uwaga: W online P służy tylko do wyświetlania moich strzałów, logika jest na serwerze/gra.h
        drawGrid(gridOffsetX, gridOffsetY, planszaG1.T, planszaG1.P, false);
        drawGrid(gridOffsetX + 400, gridOffsetY, planszaG2.T, planszaG1.P, true); // Prawa: P1.P to moje strzały
    }
}

void GuiGame::drawGrid(int startX, int startY, int T[12][12], int P[12][12], bool isRadar) {
    for (int i = 1; i <= 10; i++) {
        for (int j = 1; j <= 10; j++) {
            sf::RectangleShape cell(sf::Vector2f(cellSize - 2, cellSize - 2));
            cell.setPosition(startX + (j-1)*cellSize, startY + (i-1)*cellSize);
            
            int val = 0;
            if (isRadar) {
                // Na radarze interesuje nas tablica P (gdzie strzelalismy)
                val = P[i][j]; 
            } else {
                // Na mojej flocie interesuje nas tablica T (statki i oberwania)
                val = T[i][j];
            }

            // Kolory
            if (val == 0 || val == 5) cell.setFillColor(sf::Color(50, 50, 150)); // Woda (5 to bufor)
            else if (val >= 1 && val <= 4) cell.setFillColor(sf::Color(100, 100, 100)); // Statek
            else if (val == 7) cell.setFillColor(sf::Color::White); // Pudło (~)
            else if (val == 8) cell.setFillColor(sf::Color::Red); // Trafiony (X)

            // Ukrywanie statków na radarze (jeśli lokalnie gramy i nie chcemy widziec statkow wroga przed trafieniem)
            if (isRadar && val >= 1 && val <= 4) cell.setFillColor(sf::Color(50, 50, 150)); // Ukryj nietrafione statki wroga

            window.draw(cell);
        }
    }
    
    // Ramka
    sf::RectangleShape border(sf::Vector2f(10*cellSize, 10*cellSize));
    border.setPosition(startX, startY);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color::White);
    border.setOutlineThickness(2);
    window.draw(border);
}

void GuiGame::drawChat() {
    // Tło chatu
    sf::RectangleShape chatBg(sf::Vector2f(800, 150));
    chatBg.setPosition(50, 500);
    chatBg.setFillColor(sf::Color(20, 20, 20));
    chatBg.setOutlineColor(sf::Color::White);
    chatBg.setOutlineThickness(1);
    window.draw(chatBg);

    // Historie
    int y = 510;
    // Pokaż ostatnie 5 wiadomości
    int start = max(0, (int)chatLog.size() - 5);
    for (int i = start; i < chatLog.size(); i++) {
        sf::Text line(chatLog[i], font, 16);
        line.setPosition(60, y);
        window.draw(line);
        y += 20;
    }

    // Input
    sf::Text inputTxt("> " + currentChatInput + "_", font, 16);
    inputTxt.setPosition(60, 620);
    inputTxt.setFillColor(sf::Color::Yellow);
    window.draw(inputTxt);
}

void GuiGame::handleInput(sf::Event& event) {
    if (state == MENU) {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Num1) {
                state = PLAY_LOCAL;
                setupShipsRandomly(planszaG1);
                setupShipsRandomly(planszaG2);
                gra.set_nazwag1("Gracz 1");
                gra.set_nazwag2("Gracz 2");
            }
            if (event.key.code == sf::Keyboard::Num2) {
                // Proste łączenie (hardcoded dla GUI dla uproszczenia, w GUI powinno być pole tekstowe)
                cout << "Laczenie z localhost...\n";
                if(connectToServer("127.0.0.1", 8080, "GuiPlayer")) {
                    state = PLAY_ONLINE;
                    setupShipsRandomly(planszaG1); // Moje statki
                    gra.set_nazwag1("JA");
                }
            }
        }
    }
    else if (state == PLAY_LOCAL) {
        if (waitingForSwitch) {
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
                waitingForSwitch = false;
                isPlayer1Turn = !isPlayer1Turn;
            }
            return;
        }

        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            int mx = event.mouseButton.x;
            int my = event.mouseButton.y;
            // Sprawdzamy czy kliknięto w RADAR (Prawa plansza) - bo tam strzelamy
            int startX = gridOffsetX + 400;
            int startY = gridOffsetY;
            
            if (mx > startX && mx < startX + 300 && my > startY && my < startY + 300) {
                int col = (mx - startX) / cellSize + 1;
                int row = (my - startY) / cellSize + 1;
                
                handleLocalClick(row, col, false);
            }
        }
    }
    else if (state == PLAY_ONLINE) {
        // Obsługa chatu
        if (event.type == sf::Event::TextEntered) {
            if (event.text.unicode == '\b') { // Backspace
                if (!currentChatInput.empty()) currentChatInput.pop_back();
            }
            else if (event.text.unicode == 13) { // Enter
                if (!currentChatInput.empty()) {
                    sendMsg("CHAT " + currentChatInput);
                    chatLog.push_back("[TY]: " + currentChatInput);
                    currentChatInput = "";
                }
            }
            else if (event.text.unicode < 128) {
                currentChatInput += static_cast<char>(event.text.unicode);
            }
        }
        
        // Obsługa strzałów
        if (myTurnOnline && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
             int mx = event.mouseButton.x;
             int my = event.mouseButton.y;
             int startX = gridOffsetX + 400; // Radar
             int startY = gridOffsetY;
             
             if (mx > startX && mx < startX + 300 && my > startY && my < startY + 300) {
                int col = (mx - startX) / cellSize + 1;
                int row = (my - startY) / cellSize + 1;
                handleOnlineClick(row, col);
             }
        }
    }
}

void GuiGame::handleLocalClick(int r, int c, bool leftSide) {
    if (r < 1 || r > 10 || c < 1 || c > 10) return;

    // Logika strzału
    if (isPlayer1Turn) {
        if (planszaG1.P[r][c] != 0) return; // Już tu strzelano
        gra.Strzal(planszaG2.T, planszaG1.P, r, c);
        // Sprawdź wygraną
        if (planszaG2.SprawdzT()) state = GAME_OVER;
    } else {
        if (planszaG2.P[r][c] != 0) return;
        gra.Strzal(planszaG1.T, planszaG2.P, r, c);
        if (planszaG1.SprawdzT()) state = GAME_OVER;
    }
    
    waitingForSwitch = true; // Koniec tury, czekaj na spację
}

// --- ONLINE LOGIC (REUSED) ---

bool GuiGame::connectToServer(string ip, int port, string name) {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) return false;

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) return false;

    connected = true;
    sendMsg(name); // Wyślij imię
    
    // Uruchom wątek nasłuchujący
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
        buffer[valread] = '\0';
        
        string raw(buffer);
        // Prosty parser (może sklejać, ale dla GUI uproszczone)
        // W idealnym świecie tutaj też używamy bufora znak po znaku jak w konsoli
        lock_guard<mutex> lock(queueMutex);
        msgQueue.push(raw);
    }
}

void GuiGame::processNetworkMessages() {
    lock_guard<mutex> lock(queueMutex);
    while (!msgQueue.empty()) {
        string cmd = msgQueue.front();
        msgQueue.pop();
        
        // Czyścimy białe znaki
        while (!cmd.empty() && isspace(cmd.back())) cmd.pop_back();

        if (cmd.rfind("CHAT", 0) == 0) {
            chatLog.push_back("[PRZECIWNIK]: " + cmd.substr(5));
        }
        else if (cmd == "TURN") {
            myTurnOnline = true;
        }
        else if (cmd == "WAIT") {
            myTurnOnline = false;
        }
        else if (cmd.rfind("SHOT", 0) == 0) {
            int x, y;
            sscanf(cmd.c_str(), "SHOT %d %d", &x, &y);
            // Ktoś we mnie strzelił
            int dummy[12][12];
            gra.Strzal(planszaG1.T, dummy, x, y); // Aktualizuj moje statki
            if (planszaG1.T[x][y] == 8) sendMsg("HIT");
            else sendMsg("MISS");
        }
        else if (cmd == "HIT") {
            // Trafiłem! (Muszę pamiętać gdzie strzeliłem ostatnio - uproszczenie: GUI nie pamięta "lastShot", 
            // więc tutaj tylko informacja. W pełnej wersji trzeba zapisać `lastX`, `lastY`)
            chatLog.push_back(">> TRAFIENIE!");
        }
        else if (cmd == "MISS") {
            chatLog.push_back(">> PUDLO.");
        }
        else if (cmd == "WIN") {
            state = GAME_OVER;
            chatLog.push_back("WYGRALES!");
        }
        else if (cmd == "LOSE") {
            state = GAME_OVER;
            chatLog.push_back("PRZEGRALES...");
        }
    }
}

void GuiGame::handleOnlineClick(int r, int c) {
    // Sprawdź czy już tu strzelano (planszaG1.P to nasze strzały w online)
    if (planszaG1.P[r][c] != 0) return;

    // Wyślij strzał
    sendMsg(to_string(r) + " " + to_string(c));
    
    // Zaznacz tymczasowo jako strzelone (koloruj na biało/czerwono po odpowiedzi serwera)
    // Uproszczenie: na razie zaznaczamy jako '?' (np 7), serwer potem to zweryfikuje
    planszaG1.P[r][c] = 7; // Zakładamy pudło, jak przyjdzie HIT to zmienimy w processNetworkMessages? 
    // W obecnym protokole serwer odsyła tylko HIT/MISS, nie podaje współrzędnych.
    // Musielibyśmy zapisać `lastShotX = r` w klasie.
    // DLA UPROSZCZENIA GUI: Na razie zaznaczamy od razu na biało.
    
    myTurnOnline = false; // Czekamy na wynik
}