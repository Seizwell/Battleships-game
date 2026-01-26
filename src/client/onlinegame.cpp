#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <limits>
#include <sys/time.h>
#include <cerrno>

#include "onlinegame.h"

using namespace std;

OnlineGame::OnlineGame() : sock(-1), connected(false), running(false) {}

OnlineGame::~OnlineGame() {
    running = false;
    if (sock != -1) close(sock);
    if (networkThread.joinable()) networkThread.join();
}

void OnlineGame::inicjalizujStatki() {
    cout << "Losowanie Twoich statków...\n";
    mojaPlansza.ZERUJ();
    mojaPlansza.los4();
    for(int i = 0; i < 2; i++) mojaPlansza.los3();
    for(int i = 0; i < 3; i++) mojaPlansza.los2();
    for(int i = 0; i < 4; i++) mojaPlansza.los1();
    
    cout << "Twoja flota gotowa:\n";
    mojaPlansza.piszT(); // Używamy Twojej funkcji z plansza.cpp
}

bool OnlineGame::polacz(string ip, int port) {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Nie można utworzyć gniazda");
        return false;
    }

    // 1. Ustawienie Keep-Alive (chroni przed zerwaniem przy braku aktywności)
    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt))) {
        perror("Błąd ustawiania SO_KEEPALIVE");
    }

    // 2. Ustawienie Timeoutu (żeby read nie wisiał w nieskończoność)
    struct timeval tv;
    tv.tv_sec = 5;  // 5 sekund czekania
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if(inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0) {
        cout << "Błędny adres IP\n";
        return false;
    }

    cout << "Łączenie z " << ip << ":" << port << "...\n";
    if (::connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Błąd połączenia");
        return false;
    }

    connected = true;
    cout << "Połączono z serwerem!\n";
    return true;
}

void OnlineGame::wyslijWiadomosc(string msg) {
    msg += "\n";
    send(sock, msg.c_str(), msg.length(), 0);
}

string rawRead(int sock) {
    string msg = "";
    char c;
    
    // Pętla czytania znak po znaku
    while (true) {
        int valread = read(sock, &c, 1);
        
        if (valread > 0) {
            // Odebrano znak poprawnie
            if (c == '\n') break; // Koniec linii
            msg += c;
        } 
        else if (valread == 0) {
            // Serwer zamknął połączenie (FIN)
            return ""; 
        }
        else { 
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue; 
            }
            return ""; 
        }
    }
    
    // Czyszczenie białych znaków z końca
    while (!msg.empty() && isspace(msg.back())) msg.pop_back();
    return msg;
}

void OnlineGame::watekNasluchiwania() {
    while (running && connected) {
        string msg = rawRead(sock);
        
        if (msg == "") {
            connected = false;
            queueCV.notify_all(); 
            break;
        }

        if (msg.rfind("CHAT", 0) == 0) {
            cout << "\n\033[1;33m[CHAT PRZECIWNIK]: " << msg.substr(5) << "\033[0m\n"; 
        } 
        else {
            {
                lock_guard<mutex> lock(queueMutex);
                gameMsgQueue.push(msg);
            }
            queueCV.notify_one(); // Obudź główny wątek
        }
    }
}

string OnlineGame::pobierzKomendeGry() {
    unique_lock<mutex> lock(queueMutex);
    queueCV.wait(lock, [this]{ return !gameMsgQueue.empty() || !connected; });

    if (!connected) return "";

    string msg = gameMsgQueue.front();
    gameMsgQueue.pop();
    return msg;
}

