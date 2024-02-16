/*
Copyright 2024 Owain Davies
SPDX-License-Identifier: Apache-2.0
*/

#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>
#include <memory>
#include <utility>

#include "bi_.hpp"
#include "bi_exceptions.hpp"

namespace bi {

constexpr size_t compute_max_digits() {
  size_t a = std::numeric_limits<size_t>::max() / bi_sizeof_digit;
  bi_bitcount_t b = std::numeric_limits<bi_bitcount_t>::max() / bi_dbits;

  return a < b ? a : static_cast<size_t>(b);
}

constexpr size_t bi_max_digits = compute_max_digits();

template <typename T>
concept DigitIterator =
    std::input_iterator<T> &&
    std::convertible_to<typename std::iterator_traits<T>::value_type, digit>;

bi_t::dvector::~dvector() noexcept = default;

/// Default constructor.
bi_t::dvector::dvector() noexcept = default;

/// Copy constructor.
bi_t::dvector::dvector(const dvector& other)
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
    : digits_(std::make_unique_for_overwrite<digit[]>(other.capacity_)),
      size_(other.size_), capacity_(other.capacity_) {
  std::copy(other.digits_.get(), other.digits_.get() + size_, digits_.get());
}

/// Move constructor.
bi_t::dvector::dvector(dvector&& other) noexcept
    : digits_(std::move(other.digits_)), size_(other.size_),
      capacity_(other.capacity_) {
  other.size_ = 0;
  other.capacity_ = 0;
}

/// Copy assignment operator.
bi_t::dvector& bi_t::dvector::operator=(const dvector& other) {
  if (this != &other) {
    dvector temp(other);
    std::swap(digits_, temp.digits_);
    std::swap(size_, temp.size_);
    std::swap(capacity_, temp.capacity_);
  }
  return *this;
}

/// Move assignment operator.
bi_t::dvector& bi_t::dvector::operator=(dvector&& other) noexcept {
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
bi_t::dvector::dvector(DigitIterator first, DigitIterator last) {
  auto distance = std::distance(first, last);
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
  digits_ = std::make_unique_for_overwrite<digit[]>(distance);
  std::copy(first, last, digits_.get());
  size_ = distance;
  capacity_ = distance;
}

size_t bi_t::dvector::size() const noexcept { return size_; }

size_t bi_t::dvector::capacity() const noexcept { return capacity_; }

digit* bi_t::dvector::data() noexcept { return digits_.get(); }

const digit* bi_t::dvector::data() const noexcept { return digits_.get(); }

size_t bi_t::dvector::max_size() const noexcept { return bi_max_digits; }

digit& bi_t::dvector::operator[](size_t index) { return digits_[index]; }

const digit& bi_t::dvector::operator[](size_t index) const {
  return digits_[index];
}

void bi_t::dvector::resize(size_t new_size) {
  if (new_size > capacity_) {
    reserve(new_size);
  }
  size_ = new_size;
}

void bi_t::dvector::reserve(size_t new_capacity) {
  if (new_capacity > max_size()) {
    throw overflow_error("Requested capacity exceeds maximum allowable size.");
  }

  if (new_capacity > capacity_) {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
    auto new_digits = std::make_unique_for_overwrite<digit[]>(new_capacity);
    std::copy(digits_.get(), digits_.get() + size_, new_digits.get());
    digits_ = std::move(new_digits);
    capacity_ = new_capacity;
  }
}

void bi_t::dvector::push_back(const digit& value) {
  if (size_ >= capacity_) {
    reserve(capacity_ + 1);
  }
  digits_[size_] = value;
  ++size_;
}

// Iterators
bi_t::dvector::iterator bi_t::dvector::begin() noexcept {
  return digits_.get();
}

bi_t::dvector::iterator bi_t::dvector::end() noexcept {
  return digits_.get() + size_;
}

bi_t::dvector::const_iterator bi_t::dvector::begin() const noexcept {
  return digits_.get();
}

bi_t::dvector::const_iterator bi_t::dvector::end() const noexcept {
  return digits_.get() + size_;
}

bi_t::dvector::riterator bi_t::dvector::rbegin() noexcept {
  return riterator(end());
}

bi_t::dvector::riterator bi_t::dvector::rend() noexcept {
  return riterator(begin());
}

bi_t::dvector::const_riterator bi_t::dvector::rbegin() const noexcept {
  return const_riterator(end());
}

bi_t::dvector::const_riterator bi_t::dvector::rend() const noexcept {
  return const_riterator(begin());
}

/// Sets the size of the vector. Use only if `new_size <= capacity()`
void bi_t::dvector::resize_unsafe(size_t new_size) {
  assert(new_size <= capacity_);
  size_ = new_size;
}

};  // namespace bi
