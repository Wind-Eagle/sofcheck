#include <iomanip>
#include <iostream>
#include <vector>

#include "common.h"
#include "core/types.h"

using namespace SoFCore;

std::vector<bitboard_t> generateDirected(const int8_t offX[], const int8_t offY[],
                                         const size_t size) {
  std::vector<bitboard_t> results(64);

  for (coord_t c = 0; c < 64; ++c) {
    bitboard_t bb = 0;
    const subcoord_t x = coordX(c);
    const subcoord_t y = coordY(c);
    for (size_t direction = 0; direction < size; ++direction) {
      const subcoord_t nx = x + static_cast<subcoord_t>(offX[direction]);
      const subcoord_t ny = y + static_cast<subcoord_t>(offY[direction]);
      if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
        bb |= coordToBitboard(makeCoord(nx, ny));
      }
    }
    results[c] = bb;
  }

  return results;
}

template<Color C>
std::vector<bitboard_t> generateBackwardPawn() {
  std::vector<bitboard_t> result;
  for (subcoord_t i=0; i<8; i++)
  {
    for (subcoord_t j=0; j<8; j++)
    {
      bitboard_t cur=0;
      for (subcoord_t x=0; x<8; x++)
      {
        for (subcoord_t y=0; y<8; y++)
        {
            if (C==Color::White)
            {
                if ((y==j-1||y==j+1)&&x>i) cur^=coordToBitboard(makeCoord(x, y));
            }
            else
            {
                if ((y==j-1||y==j+1)&&x<i) cur^=coordToBitboard(makeCoord(x, y));
            }
        }
      }
      result.push_back(cur);
    }
  }
  return result;
}

template<Color C>
std::vector<bitboard_t> generateBackwardPawnSentries() {
  std::vector<bitboard_t> result;
  for (subcoord_t i=0; i<8; i++)
  {
    for (subcoord_t j=0; j<8; j++)
    {
        bitboard_t cur=0;
        if (C==Color::White&&i>=2)
        {
            if (j>=1) cur^=coordToBitboard(makeCoord(i-2, j-1));
            if (j<=6) cur^=coordToBitboard(makeCoord(i-2, j+1));
        }
        if (C==Color::Black&&i<=5)
        {
            if (j>=1) cur^=coordToBitboard(makeCoord(i+2, j-1));
            if (j<=6) cur^=coordToBitboard(makeCoord(i+2, j+1));
        }
      result.push_back(cur);
    }
  }
  return result;
}

template<Color C>
std::vector<bitboard_t> generatePass() {
  std::vector<bitboard_t> result;
  for (subcoord_t i=0; i<8; i++)
  {
    for (subcoord_t j=0; j<8; j++)
    {
      bitboard_t cur=0;
      for (subcoord_t x=0; x<8; x++)
      {
        for (subcoord_t y=0; y<8; y++)
        {
            if (C==Color::White)
            {
                if ((y==j-1||y==j+1)&&x<i) cur^=coordToBitboard(makeCoord(x, y));
            }
            else
            {
                if ((y==j-1||y==j+1)&&x>i) cur^=coordToBitboard(makeCoord(x, y));
            }
        }
      }
      result.push_back(cur);
    }
  }
  return result;
}

template<Color C>
std::vector<bitboard_t> generatePassDef() {
  std::vector<bitboard_t> result;
  for (subcoord_t i=0; i<8; i++)
  {
    for (subcoord_t j=0; j<8; j++)
    {
      bitboard_t cur=0;
      for (subcoord_t x=0; x<8; x++)
      {
        for (subcoord_t y=0; y<8; y++)
        {
            if (C==Color::White)
            {
                if ((y==j-1||y==j+1)&&(x==i+1||x==i)) cur^=coordToBitboard(makeCoord(x, y));
            }
            else
            {
                if ((y==j-1||y==j+1)&&((x==i-1||x==i))) cur^=coordToBitboard(makeCoord(x, y));
            }
        }
      }
      result.push_back(cur);
    }
  }
  return result;
}

template<Color C>
std::vector<bitboard_t> generateVertical() {
  std::vector<bitboard_t> result;
  for (subcoord_t i=0; i<8; i++)
  {
    for (subcoord_t j=0; j<8; j++)
    {
      bitboard_t cur=0;
      for (subcoord_t x=0; x<8; x++)
      {
        for (subcoord_t y=0; y<8; y++)
        {
            if (C==Color::White&&x>=i) continue;
            if (C==Color::Black&&x<=i) continue;
            if (y==j) cur^=coordToBitboard(makeCoord(x, y));
        }
      }
      result.push_back(cur);
    }
  }
  return result;
}

