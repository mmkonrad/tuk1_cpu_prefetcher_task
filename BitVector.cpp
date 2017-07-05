#include <cstdint>
#include <vector>
#include <cmath>
#include <iostream>
#include <stdio.h>
#include <string>
#include <chrono>
#include <functional>
#include <iomanip>
#include <unistd.h>
#include <random>
#include <algorithm>
#include <bitset>

// get AVX intrinsics
#include <immintrin.h>

using namespace std;
using Time = std::chrono::nanoseconds;

const __m256i selection_mask_64 = _mm256_set_epi64x(-1, -1, -1, -1);
const __m256i selection_mask = _mm256_set_epi32(-1, -1, -1, -1, -1, -1, -1, -1);
const __m256i selection_order_first = _mm256_setr_epi32(0, 1, 0, 1, 2, 3, 2, 3);
const __m256i selection_order_second = _mm256_setr_epi32(1, 2, 1, 2, 3, 4, 3, 4);

template<unsigned int Bits, typename T = uint64_t>
class BitVector {
public:
    BitVector(size_t size) : _size(size), _first_unused(0), _data(ceil(1.0 * size * Bits / row_bits)) {
      int bit_position = 0;
      for (size_t i = 0; i < 4; ++i) {
        long long int current_bit_shifts_first[4];
        long long int current_bit_shifts_second[4];
        for (size_t j = 0; j < 4; j+=2) {
          current_bit_shifts_first[j] = bit_position;
          bit_position += Bits;
          bit_position %= 64;
          current_bit_shifts_first[j+1] = bit_position;
          bit_position += Bits+32;
          bit_position %= 64;
          current_bit_shifts_second[j] = bit_position;
          bit_position += Bits;
          bit_position %= 64;
          current_bit_shifts_second[j+1] = bit_position;
          bit_position += Bits+32;
          bit_position %= 64;
        }
        bit_shifts_first[i] = _mm256_maskload_epi64(current_bit_shifts_first, selection_mask_64);
        bit_shifts_second[i] = _mm256_maskload_epi64(current_bit_shifts_second, selection_mask_64);
      }
    }

    void push_back(uint64_t value) {
      if (Bits < row_bits) {
        value &= (1ul << Bits) - 1;
      }
      const size_t bit_index = _first_unused * Bits;
      const size_t first_ind = bit_index / row_bits;
      const size_t first_bit = bit_index % row_bits;
      _data[first_ind] |= value << first_bit;
      const size_t inverted_last_bit = row_bits - first_bit;
      if (inverted_last_bit < Bits) {
        _data[first_ind + 1] = value >> inverted_last_bit;
      }
      ++_first_unused;
    }

    uint64_t operator[](size_t pos) const {
      const size_t bit_index = pos * Bits;
      const size_t first_ind = bit_index / row_bits;
      const size_t first_bit = bit_index % row_bits;
      const int bits_left = row_bits - first_bit - Bits;
      if (bits_left >= 0) {
        return (_data[first_ind] << bits_left) >> (row_bits - Bits);
      }
      uint64_t value = _data[first_ind] >> first_bit;
      value |= (_data[first_ind + 1] << (row_bits + bits_left)) >> (row_bits - Bits);
      return value;
    }

    /**
     * Only tested with Bits = 17
     */
    std::vector<bool> avx2_search(uint64_t value) const {
      std::vector<bool> found(_size);
      auto itr = found.begin();
      int * data_ptr = (int*) _data.data();
      int bitshift_index = 0;
      for (size_t i = 0; i < _size;) {
        const __m256i bit_mask = _mm256_set1_epi64x((1l << Bits)-1);
        const __m256i input_values = _mm256_maskload_epi64((long long int*) data_ptr, selection_mask_64);
        __m256i values_first = _mm256_permutevar8x32_epi32(input_values, selection_order_first);
        values_first = _mm256_srlv_epi64(values_first, bit_shifts_first[bitshift_index]);
        values_first = _mm256_and_si256(values_first, bit_mask);
        __m256i values_second = _mm256_permutevar8x32_epi32(input_values, selection_order_second);
        values_second = _mm256_srlv_epi64(values_second, bit_shifts_second[bitshift_index]);
        values_second = _mm256_and_si256(values_second, bit_mask);
        __m256i values = _mm256_hadd_epi32(values_first, values_second);
        __m256i search_value = _mm256_set1_epi32(value);
        values = _mm256_cmpeq_epi32(values, search_value);
        int* result = (int*) &values;
        for (size_t j = 0; j < 8 && i < _size; ++j, ++i) {
          *itr++ = static_cast<bool>(*result++);
        }
        data_ptr += 4;
        ++bitshift_index;
        if (bitshift_index == 4) {
          ++data_ptr;
        }
        bitshift_index %= 4;
      }
      return found;
    }

