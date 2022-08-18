CXXFLAGS=-Wall -pipe -O2 -march=native -g --std=c++17 -Wno-char-subscripts
LDLIBS=-lm
BINS=player
OBJS=board.o

all: $(BINS)

clean:
	rm -f $(BINS) $(OBJS)

player: player.cc $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)
