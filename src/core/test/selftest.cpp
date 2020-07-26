#include "core/test/selftest.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

#include "core/move.h"
#include "core/move_parser.h"
#include "core/movegen.h"
#include "core/strutil.h"
#include "util/misc.h"

namespace SoFCore::Test {

using SoFUtil::panic;

void testBoardValid(const Board &b) {
  Board copied = b;
  if (copied.validate() != ValidateResult::Ok) {
    panic("Board::validate() reported that the board is invalid");
  }
  if (copied.castling != b.castling) {
    panic("Casting flags are incorrect");
  }
  if (copied.enpassantCoord != b.enpassantCoord) {
    panic("enpassantCoord is incorrect");
  }
  if (copied.bbAll != b.bbAll) {
    panic("bbAll is incorrect");
  }
  if (copied.bbWhite != b.bbWhite) {
    panic("bbWhite is incorrect");
  }
  if (copied.bbBlack != b.bbBlack) {
    panic("bbBlack is incorrect");
  }
  for (cell_t i = 0; i < static_cast<cell_t>(Board::BB_PIECES_SZ); ++i) {
    if (copied.bbPieces[i] != b.bbPieces[i]) {
      panic("bbPieces[" + std::to_string(i) + "] is incorrect");
    }
  }
  if (copied.hash != b.hash) {
    panic("hash is incorrect");
  }
}

static bool boardsBitCompare(const Board &a, const Board &b) {
  return std::memcmp(reinterpret_cast<const void *>(&a), reinterpret_cast<const void *>(&b),
                     sizeof(Board)) == 0;
}

void runSelfTest(Board b) {
  // Check that the board itself is valid
  testBoardValid(b);

  // Check that asFen and setFromFen are symmetrical
  char fen[4096];
  std::memset(fen, '?', sizeof(fen));
  b.asFen(fen);
  fen[4095] = '\0';
  if (std::strlen(fen) + 1 > BUFSZ_BOARD_FEN) {
    panic("Buffer constant for FEN is too slow");
  }
  auto loadResult = Board::fromFen(fen);
  if (!loadResult.isOk()) {
    panic("Cannot load from the board from its own FEN");
  }
  const Board loaded = loadResult.unwrap();
  if (!boardsBitCompare(b, loaded)) {
    panic("Loading the board from FEN produces a different board");
  }

  // Check that asPretty doesn't overflow the buffer
  const std::vector<std::pair<BoardPrettyStyle, size_t>> bufSizes = {
      {BoardPrettyStyle::Ascii, BUFSZ_BOARD_PRETTY_ASCII},
      {BoardPrettyStyle::Utf8, BUFSZ_BOARD_PRETTY_UTF8}};
  for (auto [style, bufSize] : bufSizes) {
    char pretty[4096];
    std::memset(pretty, '?', sizeof(pretty));
    b.asPretty(pretty, style);
    pretty[4095] = '\0';
    if (std::strlen(pretty) + 1 > bufSize) {
      panic("Buffer constant for pretty board is too slow");
    }
  }

  auto cmpMoves = [](Move a, Move b) { return a.asUint() < b.asUint(); };

  // Try to generate moves in total and compare the result if we generate simple moves and captures
  // separately
  Move moves[300];
  size_t moveCnt = genAllMoves(b, moves);
  Move movesSeparate[300];
  size_t moveSeparateCnt = genSimpleMoves(b, movesSeparate);
  moveSeparateCnt += genCaptures(b, movesSeparate + moveSeparateCnt);
  if (moveSeparateCnt != moveCnt) {
    panic("Moves generated by genAllMoves() differ from genSimpleMoves() + genCaptures()");
  }
  std::sort(moves, moves + moveCnt, cmpMoves);
  std::sort(movesSeparate, movesSeparate + moveCnt, cmpMoves);
  if (!std::equal(moves, moves + moveCnt, movesSeparate)) {
    panic("Moves generated by genAllMoves() differ from genSimpleMoves() + genCaptures()");
  }

  // Check that all the generated moves are well-formed
  for (size_t i = 0; i < moveCnt; ++i) {
    if (!moves[i].isWellFormed(b.side)) {
      panic("Move \"" + moveToStr(moves[i]) + "\" generated by genAllMoves() is not well-formed");
    }
  }

  // Check that move parser works correctly
  for (size_t i = 0; i < moveCnt; ++i) {
    const Move move = moves[i];
    char str[BUFSZ_MOVE_STR];
    moveToStr(move, str);
    const Move move2 = moveParse(str, b);
    if (move != move2) {
      panic("Move \"" + std::string(str) +
            "\" changed after being converted to string and vice versa");
    }
  }

  // Check that a well-formed move is generated by `genAllMoves()` iff isMoveValid returns true
  std::vector<Move> validMoves;
  const MoveKind kinds[] = {MoveKind::Null,
                            MoveKind::Simple,
                            MoveKind::PawnDoubleMove,
                            MoveKind::Enpassant,
                            MoveKind::Promote,
                            MoveKind::CastlingKingside,
                            MoveKind::CastlingQueenside};
  const cell_t promotes[] = {EMPTY_CELL, makeCell(b.side, Piece::Knight),
                             makeCell(b.side, Piece::Bishop), makeCell(b.side, Piece::Rook),
                             makeCell(b.side, Piece::Queen)};
  for (MoveKind kind : kinds) {
    for (cell_t promote : promotes) {
      if ((promote != EMPTY_CELL) != (kind == MoveKind::Promote)) {
        continue;
      }
      for (coord_t src = 0; src < 64; ++src) {
        for (coord_t dst = 0; dst < 64; ++dst) {
          const Move move{kind, src, dst, promote};
          if (!move.isWellFormed(b.side)) {
            continue;
          }
          if (isMoveValid(b, move)) {
            validMoves.push_back(move);
          }
        }
      }
    }
  }
  std::sort(validMoves.begin(), validMoves.end(), cmpMoves);
  if (!std::equal(validMoves.begin(), validMoves.end(), moves, moves + moveCnt)) {
    panic("Valid move list and generated move list mismatch");
  }

  for (size_t i = 0; i < moveCnt; ++i) {
    const Move move = moves[i];
    const Board saved = b;
    MovePersistence p = moveMake(b, move);
    if (isMoveLegal(b)) {
      testBoardValid(b);
    }
    moveUnmake(b, move, p);
    if (!boardsBitCompare(b, saved)) {
      panic("Board becomes different after making and unmaking move \"" + moveToStr(move) + "\"");
    }
  }
}

}  // namespace SoFCore::Test
