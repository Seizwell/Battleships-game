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

const int PORT = 8080;
//const int BUFFER_SIZE = 1024;
const int MAX_HITS = 20; // 1x4 + 2x3 + 3x2 + 4x1 = 20 masztów do zestrzelenia

struct Player {
    int socket;
    string name;
    int hits; // Licznik trafień (do sprawdzania wygranej)
};

// Funkcja pomocnicza do wysyłania wiadomości
void sendMsg(int sock, string msg) {
    // Dodajemy nową linię, jeśli jej nie ma, dla pewności buforowania
    if (msg.back() != '\n') msg += "\n";
    send(sock, msg.c_str(), msg.length(), 0);
}

// Funkcja pomocnicza do odbierania wiadomości (i czyszczenia białych znaków)
string readMsg(int sock) {
    string msg = "";
    char c;
    int retry_count = 0;
    
    // Serwer jest bardziej cierpliwy - np. 3 minuty (36 * 5s)
    const int MAX_RETRIES = 36; 

    while (true) {
        int valread = read(sock, &c, 1);
        
        if (valread > 0) {
            if (c == '\n') break;
            msg += c;
        } 
        else if (valread == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                retry_count++;
                // Serwer tylko loguje w konsoli, nie wysyła do klienta
                if (retry_count % 2 == 0) { // Co 10 sekund log
                    cout << "Oczekiwanie na dane z socketu " << sock << "..." << endl;
                }
                
                if (retry_count >= MAX_RETRIES) {
                    return ""; // Timeout definitywny
                }
                continue;
            } else {
                return ""; // Błąd krytyczny
            }
        } 
        else {
            return ""; // Rozłączenie
        }
    }
    while (!msg.empty() && isspace(msg.back())) msg.pop_back();
    return msg;
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // 1. Tworzenie gniazda
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Opcje gniazda (pozwala na szybki restart serwera na tym samym porcie)
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Nasłuchuj na wszystkich interfejsach
    address.sin_port = htons(PORT);

    // 2. Bindowanie (przypisanie adresu i portu)
    if (::bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // 3. Nasłuchiwanie
    if (listen(server_fd, 2) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    cout << "Serwer nasłuchuje na porcie " << PORT << "..." << endl;

    Player players[2];

    // 4. Akceptowanie graczy
    for (int i = 0; i < 2; i++) {
        cout << "Oczekiwanie na Gracza " << i + 1 << "..." << endl;
        if ((players[i].socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        int opt = 1;
        setsockopt(players[i].socket, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));

        // 2. Timeout (5 sekund)
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        setsockopt(players[i].socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

        cout << "Gracz " << i + 1 << " połączony!" << endl;
        
        // Pobierz imię
        players[i].name = readMsg(players[i].socket);
        players[i].hits = 0;
        cout << "Gracz " << i + 1 << " przedstawił się jako: " << players[i].name << endl;
    }

    cout << "=== ROZPOCZĘCIE GRY ===" << endl;
    
    // 5. Główna pętla gry
    int current = 0; // Indeks gracza, który ma turę (0 lub 1)
    bool gameRunning = true;

    while (gameRunning) {
        int other = (current + 1) % 2; // Indeks przeciwnika

        // Informujemy graczy o stanie tury
        sendMsg(players[current].socket, "TURN");
        sendMsg(players[other].socket, "WAIT");

        cout << "Tura: " << players[current].name << endl;

        // Odbieramy współrzędne strzału od aktualnego gracza
        string msg = readMsg(players[current].socket);
        if (msg == "") {
            cout << "Gracz " << players[current].name << " rozłączył się." << endl;
            break;
        }

        if (msg.rfind("CHAT", 0) == 0) { // Jeśli wiadomość zaczyna się od "CHAT"
            // Prześlij do drugiego gracza i NIC WIĘCEJ NIE RÓB (nie zmieniaj tury)
            sendMsg(players[other].socket, msg);
            cout << "[CHAT] " << players[current].name << ": " << msg.substr(5) << endl;
            continue; // Wracamy na początek pętli (czekamy na dalsze komendy)
        }
        // ===================================

        // Jeśli to nie czat, zakładamy że to współrzędne (logika gry)
        string coords = msg;

        // Przesyłamy informację o strzale do przeciwnika
        // Format: "SHOT x y"
        sendMsg(players[other].socket, "SHOT " + coords);

        // Odbieramy wynik strzału od przeciwnika (HIT lub MISS)
        string result = readMsg(players[other].socket);
        if (result == "") {
            cout << "Gracz " << players[other].name << " rozłączył się." << endl;
            break;
        }

        cout << "Strzał gracza " << players[current].name << " w " << coords << " -> " << result << endl;

        // Przesyłamy wynik z powrotem do strzelającego (żeby zaktualizował mapę P)
        sendMsg(players[current].socket, result);

        // Logika wygranej
        if (result == "HIT") {
            players[current].hits++;
            
            // Sprawdzenie czy wygrał
            if (players[current].hits >= MAX_HITS) {
                sendMsg(players[current].socket, "WIN");
                sendMsg(players[other].socket, "LOSE");
                cout << "Gracz " << players[current].name << " WYGRAŁ!" << endl;
                gameRunning = false;
                break;
            }
            current = other;
        } else {
            current = other;
        }
        // Jeśli MISS, zmiana tury. Jeśli HIT, tura zostaje (opcjonalna zasada, 
        // ale w Twoim localgame tura się zmienia zawsze, więc tu też zmieniamy).
        // W standardowych statkach po trafieniu strzela się dalej, 
        // ale w Twoim kodzie localgame tura przechodzi. Utrzymujemy zgodność z localgame.
        current = other; 
    }

    // Zamknięcie gniazd
    close(players[0].socket);
    close(players[1].socket);
    close(server_fd);

    return 0;
}