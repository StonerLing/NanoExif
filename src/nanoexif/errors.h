// SPDX-License-Identifier: MIT

#pragma once

#include <cerrno>
#include <exception>
#include <new>
#include <ostream>
#include <string>
#include <system_error>
#include <type_traits>
#include <utility>

namespace nne {

enum class ErrorCode : uint8_t {
  COULD_NOT_OPEN_FILE,
  COULD_NOT_DETERMINE_EXTENSION,
  INVALID_EXIF_HEADER,
  INVALID_JPEG_HEADER,
  UNSUPPORTED_FORMAT,
  INVALID_XMP_HEADER,
  XML_PARSE_ERROR,
  XMP_NOT_FOUND
};

class ErrorCategory : public std::error_category {
 public:
  ErrorCategory() = default;
  [[nodiscard]] const char* name() const noexcept override {
    return "nanoexif error";
  }
  [[nodiscard]] std::string message(int errnum) const override;
};

inline const ErrorCategory& error_category() {
  static ErrorCategory error_category;
  return error_category;
}

[[nodiscard]] std::string ErrorCategory::message(int errnum) const {
  switch (static_cast<ErrorCode>(errnum)) {
    case ErrorCode::COULD_NOT_OPEN_FILE:
      return "Could not open file";
      break;
    case ErrorCode::COULD_NOT_DETERMINE_EXTENSION:
      return "Could not determine extension";
      break;
    case ErrorCode::INVALID_EXIF_HEADER:
      return "Invalid EXIF header";
      break;
    case ErrorCode::INVALID_JPEG_HEADER:
      return "Invalid JPEG header";
      break;
    case ErrorCode::UNSUPPORTED_FORMAT:
      return "Unsupported format";
      break;
    case ErrorCode::INVALID_XMP_HEADER:
      return "Invalid XMP header";
      break;
    case ErrorCode::XML_PARSE_ERROR:
      return "XML parse error";
      break;
    case ErrorCode::XMP_NOT_FOUND:
      return "XMP data not found";
      break;
    default:
      return "Unknown error";
  }
}
}  // namespace nne

namespace std {
template <>
struct is_error_code_enum<nne::ErrorCode> : public true_type {};

std::error_code make_error_code(nne::ErrorCode errnum) {
  return {static_cast<int>(errnum), nne::error_category()};
}
}  // namespace std

namespace nne {

// A simple 'std::expected' in c++17
template <typename T, typename E = std::error_code>
class Result {
 public:
  using value_type = T;
  using error_type = E;

  Result() : has_value_(false) { new (&storage_.error) E(); }

  template <typename U = T,
            typename = std::enable_if_t<std::is_constructible_v<T, U&&>>>
  explicit Result(U&& value) : has_value_(true) {
    new (&storage_.val) T(std::forward<U>(value));
  }

  template <typename U = E,
            typename = std::enable_if_t<std::is_constructible_v<E, U&&>>>
  static Result FromError(U&& error) {
    Result result;
    result.AssignError(std::forward<U>(error));
    return result;
  }

  static Result FromErrno(int errnum) {
    return FromError(E(errnum, std::generic_category()));
  }

  Result(const Result& other) : has_value_(other.has_value_) {
    if (has_value_) {
      new (&storage_.val) T(other.value());
    } else {
      new (&storage_.error) E(other.error());
    }
  }

  Result(Result&& other) noexcept : has_value_(other.has_value_) {
    if (has_value_) {
      new (&storage_.val) T(std::move(other.value()));
    } else {
      new (&storage_.error) E(std::move(other.error()));
    }
  }

  Result& operator=(const Result& other) {
    if (this == &other) {
      return *this;
    }
    if (has_value_ && other.has_value_) {
      value() = other.value();
    } else if (has_value_ && !other.has_value_) {
      value().~T();
      new (&storage_.error) E(other.error());
      has_value_ = false;
    } else if (!has_value_ && other.has_value_) {
      error().~E();
      new (&storage_.val) T(other.value());
      has_value_ = true;
    } else {
      error() = other.error();
    }
    return *this;
  }

  Result& operator=(Result&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    if (has_value_ && other.has_value_) {
      value() = std::move(other.value());
    } else if (has_value_ && !other.has_value_) {
      value().~T();
      new (&storage_.error) E(std::move(other.error()));
      has_value_ = false;
    } else if (!has_value_ && other.has_value_) {
      error().~E();
      new (&storage_.val) T(std::move(other.value()));
      has_value_ = true;
    } else {
      error() = std::move(other.error());
    }
    return *this;
  }

