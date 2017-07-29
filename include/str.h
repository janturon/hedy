#ifndef STR_H
#define STR_H

#include <stddef.h>
#include <vector>
#include <stdarg.h>

class str {
  void handleFrom(int& from);
protected:
  std::vector<size_t> _tokens;
  char* _cstr;
  int _length;
  size_t _capacity;
  int _position;
public:
// properties
  const std::vector<size_t>& tokens;
  char* const& cstr;
  const int& length;
  const size_t& capacity;
  const int& position;

// constructors
  explicit str(int len=0);
  str(long long rhs, size_t addlen=0);
  str(const char* rhs, size_t addlen=0);
  str(const str& rhs, size_t addlen=0);
  str(str&& rhs);

  ~str();

// operators
  str& operator=(const str& rhs);
//	str& operator=(str&& rhs);
  str& operator=(const char* rhs);
  str& operator=(long long rhs);
  bool operator==(const char* rhs);
  bool operator!=(const str& rhs);
  bool operator!=(const char* rhs);
  operator bool() const;
  operator const char*() const;
  bool operator<(const char* rhs) const;
  bool operator<(const str& rhs) const;
  str operator+(const str& rhs);
  str operator+(const char* rhs);
  str operator+(int rhs);
  str operator+(long long rhs);
  friend str operator+(const char* lhs, str& rhs);
  str& operator+=(const str& rhs);
  str& operator+=(const char* rhs);
  str& operator+=(char rhs);
  str& operator+=(int rhs);
  str& operator+=(long long rhs);
  str& operator-=(const str& rhs);
  str& operator-=(const char* rhs);
  str& operator-=(char rhs);
  str& operator-=(int rhs);
  str& operator-=(long long rhs);
  const char* operator()(const char*);
  char* operator-();
  friend const char* operator-(const str& arg);
  char* operator~();
  friend const char* operator~(const str& arg);
  const char* operator()(int pos);
  char& operator[](int pos);

// methods
  unsigned long hash(); // (c) Dan Bernstein
  str& append(const char* what, size_t length=0);
  str& append(long long what);
  str& prepend(const char* what, size_t length=0);
  str& prepend(long long what);
  char* markTo(int where);
  char* markShift(int where=1);
  str& pack();
  str& growBy(size_t bytes);
  str& growTo(size_t bytes);
  str& trimMe();
  const char* trim();
  str& spliceMe(int start, size_t delcnt, const char* add="");
  const char* splice(int start, size_t delcnt, const char* add="");
  str& sliceMe(int from, int to=0);
  const char* slice(int from, int to=0);
  str& substrMe(int from, size_t length=0);
  const char* substr(int from, size_t length=0);
  int indexOf(char c, int from=0) const;
  int indexOf(const char* str, int from=0) const;
  int lastIndexOf(char c, int from=0) const;
  int lastIndexOf(const char* str, int from=0) const;
  int count(char search) const;
	int count(const char* search) const;
  bool startsWith(const char* test, int pos=0);
  bool startsWith(std::vector<const char*> test, int pos=0);
  bool endsWith(const char* test, int pos=0);
  bool endsWith(std::vector<const char*> test, int pos=0);
  void reset();

// secondary methods
  static char buffer[8192];
  const char* moves(const char* pattern="%8191s");
  int movei(const char* pattern="%d", int def=0);
  int scanf(const char* pattern,...);
  bool eat(const char* pattern);
	str& explodeMe(const char* separator=" ", size_t max=0, bool skipempty=true);
	str explode(const char* separator=" ", size_t max=0, bool skipempty=true);
	str& implodeMe(const char* glue="");
	str implode(const char* glue="");
	str replace(const char* from, const char* to, size_t limit=0) const;
	str& replaceMe(const char* from, const char* to, size_t limit=0);
};

#endif