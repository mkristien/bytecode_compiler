#include <cstddef>
#include <cstdint>

extern "C" {

/**
 * Anonymous namespace provide implementation details of runtime data and functions.
 *
 * In particular, the actual stack for Words is declared here.
 */
namespace {
constexpr uint16_t kWordSize = 32;

struct Word {
  Word() = default;

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

class WordStack {
 public:
  ~WordStack() { delete[] storage_; }

  void push(const uint8_t* w) {
    ensure_capacity();
    storage_[size_++] = Word(w);
  }

  void push(const Word& w) {
    ensure_capacity();
    storage_[size_++] = w;
  }

  Word& top() { return storage_[size_ - 1]; }

  void pop() { --size_; }

 private:
  void ensure_capacity() {
    if (size_ < capacity_) {
      return;
    }
    size_t const new_cap = capacity_ == 0 ? 16 : capacity_ * 2;
    Word* const new_storage = new Word[new_cap];
    for (size_t i = 0; i < size_; ++i) {
      new_storage[i] = storage_[i];
    }
    delete[] storage_;
    storage_  = new_storage;
    capacity_ = new_cap;
  }

  Word*    storage_  = nullptr;
  size_t   size_     = 0;
  size_t   capacity_ = 0;
};

WordStack s;

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
