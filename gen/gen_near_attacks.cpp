#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include "../src/types.h"

using namespace SoFCore;

void printBitboard(std::ostream &out, bitboard_t val) {
  out << "0x" << std::hex << std::setw(16) << std::setfill('0') << val
      << std::dec << std::setfill(' ');
}

void printBitboardArray(std::ostream &out, const std::vector<bitboard_t> &array,
                        const char *name) {
  out << "SoFCore::bitboard_t " << name << "[" << std::dec << array.size()
      << "] = {\n";
  for (size_t i = 0; i < array.size(); ++i) {
    out << "  /*" << std::setw(2) << i << "*/ ";
    printBitboard(out, array[i]);
    if (i + 1 != array.size()) {
      out << ",";
    }
    out << "\n";
  }
  out << "};\n";
}

std::vector<bitboard_t> generateDirected(const int8_t offX[],
                                         const int8_t offY[], size_t size) {
  std::vector<bitboard_t> results(64);

  for (cell_t cell = 0; cell < 64; ++cell) {
    bitboard_t bb = 0;
    uint8_t x = coordX(cell);
    uint8_t y = coordY(cell);
    for (size_t direction = 0; direction < size; ++direction) {
      uint8_t nx = x + static_cast<uint8_t>(offX[direction]);
      uint8_t ny = y + static_cast<uint8_t>(offY[direction]);
      if (nx < 8 && ny < 8) {
        bb |= coordToBitboard(makeCoord(nx, ny));
      }
    }
    results[cell] = bb;
  }

  return results;
}

std::vector<bitboard_t> generateKnightAttacks() {
  const int8_t offX[] = {-2, -2, -1, -1, 2, 2, 1, 1};
  const int8_t offY[] = {-1, 1, -2, 2, -1, 1, -2, 2};
  return generateDirected(offX, offY, 8);
}

std::vector<bitboard_t> generateKingAttacks() {
  const int8_t offX[] = {-1, -1, -1, 0, 0, 1, 1, 1};
  const int8_t offY[] = {-1, 0, 1, -1, 1, -1, 0, 1};
  return generateDirected(offX, offY, 8);
}

std::vector<bitboard_t> generateWhitePawnAttacks() {
  const int8_t offX[] = {-1, -1};
  const int8_t offY[] = {-1, 1};
  return generateDirected(offX, offY, 2);
}

std::vector<bitboard_t> generateBlackPawnAttacks() {
  const int8_t offX[] = {1, 1};
  const int8_t offY[] = {-1, 1};
  return generateDirected(offX, offY, 2);
}

void printCombinedMatrix(std::ostream &out, const std::vector<uint64_t> &pawn,
                         const std::vector<uint64_t> &knight,
                         const std::vector<uint64_t> &king, const char *name) {
  out << "SoFCore::bitboard_t " << name << "[64][4] = {\n";
  for (size_t i = 0; i < 64; ++i) {
    out << "  /*" << std::setw(2) << i << "*/ {0x0, ";
    printBitboard(out, pawn[i]);
    out << ",";
    printBitboard(out, knight[i]);
    out << ",";
    printBitboard(out, king[i]);
    out << "}";
    if (i + 1 != 64) {
      out << ",";
    }
    out << "\n";
  }
  out << "};\n";
}

void doGenerate(std::ostream &out) {
  out << "#ifndef NEAR_ATTACKS_INCLUDED\n";
  out << "#define NEAR_ATTACKS_INCLUDED\n";
  out << "\n";
  out << "#include \"types.h\"\n";
  out << "\n";

  auto knightAttacks = generateKingAttacks();
  auto kingAttacks = generateKingAttacks();
  auto whitePawnAttacks = generateWhitePawnAttacks();
  auto blackPawnAttacks = generateBlackPawnAttacks();

  printBitboardArray(out, knightAttacks, "KNIGHT_ATTACKS");
  out << "\n";
  printBitboardArray(out, kingAttacks, "KING_ATTACKS");
  out << "\n";
  printBitboardArray(out, whitePawnAttacks, "WHITE_PAWN_ATTACKS");
  out << "\n";
  printBitboardArray(out, blackPawnAttacks, "BLACK_PAWN_ATTACKS");
  out << "\n";

  printCombinedMatrix(out, whitePawnAttacks, knightAttacks, kingAttacks,
                      "WHITE_ATTACK_MATRIX");
  out << "\n";
  printCombinedMatrix(out, blackPawnAttacks, knightAttacks, kingAttacks,
                      "BLACK_ATTACK_MATRIX");
  out << "\n";

  out << "#endif // NEAR_ATTACKS_INCLUDED\n";
}

int main(int argc, char **argv) {
  if (argc == 1) {
    doGenerate(std::cout);
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "-h") != 0 &&
      strcmp(argv[1], "--help") != 0) {
    std::ofstream file(argv[1]);
    doGenerate(file);
    return 0;
  }
  std::cerr << "usage: " << argv[0] << " OUT_FILE" << std::endl;
  return 0;
}