std::vector<bitboard_t> generateHorisontal() {
  std::vector<bitboard_t> result;
  for (subcoord_t i=0; i<8; i++)
  {
    for (subcoord_t j=0; j<8; j++)
    {
      bitboard_t cur=0;
      for (subcoord_t x=0; x<8; x++)
      {
        for (subcoord_t y=0; y<8; y++)
        {
            if (x==i) cur^=coordToBitboard(makeCoord(x, y));
        }
      }
      result.push_back(cur);
    }
  }
  return result;
}

std::vector<bitboard_t> generateIsolated() {
  std::vector<bitboard_t> result;
  for (subcoord_t i=0; i<8; i++)
  {
    for (subcoord_t j=0; j<8; j++)
    {
      bitboard_t cur=0;
      for (subcoord_t x=0; x<8; x++)
      {
        for (subcoord_t y=0; y<8; y++)
        {
            if ((y==j-1||y==j+1)) cur^=coordToBitboard(makeCoord(x, y));
        }
      }
      result.push_back(cur);
    }
  }
  return result;
}

template<Color C>
std::vector<bitboard_t> generateHalfOpen() {
  std::vector<bitboard_t> result;
  for (subcoord_t i=0; i<8; i++)
  {
    for (subcoord_t j=0; j<8; j++)
    {
      bitboard_t cur=0;
      for (subcoord_t x=0; x<8; x++)
      {
        for (subcoord_t y=0; y<8; y++)
        {
            if (C==Color::White)
            {
                if (y==j && x>=i) cur^=coordToBitboard(makeCoord(x, y));
            }
            else
            {
                if (y==j && x<=j) cur^=coordToBitboard(makeCoord(x, y));
            }
        }
      }
      result.push_back(cur);
    }
  }
  return result;
}

void doGenerate(std::ostream &out) {
  out << "#ifndef SOF_SEARCH_PRIVATE_EVAL_BITBOARDS_INCLUDED\n";
  out << "#define SOF_SEARCH_PRIVATE_EVAL_BITBOARDS_INCLUDED\n";
  out << "\n";
  out << "#include \"core/types.h\"\n";
  out << "\n";
  out << "namespace SoFSearch::Private {\n";
  out << "\n";

  auto whiteBackwardPawn = generateBackwardPawn<Color::White>();
  auto blackBackwardPawn = generateBackwardPawn<Color::Black>();
  auto whiteBackwardPawnSentry = generateBackwardPawnSentries<Color::White>();
  auto blackBackwardPawnSentry = generateBackwardPawnSentries<Color::Black>();
  auto whitePass = generatePass<Color::White>();
  auto blackPass = generatePass<Color::Black>();
  auto whitePassDef = generatePassDef<Color::White>();
  auto blackPassDef = generatePassDef<Color::Black>();
  auto whiteVertical = generateVertical<Color::White>();
  auto blackVertical = generateVertical<Color::Black>();
  auto Horisontal = generateHorisontal();
  auto Isolated = generateIsolated();
  auto whiteHalfOpen = generateVertical<Color::White>();
  auto blackHalfOpen = generateVertical<Color::Black>();

  printBitboardArray(out, whiteBackwardPawn, "WHITE_BACKWARD_PAWN");
  printBitboardArray(out, blackBackwardPawn, "BLACK_BACKWARD_PAWN");
  printBitboardArray(out, whiteBackwardPawnSentry, "WHITE_BACKWARD_PAWN_SENTRY");
  printBitboardArray(out, blackBackwardPawnSentry, "BLACK_BACKWARD_PAWN_SENTRY");
  printBitboardArray(out, whitePass, "WHITE_PASS");
  printBitboardArray(out, blackPass, "BLACK_PASS");
  printBitboardArray(out, whitePassDef, "WHITE_PASS_DEF");
  printBitboardArray(out, blackPassDef, "BLACK_PASS_DEF");
  printBitboardArray(out, whiteVertical, "WHITE_VERTICAL");
  printBitboardArray(out, blackVertical, "BLACK_VERTICAL");
  printBitboardArray(out, Horisontal, "HORISONTAL");
  printBitboardArray(out, Isolated, "ISOLATED");
  printBitboardArray(out, whiteHalfOpen, "WHITE_HALF_OPEN");
  printBitboardArray(out, blackHalfOpen, "BLACK_HALF_OPEN");
  out << "\n";

  out << "}  // namespace SoFSearch::Private\n";
  out << "\n";

  out << "#endif  // SOF_SEARCH_PRIVATE_EVAL_BITBOARDS_INCLUDED\n";
}
