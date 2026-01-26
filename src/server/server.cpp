#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <sys/time.h>
#include <cerrno>

using namespace std;

// const int PORT = 8080;
const int MAX_HITS = 20;

struct Player {
    int socket;
    string name;
    int hits;
};

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

int main() {
    int port;
    cout << "=== KONFIGURACJA SERWERA ===" << endl;
    cout << "Podaj PORT nasluchu (domyslnie 8080): ";
    if (!(cin >> port)) {
        port = 8080;
        cin.clear(); cin.ignore(1000, '\n');
    }
    cin.ignore(1000, '\n'); // Czyszczenie bufora

    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (::bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 2) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    cout << "Serwer nasluchuje na porcie " << port << "..." << endl;

    Player players[2];

    for (int i = 0; i < 2; i++) {
        cout << "Oczekiwanie na Gracza " << i + 1 << "..." << endl;
        if ((players[i].socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        cout << "Gracz " << i + 1 << " polaczony!" << endl;
        
        struct timeval tv; tv.tv_sec = 60; tv.tv_usec = 0;
        setsockopt(players[i].socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

        players[i].name = readMsg(players[i].socket);
        players[i].hits = 0;
        cout << "Gracz " << i + 1 << ": " << players[i].name << endl;
    }

    cout << "=== START GRY ===" << endl;

    int currentTurn = 0; 
    bool waitingForResult = false; 
    bool gameRunning = true;

    sendMsg(players[0].socket, "TURN");
    sendMsg(players[1].socket, "WAIT");

    while (gameRunning) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(players[0].socket, &readfds);
        FD_SET(players[1].socket, &readfds);
        int max_sd = max(players[0].socket, players[1].socket);

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            perror("select error");
        }

        for (int i = 0; i < 2; i++) {
            if (FD_ISSET(players[i].socket, &readfds)) {
                string msg = readMsg(players[i].socket);
                
                if (msg == "") {
                    cout << "Gracz " << players[i].name << " rozlaczyl sie.\n";
                    gameRunning = false;
                    break;
                }

                if (msg.rfind("CHAT", 0) == 0) {
                    cout << "[CHAT] " << players[i].name << ": " << msg.substr(5) << endl;
                    sendMsg(players[1-i].socket, msg); 
                    continue; 
                }

                if (i == currentTurn && !waitingForResult) {
                    cout << "Strzal gracza " << players[i].name << ": " << msg << endl;
                    sendMsg(players[1-i].socket, "SHOT " + msg);
                    waitingForResult = true;
                }
                else if (i != currentTurn && waitingForResult) {
                    cout << "Wynik od " << players[i].name << ": " << msg << endl;
                    sendMsg(players[currentTurn].socket, msg);
                    
                    if (msg == "HIT") {
                        players[currentTurn].hits++;
                        if (players[currentTurn].hits >= MAX_HITS) {
                            sendMsg(players[currentTurn].socket, "WIN");
                            sendMsg(players[1-currentTurn].socket, "LOSE");
                            gameRunning = false;
                        } else {
                            // TRAFIONY - STRZELA DALEJ
                            waitingForResult = false;
                            
                            // Ponawiamy statusy (dla pewności klienta)
                            sendMsg(players[currentTurn].socket, "TURN");
                            sendMsg(players[1-currentTurn].socket, "WAIT");
                            cout << "Trafienie! Gracz " << players[currentTurn].name << " strzela ponownie.\n";
                        }
                    } else {
                        // === PUDŁO: ZMIANA TURY ===
                        currentTurn = 1 - currentTurn;
                        waitingForResult = false;
                        
                        sendMsg(players[currentTurn].socket, "TURN");
                        sendMsg(players[1-currentTurn].socket, "WAIT");
                        cout << "Pudlo. Tura gracza: " << players[currentTurn].name << endl;
                    }
                }
            }
        }
    }

    close(players[0].socket);
    close(players[1].socket);
    close(server_fd);
    return 0;
}