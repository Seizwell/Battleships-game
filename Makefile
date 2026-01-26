CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Isrc/common

# Wykrywanie systemu operacyjnego
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Darwin)
    # === macOS ===
    # Homebrew zazwyczaj instaluje tutaj:
    CXXFLAGS += -I/usr/local/include -I/opt/homebrew/include
    LDFLAGS = -L/usr/local/lib -L/opt/homebrew/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network
else
    # === Linux ===
    # Na Linuxie biblioteki są zazwyczaj w standardowych ścieżkach (/usr/include, /usr/lib)
    # więc nie trzeba dodawać -I ani -L, linker sam je znajdzie.
    LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network -lpthread
endif

BINDIR = bin
COMMONDIR = src/common
CLIENTDIR = src/client

# Pliki źródłowe
COMMON_SRC = $(COMMONDIR)/gra.cpp $(COMMONDIR)/plansza.cpp
SERVER_SRC = src/server/server.cpp
CLIENT_SRC = $(CLIENTDIR)/client.cpp $(CLIENTDIR)/localgame.cpp $(CLIENTDIR)/onlinegame.cpp $(CLIENTDIR)/guigame.cpp

# Pliki nagłówkowe (dla zależności)
COMMON_HDR = $(COMMONDIR)/gra.h $(COMMONDIR)/plansza.h
CLIENT_HDR = $(CLIENTDIR)/localgame.h $(CLIENTDIR)/onlinegame.h $(CLIENTDIR)/guigame.h

all: $(BINDIR)/server $(BINDIR)/client

# Serwer
$(BINDIR)/server: $(SERVER_SRC) $(COMMON_SRC) $(COMMON_HDR)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(SERVER_SRC) $(COMMON_SRC) -o $@ $(LDFLAGS)
	@echo "✓ Serwer skompilowany ($(UNAME_S))"

# Klient
$(BINDIR)/client: $(CLIENT_SRC) $(COMMON_SRC) $(COMMON_HDR) $(CLIENT_HDR)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(CLIENT_SRC) $(COMMON_SRC) -o $@ $(LDFLAGS)
	@echo "✓ Klient skompilowany ($(UNAME_S))"

clean:
	rm -rf $(BINDIR)/*
	@echo "✓ Wyczyszczono"

# Uruchamianie
run-server: $(BINDIR)/server
	./$(BINDIR)/server

run-client: $(BINDIR)/client
	./$(BINDIR)/client