    std::vector<bool> search(uint64_t value) const {
      std::vector<bool> found(_size);
      auto itr = found.begin();
      for (size_t i = 0; i < _first_unused; ++i) {
        *itr = this->operator[](i) == value;
        ++itr;
      }
      return found;
    }

    size_t size() const { return _size; }

    friend ostream& operator<<(ostream& os, BitVector &v) {
      for (auto & field : v._data) {
        bitset<row_bits> bs(field);
        os << bs << endl;
      }
      return os;
    }

protected:
    constexpr static size_t row_bits = sizeof(T) * 8;
    const size_t _size;
    size_t _first_unused;
    std::vector<T> _data;
    __m256i bit_shifts_first[4];
    __m256i bit_shifts_second[4];

    static_assert(Bits <= row_bits, "Bits > row_bits");
};

void test_avx2() {
  cout << "Testing AVX2 Code ..." << endl;
  for (size_t vector_size = 1; vector_size < 100; ++vector_size) { // 2000
    if (vector_size % 100 == 0) {
      cout << "With size: " << vector_size << endl;
    }
    BitVector<17> bit_vector(vector_size);
    for (size_t i = 0; i < bit_vector.size(); ++i) {
      bit_vector.push_back(i);
    }
    for (size_t i = 0; i < bit_vector.size(); ++i) {
      auto found = bit_vector.avx2_search(i);
      for (size_t j = 0; j < bit_vector.size(); ++j) {
        if (i == j) {
          if (found[j] == false) {
            cout << "found[j] == false" << endl;
          }
        } else {
          if (found[j] == true) {
            cout << "found[j] == true" << endl;
          }
        }
      }
    }
  }
  cout << "... finished" << endl;
}

double measureTime(std::function<void()> benchmark)
{

  // warmup
  for (size_t i = 0; i < 3; ++i) {
    benchmark();
  }

  size_t runs = 10;
  const auto begin = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < runs; ++i) {
    benchmark();
  }

  const auto end = std::chrono::high_resolution_clock::now();
  Time total_time = std::chrono::duration_cast<Time>(end - begin);
  return static_cast<double>(total_time.count()) / (runs * 1000000);
}

int main(int argc, char** argv)
{
//  test_avx2();
  size_t size = 500000000;
  size_t value = 1;
  BitVector<17> bit_vector(size);
  std::vector<int> vect;
  std::vector<int> rand_access;
  vect.reserve(size);
  rand_access.reserve(size);
  for (size_t i = 0; i < size; ++i) {
    int tmp_val = i % (1 << 17);
    bit_vector.push_back(tmp_val);
    vect.push_back(tmp_val);
    rand_access.push_back(i);
  }
  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(rand_access.begin(), rand_access.end(), g);
  cout << std::fixed << std::setprecision(6);
  cout << "Created BitVector and Vector. Starting Benchmarks." << endl;
  cout << "Search" << endl;
  std::vector<bool> search_result;
  auto result1 = measureTime([&](){
      search_result = bit_vector.avx2_search(value);
  });
  cout << "AVX2 " << result1 << " ms" << endl;

  auto result2 = measureTime([&](){
      search_result = bit_vector.search(value);
  });
  cout << "BitVector " << result2 << " ms" <<  endl;
  auto result3 = measureTime([&](){
      std::vector<bool> result;
      result.reserve(size);
      for (auto itr = vect.begin(); itr != vect.end(); ++itr) {
        result.emplace_back(*itr == value);
      }
      search_result = result;
  });
  cout << "Vector " << result3 << " ms" <<  endl;
  cout << "Random access" << endl;
  int64_t accessed_value;
  auto result4 = measureTime([&](){
      int64_t current_value = 0;
      for (auto itr = rand_access.begin(); itr != rand_access.end(); ++itr) {
        current_value ^= bit_vector[*itr];
      }
      accessed_value = current_value;
  });
  cout << "BitVector " << result4 << " ms" <<  endl;
  auto result5 = measureTime([&](){
      int64_t current_value = 0;
      for (auto itr = rand_access.begin(); itr != rand_access.end(); ++itr) {
        current_value ^= vect[*itr];
      }
      accessed_value = current_value;
  });
  cout << "Vector " << result5 << " ms" <<  endl;
}