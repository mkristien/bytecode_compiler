#include <stack>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <bit>

extern "C" {

/**
 * Anonymous namespace provide implementation details of runtime data and functions.
 *
 * In particular, the actual stack for Words is declared here.
 */
namespace {
constexpr uint16_t kWordSize = 32;

struct Word {
  Word(const uint8_t* w) {
    // flip endianness
    for (int i = 0; i < kWordSize; i++) {
      data.B[kWordSize - 1 - i] = w[i];
    }
  }
  union {
    uint8_t  B[kWordSize / sizeof(uint8_t)];
    uint16_t S[kWordSize / sizeof(uint16_t)];
    uint32_t W[kWordSize / sizeof(uint32_t)];
    uint64_t D[kWordSize / sizeof(uint64_t)];
  } data;

  void print() {
    // uncomment print statements for a debug runtime

    // for (int i = kWordSize - 1; i >= 0; i--) {
    //   printf("%x", data.B[i]);
    // }
    // printf("\n");
  }
};

std::stack<Word> s;

/**
 * Add two Words in 32-bit chunks, from the least significant chunk.
 *
 * For each chunk, perform a three-chunk addition:
 * C = A + B + carry
 *
 * Result chunks are trimmed to 32-bits, any overflow becomes carry for the next
 * chunk iteration. To detect overflow, 32-bit chunks are promoted to uint64 in
 * each chunk iteration.
 */
void DoAdd(Word& a, Word& b, Word* result) {
  a.print();
  b.print();
  uint64_t carry = 0;
  for (int i = 0; i < kWordSize / sizeof(uint32_t); i++) {
    uint64_t sum = carry + a.data.W[i] + b.data.W[i];
    result->data.W[i] = sum;
    carry = sum >> 32;
  }
  result->print();
}

/**
 * Sub two Words, A - B, in 32-bit chunks, from the least significant chunk.
 * Rely on two-s complement represenation to reuse a design for addition as:
 * C = A + ~B + 1;
 */
void DoSub(Word& a, Word& b, Word* result) {
  a.print();
  b.print();
  uint64_t carry = 1;
  for (int i = 0; i < kWordSize / sizeof(uint32_t); i++) {
    uint64_t sum = carry + a.data.W[i] + ~b.data.W[i];
    result->data.W[i] = sum;
    carry = sum >> 32;
  }
  result->print();
}

}   // namespace

////////////////////////////////////////////////////////////////////////////////
// Runtime functions
////////////////////////////////////////////////////////////////////////////////

/**
 * @param out - pointer to the data where the top Word is stored
 * @note: array offset considered by the IR before this function is called
 */
void store(uint8_t* out) {
  // flip endianness
  uint8_t* data = s.top().data.B;
  for (int i = 0; i < kWordSize; i++) {
    out[i] = data[kWordSize - 1 - i];
  }
}

/**
 * @param in - pointer to the data from which a Word is pushed
 * @note: array offset considered by the IR before this function is called
 */
void load(const uint8_t* in) {
  s.push(in);
}

void pop() {
  s.pop();
}

/**
 * C = A + B
 * (1) A: copy a Word from the top of the stack, pop the stack
 * (2) B: pass a reference to the top of the stack Word
 * (3) C: pass a pointer to the top of the stack Word (this reuses the memory of B)
 */
void add() {
  Word a = s.top();
  s.pop();
  DoAdd(a, s.top(), &s.top());
}

/**
 * C = A - B
 * (1) A: copy a Word from the top of the stack, pop the stack
 * (2) B: pass a reference to the top of the stack Word
 * (3) C: pass a pointer to the top of the stack Word (this reuses the memory of B)
 */
void sub() {
  Word a = s.top();
  s.pop();
  DoSub(a, s.top(), &s.top());
}

void dup() {
  s.push(s.top());
}

}
