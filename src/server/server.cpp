#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <algorithm>
#include <sstream>
#include "../common/plansza.h"

using namespace std;

const int MAX_HITS = 20;

struct Player {
    int socket;
    string name;
    Plansza board; // Serwerowa kopia planszy gracza
    int hitsScored;
};

// --- FUNKCJE POMOCNICZE ---

void sendMsg(int sock, string msg) {
    if (msg.empty() || msg.back() != '\n') msg += "\n";
    send(sock, msg.c_str(), msg.length(), 0);
}

string readMsg(int sock) {
    string msg = "";
    char c;
    while (true) {
        int valread = read(sock, &c, 1);
        if (valread > 0) {
            if (c == '\n') break;
            msg += c;
        } else {
            return ""; 
        }
    }
    while (!msg.empty() && isspace(msg.back())) msg.pop_back();
    return msg;
}

// Odbiera planszę od klienta (ciąg 100 cyfr) i zapisuje w obiekcie Plansza
bool receiveBoard(Player& p) {
    string rawBoard = readMsg(p.socket);
    if (rawBoard.length() < 100) return false;

    int idx = 0;
    for (int i = 1; i <= 10; i++) {
        for (int j = 1; j <= 10; j++) {
            // Zakładamy format: "0011100..."
            p.board.T[i][j] = rawBoard[idx] - '0';
            idx++;
        }
    }
    return true;
}

// --- LOGIKA SESJI GRY (WĄTEK) ---

void gameSession(int sock1, int sock2) {
    Player p1, p2;
    p1.socket = sock1;
    p2.socket = sock2;
    p1.hitsScored = 0;
    p2.hitsScored = 0;

    // 1. Handshake (Imiona)
    p1.name = readMsg(p1.socket);
    p2.name = readMsg(p2.socket);
    
    cout << "[MATCH] Rozpoczynanie gry: " << p1.name << " vs " << p2.name << endl;

    // 2. Pobieranie plansz (Setup)
    // Prosimy o wysłanie plansz
    sendMsg(p1.socket, "SEND_BOARD");
    sendMsg(p2.socket, "SEND_BOARD");

    if (!receiveBoard(p1) || !receiveBoard(p2)) {
        cout << "[MATCH] Błąd odbierania planszy. Koniec sesji.\n";
        close(sock1); close(sock2);
        return;
    }
    cout << "[MATCH] Plansze odebrane i zweryfikowane.\n";

    // 3. Pętla Gry
    Player* players[2] = {&p1, &p2};
    int currentTurn = 0; 
    bool gameRunning = true;

    // Informacja o starcie
    sendMsg(p1.socket, "START " + p2.name);
    sendMsg(p2.socket, "START " + p1.name);

    sendMsg(players[currentTurn]->socket, "TURN");
    sendMsg(players[1-currentTurn]->socket, "WAIT");

    while (gameRunning) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock1, &readfds);
        FD_SET(sock2, &readfds);
        int max_sd = max(sock1, sock2);

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0 && errno != EINTR) break;

        for (int i = 0; i < 2; i++) {
            if (FD_ISSET(players[i]->socket, &readfds)) {
                string msg = readMsg(players[i]->socket);
                
                if (msg == "") {
                    cout << "[MATCH] Gracz " << players[i]->name << " rozlaczyl sie.\n";
                    sendMsg(players[1-i]->socket, "OPPONENT_LEFT");
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    gameRunning = false;
                    break;
                }

                // CHAT
                if (msg.rfind("CHAT", 0) == 0) {
                    sendMsg(players[1-i]->socket, msg); 
                    continue;
                }

                // LOGIKA STRZAŁU (Tylko aktywny gracz)
                if (i == currentTurn && msg.rfind("SHOT", 0) == 0) {
                    // Format: "SHOT x y"
                    int x, y;
                    if (sscanf(msg.c_str(), "SHOT %d %d", &x, &y) == 2) {
                        
                        // === WALIDACJA SERWERA ===
                        if (x < 1 || x > 10 || y < 1 || y > 10) {
                            sendMsg(players[i]->socket, "ERROR Nieprawidlowe wspolrzedne");
                            continue;
                        }

                        // Sprawdzenie na planszy PRZECIWNIKA (players[1-i])
                        int targetState = players[1-i]->board.T[x][y];
                        
                        // Czy to statek? (1-4)
                        bool isHit = (targetState >= 1 && targetState <= 4);
                        
                        // Odpowiedź do strzelającego (RESULT HIT/MISS x y)
                        string resultCmd = "RESULT " + string(isHit ? "HIT " : "MISS ") + to_string(x) + " " + to_string(y);
                        sendMsg(players[i]->socket, resultCmd);

                        // Informacja do obrywającego (OPPONENT_SHOT x y)
                        string oppCmd = "OPPONENT_SHOT " + to_string(x) + " " + to_string(y);
                        sendMsg(players[1-i]->socket, oppCmd);

                        if (isHit) {
                            players[i]->hitsScored++;
                            players[1-i]->board.T[x][y] = 8; 

                            if (players[i]->hitsScored >= MAX_HITS) {
                                sendMsg(players[i]->socket, "WIN");
                                sendMsg(players[1-i]->socket, "LOSE");
                                gameRunning = false;
                            } else {
                                // Trafiony - strzela dalej (nie zmieniamy currentTurn)
                                sendMsg(players[i]->socket, "TURN");
                                sendMsg(players[1-i]->socket, "WAIT");
                            }
                        } else {
                            // Pudło - zmiana tury
                            currentTurn = 1 - currentTurn;
                            sendMsg(players[currentTurn]->socket, "TURN");
                            sendMsg(players[1-currentTurn]->socket, "WAIT");
                        }
                    }
                }
            }
        }
    }

    close(sock1);
    close(sock2);
    cout << "[MATCH] Sesja zakonczona.\n";
}


int main() {
    int port;
    cout << "Port serwera: ";
    if (!(cin >> port)) port = 8080;

    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) exit(EXIT_FAILURE);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (::bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed"); exit(EXIT_FAILURE);
    }
    listen(server_fd, 5);

    cout << "Serwer wielowatkowy gotowy na porcie " << port << ".\n";

    queue<int> waitingRoom;
    mutex roomMutex;

    while (true) {
        int new_socket;
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept error");
            continue;
        }

        cout << "[CONN] Nowe polaczenie: " << new_socket << endl;

        roomMutex.lock();
        waitingRoom.push(new_socket);
        
        if (waitingRoom.size() >= 2) {
            int p1 = waitingRoom.front(); waitingRoom.pop();
            int p2 = waitingRoom.front(); waitingRoom.pop();
            
            // Uruchomienie osobnego wątku dla tej pary
            thread t(gameSession, p1, p2);
            t.detach();
        } else {
            string waitMsg = "INFO Oczekiwanie na drugiego gracza...\n";
            send(new_socket, waitMsg.c_str(), waitMsg.length(), 0);
        }
        roomMutex.unlock();
    }

    return 0;
}