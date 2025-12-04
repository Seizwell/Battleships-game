CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -Isrc/common
BINDIR = bin
COMMONDIR = src/common

# Pliki współdzielone
COMMON_SOURCES = $(COMMONDIR)/gra.cpp $(COMMONDIR)/plansza.cpp
COMMON_HEADERS = $(COMMONDIR)/gra.h $(COMMONDIR)/plansza.h

all: $(BINDIR)/server.out $(BINDIR)/client.out

# Serwer zależy od server.cpp i wszystkich plików z common
$(BINDIR)/server.out: src/server/server.cpp $(COMMON_SOURCES) $(COMMON_HEADERS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) src/server/server.cpp $(COMMON_SOURCES) -o $@
	@echo "✓ Serwer skompilowany"

# Klient zależy od client.cpp i wszystkich plików z common
$(BINDIR)/client.out: src/client/client.cpp $(COMMON_SOURCES) $(COMMON_HEADERS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) src/client/client.cpp $(COMMON_SOURCES) -o $@
	@echo "✓ Klient skompilowany"

clean:
	rm -rf $(BINDIR)/*
	@echo "✓ Wyczyszczono"

# Pomocne cele
run-server: $(BINDIR)/server.out
	./$(BINDIR)/server.out

run-client: $(BINDIR)/client.out
	./$(BINDIR)/client.out

.PHONY: all clean run-server run-client