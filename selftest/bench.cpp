#include <benchmark/benchmark.h>

#include <algorithm>

#include "chess_intf.h"
#include "core/bench_boards.h"
#include "util.h"

inline void runGenMoves(benchmark::State &state, const char *fen) {
  using namespace ChessIntf;

  init();

  Board board = boardFromFen(fen);
  for ([[maybe_unused]] auto _ : state) {
    MoveList moves = generateMoves(board);
    benchmark::DoNotOptimize(moves);
  }
}

#define BENCH_DO(name)                                                                        \
  static void BM_GenMoves##name(benchmark::State &state) { runGenMoves(state, g_fen##name); } \
  BENCHMARK(BM_GenMoves##name);
#include "core/bench_xmacro.h"
#undef BENCH_DO

inline void runMakeMove(benchmark::State &state, const char *fen) {
  using namespace ChessIntf;

  init();

  Board board = boardFromFen(fen);
  MoveList moveList = generateMoves(board);

  Move moves[MOVES_MAX];
  int cnt = getMoveCount(moveList);
  for (int i = 0; i < cnt; ++i) {
    moves[i] = getMove(moveList, i);
  }
  std::sort(moves, moves + cnt, [&](const Move &a, const Move &b) {
    return getMoveHash(board, a) < getMoveHash(board, b);
  });

  for ([[maybe_unused]] auto _ : state) {
    for (int i = 0; i < cnt; ++i) {
      MovePersistence p = makeMove(board, moves[i]);
      unmakeMove(board, moves[i], p);
    }
  }
}

#define BENCH_DO(name)                                                                        \
  static void BM_MakeMove##name(benchmark::State &state) { runMakeMove(state, g_fen##name); } \
  BENCHMARK(BM_MakeMove##name);
#include "core/bench_xmacro.h"
#undef BENCH_DO

inline void runIsAttacked(benchmark::State &state, const char *fen) {
  using namespace ChessIntf;

  init();

  Board board = boardFromFen(fen);

  for ([[maybe_unused]] auto _ : state) {
    for (bool color : {true, false}) {
      for (char y = '8'; y >= '1'; --y) {
        for (char x = 'a'; x <= 'h'; ++x) {
          benchmark::DoNotOptimize(isAttacked(board, color, x, y));
        }
      }
    }
  }
}

#define BENCH_DO(name)                                                                            \
  static void BM_IsAttacked##name(benchmark::State &state) { runIsAttacked(state, g_fen##name); } \
  BENCHMARK(BM_IsAttacked##name);
#include "core/bench_xmacro.h"
#undef BENCH_DO

inline void runKingAttack(benchmark::State &state, const char *fen) {
  using namespace ChessIntf;

  init();

  Board board = boardFromFen(fen);

  for ([[maybe_unused]] auto _ : state) {
    benchmark::DoNotOptimize(isOpponentKingAttacked(board));
  }
}

#define BENCH_DO(name)                                                                            \
  static void BM_KingAttack##name(benchmark::State &state) { runKingAttack(state, g_fen##name); } \
  BENCHMARK(BM_KingAttack##name);
#include "core/bench_xmacro.h"
#undef BENCH_DO

void recurseSearch(ChessIntf::Board &board, int d) {
  using namespace ChessIntf;

  if (d == 0) {
    return;
  }

  MoveList moves = generateMoves(board);
  int cnt = getMoveCount(moves);

  for (int i = 0; i < cnt; ++i) {
    const Move &move = getMove(moves, i);
    MovePersistence persistence = makeMove(board, move);
    if (!isOpponentKingAttacked(board)) {
      recurseSearch(board, d - 1);
    }
    unmakeMove(board, move, persistence);
  }
}

inline void runRecurse(benchmark::State &state, const char *fen) {
  using namespace ChessIntf;

  init();

  Board board = boardFromFen(fen);

  for ([[maybe_unused]] auto _ : state) {
    int d = state.range(0);
    recurseSearch(board, d);
  }
}

#define BENCH_DO(name)                                                                      \
  static void BM_Recurse##name(benchmark::State &state) { runRecurse(state, g_fen##name); } \
  BENCHMARK(BM_Recurse##name)->Arg(1)->Arg(2)->Arg(3)->Arg(4)->Unit(benchmark::kMillisecond);
#include "core/bench_xmacro.h"
#undef BENCH_DO
