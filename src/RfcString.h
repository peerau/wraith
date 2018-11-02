#ifndef _RFCSTRING_H
#define _RFCSTRING_H 1

namespace bd {
  class String;
}
#include "rfc1459.h"
#include <bdlib/src/String.h>

class RfcString : public bd::String {
  private:

  protected:

  public:
    using String::String;
    RfcString() = default;
    RfcString(const RfcString& str) : String(str) {}
    RfcString(RfcString&& str) noexcept :
      String(std::move(str)) {}
    RfcString(const String &str) : String(str) {}
    RfcString(String &&str) noexcept :
      String(std::move(str)) {}
    using String::operator=;
    RfcString& operator=(const RfcString& rhs) {
      String::operator=(rhs);
      return *this;
    }
    RfcString& operator=(RfcString&& rhs) noexcept {
      String::operator=(std::move(rhs));
      return *this;
    }
    RfcString& operator=(const String& rhs) {
      String::operator=(rhs);
      return *this;
    }

    RfcString& operator=(String&& rhs) noexcept {
      String::operator=(std::move(rhs));
      return *this;
    }

    int compare(const RfcString& str, size_t n = npos) const noexcept
      __attribute__((pure));
    friend bool operator==(const RfcString&, const RfcString&);
    friend bool operator!=(const RfcString&, const RfcString&);
    friend bool operator<(const RfcString&, const RfcString&);
    friend bool operator<=(const RfcString&, const RfcString&);
    friend bool operator>(const RfcString&, const RfcString&);
    friend bool operator>=(const RfcString&, const RfcString&);

    size_t hash() const noexcept;
};

inline bool __attribute__((pure))
operator==(const RfcString& lhs, const RfcString& rhs) {
  return (lhs.length() == rhs.length() &&
      lhs.compare(rhs) == 0);
}

inline bool __attribute__((pure))
operator!=(const RfcString& lhs, const RfcString& rhs) {
  return ! (lhs == rhs);
}

inline bool __attribute__((pure))
operator<(const RfcString& lhs, const RfcString& rhs) {
  return (lhs.compare(rhs) < 0);
}

inline bool __attribute__((pure))
operator<=(const RfcString& lhs, const RfcString& rhs) {
  return ! (rhs < lhs);
}

inline bool __attribute__((pure))
operator>(const RfcString& lhs, const RfcString& rhs) {
  return (rhs < lhs);
}

inline bool __attribute__((pure))
operator>=(const RfcString& lhs, const RfcString& rhs) {
  return ! (lhs < rhs);
}

namespace std {
  template<>
  struct hash<RfcString>
    {
          inline size_t operator()(const RfcString& val) const {
            return val.hash();
          }
    };
}

#endif
