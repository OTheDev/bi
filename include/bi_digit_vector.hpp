/*
Copyright 2024 Owain Davies
SPDX-License-Identifier: Apache-2.0
*/

#ifndef BI_INCLUDE_BI_DIGIT_VECTOR_HPP_
#define BI_INCLUDE_BI_DIGIT_VECTOR_HPP_

#include <algorithm>
#include <cassert>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <utility>

#include "bi_config.hpp"
#include "bi_exceptions.hpp"

namespace bi {

using bi_config::bi_max_digits;
using bi_config::digit;
using bi_config::range_check;

template <typename T>
concept DigitIterator =
    std::input_iterator<T> &&
    std::convertible_to<typename std::iterator_traits<T>::value_type, digit>;

/**
 *  @brief Digit vector class.
 *  @note Not to be confused with `std::vector`. The semantics of some
 *  operations differ.
 */
class digit_vector {
 public:
  using iterator = digit*;
  using const_iterator = const digit*;
  using riterator = std::reverse_iterator<digit*>;
  using const_riterator = std::reverse_iterator<const digit*>;

  ~digit_vector() noexcept = default;

  // Default constructor
  digit_vector() noexcept = default;

  // Copy constructor
  digit_vector(const digit_vector& other)
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
      : digits_(std::make_unique_for_overwrite<digit[]>(other.capacity_)),
        size_(other.size_), capacity_(other.capacity_) {
    std::copy(other.digits_.get(), other.digits_.get() + size_, digits_.get());
  }

  // Move constructor
  digit_vector(digit_vector&& other) noexcept
      : digits_(std::move(other.digits_)), size_(other.size_),
        capacity_(other.capacity_) {
    other.size_ = 0;
    other.capacity_ = 0;
  }

  // Copy assignment operator
  digit_vector& operator=(const digit_vector& other) {
    if (this != &other) {
      digit_vector temp(other);
      std::swap(digits_, temp.digits_);
      std::swap(size_, temp.size_);
      std::swap(capacity_, temp.capacity_);
    }
    return *this;
  }

  // Move assignment operator
  digit_vector& operator=(digit_vector&& other) noexcept {
    if (this != &other) {
      digits_ = std::move(other.digits_);
      size_ = other.size_;
      capacity_ = other.capacity_;
      other.size_ = 0;
      other.capacity_ = 0;
    }
    return *this;
  }

  template <typename DigitIterator>
  digit_vector(DigitIterator first, DigitIterator last) {
    auto distance = std::distance(first, last);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
    digits_ = std::make_unique_for_overwrite<digit[]>(distance);
    std::copy(first, last, digits_.get());
    size_ = distance;
    capacity_ = distance;
  }

  size_t size() const noexcept { return size_; }
  size_t capacity() const noexcept { return capacity_; }
  digit* data() noexcept { return digits_.get(); }
  const digit* data() const noexcept { return digits_.get(); }
  size_t max_size() const noexcept { return bi_max_digits; }

  digit& operator[](size_t index) {
    if constexpr (range_check) {
      if (index >= size_) {
        throw std::out_of_range("Index out of range");
      }
    }
    return digits_[index];
  }

  const digit& operator[](size_t index) const {
    if constexpr (range_check) {
      if (index >= size_) {
        throw std::out_of_range("Index out of range");
      }
    }
    return digits_[index];
  }

  void resize(size_t new_size) {
    if (new_size > capacity_) {
      reserve(new_size);
    }
    size_ = new_size;
  }

  void reserve(size_t new_capacity) {
    if (new_capacity > max_size()) {
      throw overflow_error(
          "Requested capacity exceeds maximum allowable size.");
    }

    if (new_capacity > capacity_) {
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
      auto new_digits = std::make_unique_for_overwrite<digit[]>(new_capacity);
      std::copy(digits_.get(), digits_.get() + size_, new_digits.get());
      digits_ = std::move(new_digits);
      capacity_ = new_capacity;
    }
  }

  void push_back(const digit& value) {
    if (size_ >= capacity_) {
      reserve(capacity_ + 1);
    }
    digits_[size_] = value;
    ++size_;
  }

  // Iterators
  iterator begin() noexcept { return digits_.get(); }
  iterator end() noexcept { return digits_.get() + size_; }
  const_iterator begin() const noexcept { return digits_.get(); }
  const_iterator end() const noexcept { return digits_.get() + size_; }

  riterator rbegin() noexcept { return riterator(end()); }
  riterator rend() noexcept { return riterator(begin()); }
  const_riterator rbegin() const noexcept { return const_riterator(end()); }
  const_riterator rend() const noexcept { return const_riterator(begin()); }

  // Deviations
  /// Sets the size of the vector. Use only if `new_size <= capacity()`
  void resize_unsafe(size_t new_size) {
    assert(new_size <= capacity_);
    size_ = new_size;
  }

 private:
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
  std::unique_ptr<digit[]> digits_;
  size_t size_{0};
  size_t capacity_{0};
};

};  // namespace bi

#endif  // BI_INCLUDE_BI_DIGIT_VECTOR_HPP_
