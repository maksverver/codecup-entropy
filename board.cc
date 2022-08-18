#include "board.h"

bool Board::IsEmpty() const {
  for (int color : pieces_contiguous) if (color != 0) return false;
  return true;
}

bool Board::IsFull() const {
  for (int color : pieces_contiguous) if (color == 0) return false;
  return true;
}

bool Board::IsValidPlace(int r, int c, int color) const {
  return IsValidCoords(r, c) && IsValidColor(color) && pieces[r][c] == 0;
}

bool Board::IsValidMove(int r1, int c1, int r2, int c2) const {
  if (!IsValidCoords(r1, c1) || !IsValidCoords(r2, c2) || pieces[r1][c1] == 0) return false;
  if (r1 == r2) {
    if (c1 > c2) std::swap(c1, c2);
    for (int c = c1 + 1; c < c2; ++c) if (pieces[r1][c] != 0) return false;
    return true;
  }
  if (c1 == c2) {
    if (r1 > r2) std::swap(r1, r2);
    for (int r = r1 + 1; r < r2; ++r) if (pieces[r][c1] != 0) return false;
    return true;
  }
  return false;  // non-orthogonal move.
}