  ~Result() { Destroy(); }

  [[nodiscard]] bool has_value() const noexcept { return has_value_; }
  explicit operator bool() const noexcept { return has_value(); }

  const E& error() const& noexcept {
    if (has_value_) {
      static const E no_error{};
      return no_error;
    }
    return storage_.error;
  }
  E& error() & noexcept {
    if (has_value_) {
      static E no_error{};
      return no_error;
    }
    return storage_.error;
  }

  T& value() & {
    if (!has_value_) {
      throw std::runtime_error("Result has no value");
    }
    return storage_.val;
  }
  const T& value() const& {
    if (!has_value_) {
      throw std::runtime_error("Result has no value");
    }
    return storage_.val;
  }
  T&& value() && {
    if (!has_value_) {
      throw std::runtime_error("Result has no value");
    }
    return std::move(storage_.val);
  }

  T& operator*() & { return value(); }
  const T& operator*() const& { return value(); }
  T* operator->() { return &value(); }
  const T* operator->() const { return &value(); }

  template <typename U>
  T value_or(U&& alt) const& {
    if (has_value_) {
      return storage_.val;
    }
    return static_cast<T>(std::forward<U>(alt));
  }
  template <typename U>
  T value_or(U&& alt) && {
    if (has_value_) {
      return std::move(storage_.val);
    }
    return static_cast<T>(std::forward<U>(alt));
  }

  template <typename... Args>
  T& emplace(Args&&... args) {
    if (has_value_) {
      storage_.val.~T();
      new (&storage_.val) T(std::forward<Args>(args)...);
    } else {
      storage_.error.~E();
      new (&storage_.val) T(std::forward<Args>(args)...);
      has_value_ = true;
    }
    return storage_.val;
  }

  // Assign an error
  template <typename U = E>
  void AssignError(U&& error) {
    if (has_value_) {
      storage_.val.~T();
      new (&storage_.error) E(std::forward<U>(error));
      has_value_ = false;
    } else {
      storage_.error = std::forward<U>(error);
    }
  }

  void swap(Result& other) noexcept {
    if (this == &other) {
      return;
    }
    if (has_value_ && other.has_value_) {
      using std::swap;
      swap(storage_.val, other.storage_.val);
    } else if (!has_value_ && !other.has_value_) {
      using std::swap;
      swap(storage_.error, other.storage_.error);
    } else if (has_value_ && !other.has_value_) {
      T tmp = std::move(storage_.val);
      E tmp_err = std::move(other.storage_.error);
      storage_.val.~T();
      new (&storage_.error) E(std::move(tmp_err));
      new (&other.storage_.val) T(std::move(tmp));
      has_value_ = false;
      other.has_value_ = true;
    } else {
      other.swap(*this);
    }
  }

 private:
  void Destroy() noexcept {
    if (has_value_) {
      storage_.val.~T();
    } else {
      storage_.error.~E();
    }
  }

  union Storage {
    char dummy_;
    T val;
    E error;
    Storage() : dummy_() {}
    ~Storage() {}
  } storage_;

  bool has_value_;
};

template <typename T, typename E>
bool operator==(const Result<T, E>& a, const Result<T, E>& b) {  // NOLINT
  if (a.has_value() && b.has_value()) {
    return *a == *b;
  }
  if (!a.has_value() && !b.has_value()) {
    return a.error() == b.error();
  }
  return false;
}
template <typename T, typename E>
bool operator!=(const Result<T, E>& a, const Result<T, E>& b) {  // NOLINT
  return !(a == b);
}

template <typename T, typename E>
auto operator<<(std::ostream& os, const Result<T, E>& r)  // NOLINT
    -> std::ostream& {
  if (r.has_value()) {
    os << r.value();
  } else {
    if constexpr (std::is_same_v<E, std::error_code>) {
      os << "[error: " << r.error().value() << " (" << r.error().message()
         << ")]";
    } else {
      os << "[error]";
      if constexpr (std::is_same_v<decltype(std::declval<std::ostream&>()
                                            << std::declval<E>()),
                                   std::ostream&>) {
        os << ' ' << r.error();
      }
    }
  }
  return os;
}

template <typename T>
Result<T> make_result(T value) {
  return Result<T>(value);
}
template <typename T>
Result<T> make_result(ErrorCode errnum) {
  return Result<T>::FromError(std::make_error_code(errnum));
}
template <typename T>
Result<T> make_result(std::error_code error) {
  return Result<T>::FromError(error);
}

}  // namespace nne
