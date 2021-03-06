#ifndef SOFCHECK_INTF_H_INCLUDED
#define SOFCHECK_INTF_H_INCLUDED

#include "core/board.h"
#include "core/init.h"
#include "core/move.h"
#include "core/movegen.h"
#include "core/strutil.h"
#include "core/test/selftest.h"
#include "util/misc.h"

namespace ChessIntf {

#define CHESS_INTF_HAS_SELF_TESTS

using Board = SoFCore::Board;
using Move = SoFCore::Move;
using MovePersistence = SoFCore::MovePersistence;

constexpr int MOVES_MAX = SoFCore::BUFSZ_MOVES;

struct MoveList {
  Move moves[MOVES_MAX];
  int count;
};

inline const char *getImplName() { return "SoFCheck"; }

inline void selfTest(Board board) { SoFCore::Test::runSelfTest(board); }

inline void init() { SoFCore::init(); }

inline const Move &getMove(const MoveList &lst, int idx) { return lst.moves[idx]; }

inline int getMoveCount(const MoveList &lst) { return lst.count; }

inline Board boardFromFen(const char *fen) {
  Board board;  // NOLINT: uninitialized
  SoFCore::FenParseResult result = board.setFromFen(fen);
  if (result != SoFCore::FenParseResult::Ok) {
    SoFUtil::panic("the given FEN is invalid");
  }
  return board;
}

inline MovePersistence makeMove(Board &board, const Move &move) {
  return SoFCore::moveMake(board, move);
}

inline void unmakeMove(Board &board, const Move &move, MovePersistence &p) {
  SoFCore::moveUnmake(board, move, p);
}

inline void moveStr(const Board &, const Move &mv, char *str) { SoFCore::moveToStr(mv, str); }

inline MoveList generateMoves(const Board &board) {
  MoveList moves;
  moves.count = static_cast<size_t>(SoFCore::genAllMoves(board, moves.moves));
  return moves;
}

inline bool isAttacked(const Board &board, bool isWhite, char cy, char cx) {
  using namespace SoFCore;
  coord_t coord = charsToCoord(cy, cx);
  if (isWhite) {
    return isCellAttacked<Color::White>(board, coord);
  } else {
    return isCellAttacked<Color::Black>(board, coord);
  }
}

inline bool isOpponentKingAttacked(const Board &board) { return !isMoveLegal(board); }

}  // namespace ChessIntf

#endif
