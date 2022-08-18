#ifndef BOARD_H_INCLUDED
#define BOARD_H_INCLUDED

#include <cassert>
#include <cstdint>
#include <tuple>

inline bool IsValidColor(int i) { return i >= 1 && i <= 7; }

inline bool IsValidCoords(int r, int c) { return r >= 0 && r < 7 && c >= 0 && c < 7; }

struct Point {
  char r, c;

  Point(const Point &p) : r(p.r), c(p.c) {}
  Point(int r, int c) : r(static_cast<decltype(this->r)>(r)), c(static_cast<decltype(this->c)>(c)) {}

  static Point Invalid() {
    return Point(-1, -1);
  }
};

inline bool operator==(const Point &p, const Point &q) { return std::tie(p.r, p.c) == std::tie(q.r, q.c); }
inline bool operator!=(const Point &p, const Point &q) { return std::tie(p.r, p.c) != std::tie(q.r, q.c); }
inline bool operator< (const Point &p, const Point &q) { return std::tie(p.r, p.c) <  std::tie(q.r, q.c); }
inline bool operator<=(const Point &p, const Point &q) { return std::tie(p.r, p.c) <= std::tie(q.r, q.c); }
inline bool operator> (const Point &p, const Point &q) { return std::tie(p.r, p.c) >  std::tie(q.r, q.c); }
inline bool operator>=(const Point &p, const Point &q) { return std::tie(p.r, p.c) >= std::tie(q.r, q.c); }

struct Movement {
  Point src, dst;

  Movement(const Movement &m) : src(m.src), dst(m.dst) {}
  Movement(Point src, Point dst) : src(src), dst(dst) {}
  Movement(int r1, int c1, int r2, int c2) : src(r1, c1), dst(r2, c2) {}

  static Movement Invalid() {
    return Movement(Point::Invalid(), Point::Invalid());
  }
};

inline bool operator==(const Movement &m, const Movement &n) { return std::tie(m.src, m.dst) == std::tie(n.src, n.dst); }
inline bool operator!=(const Movement &m, const Movement &n) { return std::tie(m.src, m.dst) != std::tie(n.src, n.dst); }
inline bool operator< (const Movement &m, const Movement &n) { return std::tie(m.src, m.dst) <  std::tie(n.src, n.dst); }
inline bool operator<=(const Movement &m, const Movement &n) { return std::tie(m.src, m.dst) <= std::tie(n.src, n.dst); }
inline bool operator> (const Movement &m, const Movement &n) { return std::tie(m.src, m.dst) >  std::tie(n.src, n.dst); }
inline bool operator>=(const Movement &m, const Movement &n) { return std::tie(m.src, m.dst) >= std::tie(n.src, n.dst); }

enum Player : char {
  NO_PLAYER  = 0,
  CHAOS      = 1,
  ORDER      = 2,
};

struct Placement {
  Point dst;
  char color;

  Placement(const Placement &p) : dst(p.dst), color(p.color) {}
  Placement(const Point &dst, int color) : dst(dst), color(static_cast<decltype(this->color)>(color)) {}
  Placement(int r, int c, int color) : dst(r, c), color(static_cast<decltype(this->color)>(color)) {}
};

struct Board {
  // Pieces on the board. 0 if empty, or a color between 1 and 7 (inclusive)
  union {
    char pieces[7][7];
    char pieces_contiguous[49];
  };

  static Board Init() {
    return Board{};
  }

  bool IsEmpty() const;
  bool IsFull() const;

  bool IsValidPlace(int r, int c, int color) const;

  bool IsValidPlace(Placement place) const {
    return IsValidPlace(place.dst.r, place.dst.c, place.color);
  }

  void Place(int r, int c, int color) {
    pieces[r][c] = color;
  }

  void Place(Placement place) {
    Place(place.dst.r, place.dst.c, place.color);
  }

  bool IsValidMove(int r1, int c1, int r2, int c2) const;
  bool IsValidMove(Movement move) const {
    return IsValidMove(move.src.r, move.src.c, move.dst.r, move.dst.c);
  }

  void Move(int r1, int c1, int r2, int c2) {
    std::swap(pieces[r1][c1], pieces[r2][c2]);
  }
  void Move(Movement move) {
    Move(move.src.r, move.src.c, move.dst.r, move.dst.c);
  }
};

// Unused; remove?
struct Bag {
  union {
    // Number of pieces per color.
    uint8_t count[8];

    uint64_t count_combined;
  };

  int PiecesLeft() const {
    uint64_t a = count_combined;
    uint32_t b = (a >> 32) + (a & 0xffffffff);
    uint16_t c = (b >> 16) + (b & 0xffff);
    uint8_t  d = (c >>  8) + (c & 0xff);
    return d;
  }

  static Bag Init() {
    return Bag{.count = {0, 7, 7, 7, 7, 7, 7, 7}};
  }
};

inline Bag BagFromBoard(const Board &board) {
  Bag bag = Bag::Init();
  for (int color : board.pieces_contiguous) {
    if (color != 0) {
      assert(IsValidColor(color));
      assert(bag.count[color] > 0);
      --bag.count[color];
    }
  }
  return bag;
}

// Unused; remove?
struct PlayerMove {
  Player player;  // CHAOS or ORDER
  union {
    Placement place;   // if player == CHAOS
    Movement move;     // if player == ORDER
  };
};

#endif // ndef BOARD_H_INCLUDED