void OnlineGame::uruchom(string ip, int port, string nazwaGracza) {
    if (!polacz(ip, port)) return;

    // Start wątku nasłuchującego
    running = true;
    networkThread = thread(&OnlineGame::watekNasluchiwania, this);

    cout << "Wysyłanie nazwy: " << nazwaGracza << "...\n";
    wyslijWiadomosc(nazwaGracza);
    gra.set_nazwag1(nazwaGracza);

    inicjalizujStatki();
    cout << "Oczekiwanie na drugiego gracza...\n";
    cout << "TIP: Wpisz '/wiadomosc' podczas swojej tury, aby czatować!\n";

    while (connected) {
        // Zmienione: pobieramy z kolejki, nie z sieci bezpośrednio
        string cmd = pobierzKomendeGry(); 
        
        if (cmd == "") break; // Koniec gry

        if (cmd == "WAIT") {
            cout << "Ruch przeciwnika...\n";
        } 
        else if (cmd == "TURN") {
            obslugaTury();
        } 
        else if (cmd.rfind("SHOT", 0) == 0) { 
             int x, y;
             sscanf(cmd.c_str(), "SHOT %d %d", &x, &y);
             cout << "Przeciwnik strzela w pole " << x << " " << y << "!\n";
             
             // Logika trafienia
             int dummyP[12][12];
             gra.Strzal(mojaPlansza.T, dummyP, x, y);
             int wynik = mojaPlansza.T[x][y];
             if (wynik == 8) wyslijWiadomosc("HIT");
             else wyslijWiadomosc("MISS");
             mojaPlansza.piszT();
        }
        else if (cmd == "WIN" || cmd == "LOSE") {
            cout << (cmd == "WIN" ? "WYGRAŁEŚ!" : "PRZEGRAŁEŚ!") << endl;
            break;
        }
    }
}

void OnlineGame::obslugaTury() {
    cout << "\n=== TWOJA TURA ===\n";
    mojaPlansza.piszP();
    
    string input;
    int x, y;

    while (true) {
        cout << "Podaj Wiersz (lub /tekst): ";
        cin >> input;

        // OBSŁUGA CZATU WYCHODZĄCEGO
        if (input[0] == '/') {
            string text = input.substr(1); // Pomiń '/'
            string resztaLinii;
            getline(cin, resztaLinii); // Pobierz resztę zdania po spacji
            text += resztaLinii;
            
            wyslijWiadomosc("CHAT " + text);
            cout << "[TY]: " << text << "\n";
            continue; // Wróć do pytania o wiersz
        }

        // Próba konwersji na liczbę
        try {
            x = stoi(input);
        } catch (...) {
            cout << "To nie liczba!\n";
            cin.clear(); cin.ignore(1000, '\n');
            continue;
        }

        cout << "Podaj Kolumne: ";
        if (!(cin >> y)) {
            cin.clear(); cin.ignore(1000, '\n');
            continue;
        }

        if (x < 1 || x > 10 || y < 1 || y > 10) {
            cout << "Zakres 1-10.\n";
            continue;
        }

        if (mojaPlansza.P[x][y] != 0) { // 0 to woda/nieodkryte
            cout << "Już strzelałeś w to pole! Wybierz inne.\n";
            continue;
        }
        cin.ignore(1000, '\n');
        break;
    }

    // Wysyłamy strzał
    string msg = to_string(x) + " " + to_string(y);
    wyslijWiadomosc(msg);
    
    // Czekamy na odpowiedź (HIT/MISS) - ona przyjdzie przez wątek do kolejki
    string odp = pobierzKomendeGry();
    
    if (odp == "HIT") {
        cout << "TRAFIONY!\n";
        mojaPlansza.P[x][y] = 8;
    } else if (odp == "MISS") {
        cout << "Pudło.\n";
        mojaPlansza.P[x][y] = 7;
    }
}

string serializeBoard(int T[12][12]) {
    string s = "";
    for (int i = 1; i <= 10; i++) {
        for (int j = 1; j <= 10; j++) {
            s += to_string(T[i][j]);
        }
    }
    return s;
}

string boardToString(int T[12][12]) {
    string s = "";
    for (int i = 1; i <= 10; i++) {
        for (int j = 1; j <= 10; j++) {
            s += to_string(T[i][j]);
        }
    }
    return s;
}