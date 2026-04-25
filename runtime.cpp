#include <stack>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <bit>

extern "C" {

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
    for (int i = kWordSize - 1; i >= 0; i--) {
      printf("%x", data.B[i]);
    }
    printf("\n");
  }
};

std::stack<Word> s;

void DoAdd(Word& a, Word& b, Word* result) {
  a.print();
  b.print();
  uint64_t carry = 0;
  for (int i = 0; i < kWordSize / sizeof(uint32_t); i++) {
    uint32_t a_val = a.data.W[i];
    uint32_t b_val = b.data.W[i];
    uint64_t sum = carry + a_val + b_val;
    printf("suming 0x%x + 0x%x = 0x%lx\n", a_val, b_val, sum);
    result->data.W[i] = sum;
    carry = sum >> 32;
  }
  result->print();
}

void DoSub(Word& a, Word& b, Word* result) {
  uint64_t carry = 1;
  for (int i = 0; i < kWordSize / sizeof(uint32_t); i++) {
    uint64_t sum = carry + a.data.W[i] + ~b.data.W[i];
    result->data.W[i] = sum;
    carry = sum >> 32;
  }
}

}   // namespace


void store(uint8_t* out) {
  // flip endianness
  uint8_t* data = s.top().data.B;
  for (int i = 0; i < kWordSize; i++) {
    out[i] = data[kWordSize - 1 - i];
  }
}

void load(const uint8_t* in) {
  s.push(in);
}

void pop() {
  s.pop();
}

void add() {
  Word a = s.top();
  s.pop();
  DoAdd(a, s.top(), &s.top());
}

void sub() {
  Word a = s.top();
  s.pop();
  DoSub(a, s.top(), &s.top());
}

void dup() {
  s.push(s.top());
}

}
