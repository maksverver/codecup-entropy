#include "board.h"

#include <cassert>
#include <algorithm>
#include <iostream>
#include <string>

namespace {

Point PlayChaos(const Board &board, char color) {
  for (char r = 0; r < 7; ++r) {
    for (char c = 0; c < 7; ++c) {
      if (board.pieces[r][c] == 0) {
        return Point{r, c};
      }
    }
  }

  assert(false);  // board is full.
  return Point::Invalid();
}

Movement PlayOrder(const Board &board) {
  for (char r = 0; r < 7; ++r) {
    for (char c = 0; c < 7; ++c) {
      if (board.pieces[r][c] != 0) {
        return Movement(r, c, r, c);
      }
    }
  }
  assert(false);  // board is full
  return Movement::Invalid();
}

}  // namespace

int main() {
  Board board = Board::Init();

  Player next_player = CHAOS;
  Player my_player = NO_PLAYER;
  std::string line;
  for (;;) {
    if (!std::getline(std::cin, line)) {
      std::cerr << "End of input." << std::endl;
      break;
    }

    if (line == "Quit") {
      std::cerr << "Quit received." << std::endl;
      break;
    }

    if (line == "Start") {
      std::cerr << "Start received" << std::endl;
      assert(my_player == NO_PLAYER);
      my_player = CHAOS;
      continue;
    }

    if (my_player == NO_PLAYER) my_player = ORDER;

    if (my_player == CHAOS) {
      if (next_player == CHAOS) {
        // Read piece color.
        assert(line.size() == 1);
        int color = line[0] - '0';
        assert(IsValidColor(color));

        // Choose where to place color.
        Placement place(PlayChaos(board, color), color);

        // Execute place.
        assert(board.IsValidPlace(place));
        board.Place(place);
        next_player = ORDER;

        // Output place.
        std::cout
            << static_cast<char>('A' + place.dst.r)
            << static_cast<char>('a' + place.dst.c)
            << std::endl;
      } else {
        assert(next_player == ORDER);
        // Read Movement from Order.
        assert(line.size() == 4);
        int r1 = line[0] - 'A';
        int c1 = line[1] - 'a';
        int r2 = line[2] - 'A';
        int c2 = line[3] - 'a';
        Movement move(r1, c1, r2, c2);
        assert(board.IsValidMove(move));
        board.Move(move);
        next_player = CHAOS;
      }
    } else {
      assert(my_player == ORDER);

      // Read piece and place from Chaos.
      assert(line.size() == 3);
      int color = line[0] - '0';
      int r = line[1] - 'A';
      int c = line[2] - 'a';
      Placement place(r, c, color);
      assert(board.IsValidPlace(place));
      board.Place(place);

      // Choose where to move.
      Movement move = PlayOrder(board);

      // Execute move.
      assert(board.IsValidMove(move));
      board.Move(move);
      next_player = ORDER;

      // Output move.
      std::cout
          << static_cast<char>('A' + move.src.r)
          << static_cast<char>('a' + move.src.c)
          << static_cast<char>('A' + move.dst.r)
          << static_cast<char>('a' + move.dst.c)
          << std::endl;
    }
  }
  std::cerr << "Exiting." << std::endl;
  return 0;
}
