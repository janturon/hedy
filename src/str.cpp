#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "str.h"

#define PROPS length(_length), capacity(_capacity), cstr(_cstr), position(_position), tokens(_tokens)

char str::buffer[8192] = "";

// private functions
// I don't want to rely on fucking locale: space character is always space. period.
inline bool isspace9(char c) {
  return c==' ' || c=='\t' || c=='\r' || c=='\n';
}

// private methods
void str::handleFrom(int& from) {
  if(from>length) from = length;
  if(from<0) from = length+from;
  if(from<0) from = 0;
}

// constructors
str::str(int len) : PROPS {
  _position = 0;
  _length = len;
  _capacity = len+1;
  _cstr = (char*)malloc(capacity);
  cstr[0] = 0;
}

str::str(const char* rhs, size_t addlen) : PROPS  {
  _position = 0;
  _length = strlen(rhs);
  _capacity = length + addlen + 1;
  _cstr = (char*)malloc(capacity);
  strcpy(cstr,rhs);
}

str::str(const str& rhs, size_t addlen) : PROPS {
  _position = rhs.position;
  _length = rhs.length;
  _capacity = rhs.capacity + addlen;
  _tokens = rhs.tokens;
  _cstr = (char*)malloc(capacity);
  // can't use strcpy: string may be exploded
  for(int i=0; i<length; ++i) cstr[i] = rhs.cstr[i];
  cstr[length] = 0;
}

str::str(str&& rhs) : PROPS {
  _position = rhs.position;
  _length = rhs.length;
  _capacity = rhs.capacity;
  _tokens = rhs.tokens;
  _cstr = rhs.cstr;
	rhs._cstr = NULL;
}

str::str(long long rhs, size_t addlen) : PROPS {
  sprintf(str::buffer,"%lld",rhs);
  _position = 0;
  _length = strlen(str::buffer);
  _capacity = length + addlen + 1;
  _cstr = (char*)malloc(capacity);
  strcpy(cstr,str::buffer);
}

str::~str() {
  free(_cstr);
}

// operators
str& str::operator=(const str& rhs) {
  _length = rhs.length;
  _position = rhs.position;
  _tokens = rhs.tokens;
  if(capacity<length+1) {
    _capacity = length+1;
    _cstr = (char*)realloc(_cstr,capacity);
  }
  for(int i=0; i<rhs.length; ++i) cstr[i] = rhs.cstr[i];
  cstr[length] = 0;
  return *this;
}
str& str::operator=(const char* rhs) {
  _length = strlen(rhs);
  _position = 0;
  if(capacity<length+1) {
    _capacity = length+1;
    _cstr = (char*)realloc(_cstr,capacity);
  }
  strcpy(cstr,rhs);
  return *this;
}
str& str::operator=(long long rhs) {
  sprintf(str::buffer,"%lld",rhs);
  _length = strlen(str::buffer);
  _position = 0;
  if(capacity<length+1) {
    _capacity = length+1;
    _cstr = (char*)realloc(_cstr,capacity);
  }
  strcpy(cstr,str::buffer);
  return *this;
}

bool str::operator==(const char* rhs) { return strcmp(cstr,rhs)==0; }
bool str::operator!=(const str& rhs) { return strcmp(cstr,rhs.cstr)!=0; }
bool str::operator!=(const char* rhs) { return strcmp(cstr,rhs)!=0; }
str::operator bool() const { for(size_t i=0; i<length; ++i) if(!isspace9(cstr[i])) return true; return false; }
str::operator const char*() const { return cstr; }
bool str::operator<(const char* rhs) const { return strcmp(cstr,rhs)<0; }
bool str::operator<(const str& rhs) const { return strcmp(cstr,rhs.cstr)<0; }

str str::operator+(const str& rhs) {
  str result(*this,rhs.length);
  strcat(result.cstr,rhs.cstr);
  result._length+= rhs.length;
  return static_cast<str&&>(result);
}
str str::operator+(const char* rhs) {
  size_t rlen = strlen(rhs);
  str result(cstr,rlen);
  strcat(result.cstr,rhs);
  result._length+= rlen;
  return static_cast<str&&>(result);
}
str str::operator+(int rhs) {
  sprintf(str::buffer,"%d",rhs);
  size_t rlen = strlen(str::buffer);
  str result(cstr,rlen);
  strcat(result.cstr,str::buffer);
  result._length+= rlen;
  return static_cast<str&&>(result);
}
str str::operator+(long long rhs) {
  sprintf(str::buffer,"%lld",rhs);
  size_t rlen = strlen(str::buffer);
  str result(cstr,rlen);
  strcat(result.cstr,str::buffer);
  result._length+= rlen;
  return static_cast<str&&>(result);
}
str operator+(const char* lhs, str& rhs) {
  str result(lhs,rhs.length);
  strcat(result.cstr,rhs.cstr);
  result._length+= rhs.length;
  return static_cast<str&&>(result);
}


str& str::operator+=(const str& rhs) { return append(rhs.cstr); }
str& str::operator+=(const char* rhs) { return append(rhs); }
str& str::operator+=(char rhs) { char srhs[]={rhs,0}; return append(srhs); }
str& str::operator+=(int rhs) { return append(rhs); }
str& str::operator+=(long long rhs) { return append(rhs); }
str& str::operator-=(const str& rhs) { return prepend(rhs.cstr); }
str& str::operator-=(const char* rhs) { return prepend(rhs); }
str& str::operator-=(char rhs) { char srhs[]={rhs,0}; return prepend(srhs); }
str& str::operator-=(int rhs) { return prepend(rhs); }
str& str::operator-=(long long rhs) { return prepend(rhs); }

char* str::operator-() { return cstr; }
const char* operator-(const str& arg) { return arg.cstr; }
char* str::operator~() { return cstr+position; }
const char* operator~(const str& arg) { return arg.cstr+arg.position; }
const char* str::operator()(int pos) {
  if(pos>tokens.size()-1) return "";
  return cstr+tokens[pos];
}
char& str::operator[](int pos) {
  return cstr[position+pos];
}


// methods
unsigned long str::hash() {
  unsigned long result = 5381;
  for(const char* i=cstr; *i!=0; ++i) result+= (result << 5) + *i;
  return result;
}
str& str::append(const char* what, size_t wlength) {
  int rhslen = strlen(what);
  if(wlength!=0 && wlength<rhslen) rhslen = wlength;
  _length+= rhslen;
  if(length>=capacity) {
    _capacity = length+1;
    _cstr = (char*)realloc(_cstr,capacity);
  }
  strncat(cstr,what,rhslen);
  cstr[length] = 0;
  return *this;
}
str& str::append(long long what) {
  sprintf(str::buffer,"%lld",what);
  return append(str::buffer,0);
}
str& str::prepend(const char* what, size_t wlength) {
  if(wlength==0) wlength = strlen(what);
  size_t cut = 0, newlength = length+wlength;
  if(newlength>=capacity) {
    _capacity = newlength+1;
    _cstr = (char*)realloc(_cstr,capacity);
  }
  for(int i=newlength; i-->wlength;) cstr[i] = cstr[i-wlength];
  for(int i=wlength; i-->0;) cstr[i] = what[i];
  _length = newlength;
  cstr[length] = 0;
  return *this;
}
str& str::prepend(long long what) {
  sprintf(str::buffer,"%lld",what);
  return prepend(str::buffer,0);
}

char* str::markTo(int where) {
  handleFrom(where);
  _position = where;
  return cstr+position;
}
char* str::markShift(int where) {
  return markTo(position+where);
}
str& str::trimMe() {
  if(!cstr[0]) return *this;
  int lshift = 0;
  char* p = _cstr;
  while(isspace9(*p)) ++lshift,++p;
  while(*p) *(p-lshift) = *p++;
  p-= lshift+1;
  while(isspace9(*p)) --_length,--p;
  *(p+1) = 0;
  _length-= lshift;
  _position-= lshift;
  if(position<0) _position = 0;
  for(p=_cstr+position; *p==' '||*p=='\t'; ++p) ++_position;
  if(position>length) _position = length;
  return *this;
}
const char* str::trim() {
  if(!cstr[0]) return "";
  int lshift = 0, rshift = 0;
  char* p = cstr;
  while(isspace9(*p)) ++lshift,++p;
  p+= length-1-lshift;
  while(isspace9(*p)) ++rshift,--p;
  int resultlen = length-lshift-rshift;
  strncpy(str::buffer, cstr+lshift, resultlen);
  str::buffer[resultlen] = 0;
  return str::buffer;
}
str& str::spliceMe(int start, size_t delcnt, const char* add) {
  handleFrom(start);
  if(delcnt>length-start) delcnt = length-start;
  int addlen = strlen(add);
  int delta = addlen-delcnt;
  if(length+delta>=capacity) {
    _capacity = length+delta+1;
    _cstr = (char*)realloc(_cstr,capacity);
  }
  if(position>start) {
    _position-= delcnt;
    if(position<start) _position = start;
  }
  if(position>start) _position+= addlen;
  if(delta>0) for(int i=length; i>=start; --i) cstr[i+delta] = cstr[i];
  for(int i=0; i<addlen; ++i) _cstr[i+start] = add[i];
  if(delta<0) for(int i=start+addlen; i<length; ++i) cstr[i] = cstr[i-delta];
  _length+= delta;
  cstr[length] = 0;
  return *this;
}
const char* str::splice(int start, size_t delcnt, const char* add) {
  handleFrom(start);
  if(delcnt>length-start) delcnt = length-start;
  int addlen = strlen(add);
  int delta = addlen-delcnt;
  strncpy(str::buffer, cstr, start);
  strncpy(str::buffer+start, add, addlen);
  strcpy(str::buffer+start+addlen, cstr+start+delcnt);
  return str::buffer;
}
str& str::sliceMe(int from, int to) {
  handleFrom(from);
  if(to==0 || to>length) to = length;
  if(to<0) to = length+to;
  if(to<0) to = 0;
  if(from<to) {
    _length = to-from+1;
    for(int i=0; i<length; ++i) _cstr[i] = _cstr[from+i];
    _cstr[length] = 0;
  }
  if(position>=from) _position-= from;
  else _position = 0;
  return *this;
}
const char* str::slice(int from, int to) {
  handleFrom(from);
  if(to==0 || to>length) to = length;
  if(to<0) to = length+to;
  if(to<0) to = 0;
  if(from>=to) return str();
  strncpy(str::buffer, cstr+from, to-from+1);
  str::buffer[to-from+1] = 0;
  return str::buffer;
}
str& str::substrMe(int from, size_t length) {
  handleFrom(from);
  if(length==0) length = _length-from;
  if(length>_length-from) length = _length-from;
  if(length>0) {
    _length = length;
    for(int i=0; i<_length; ++i) _cstr[i] = _cstr[from+i];
    cstr[_length] = 0;
  }
  _position-= from;
  if(position<0) _position = 0;
  if(position>length) _position = length;
  return *this;
}
const char* str::substr(int from, size_t length) {
  handleFrom(from);
  if(length==0) length = _length-from;
  if(length>_length-from) length = _length-from;
  if(length>0) {
    strncat(str::buffer,cstr+from,length);
    return str::buffer;
  }
  else return "";
}
int str::indexOf(char c, int from) const {
  if(from<0) from = length+from;
  if(from<0 || from>length) return -1;
  for(int i=from; i<length; ++i) if(cstr[i]==c) return i;
  return -1;
}
int str::indexOf(const char* str, int from) const {
  if(from<0) from = length+from;
  if(from<0 || from>length) return -1;
  const char* pos = strstr(cstr+from,str);
  if(pos==NULL) return -1;
  return pos-cstr;
}
int str::lastIndexOf(char c, int from) const {
  if(from==0) from = length;
  if(from<0) from = length+from;
  if(from<0 || from>length) return -1;
  for(int i=from; i-->0;) if(cstr[i]==c) return i;
  return -1;
}
int str::lastIndexOf(const char* str, int from) const {
  if(from==0) from = length;
  if(from<0) from = length+from;
  if(from<0 || from>length) return -1;
  size_t len = strlen(str);
  for(int i=from; i-->0;) if(strncmp(cstr+i,str,len)==0) return i;
  return -1;
}
int str::count(char search) const {
  int result = 0;
  for(const char* i=cstr; *i!=0; ++i) if(*i==search) ++result;
  return result;
}
int str::count(const char* search) const {
  int result = 0;
  for(const char* i=cstr; i=strstr(i,search); ++i) ++result;
  return result;
}
bool str::startsWith(const char* test, int pos) {
  handleFrom(pos);
  for(int i=0; *(test+i)!=0; ++i) {
    if(*(test+i)!=*(cstr+i+pos)) return false;
  }
  return true;
}
bool str::startsWith(std::vector<const char*> test, int pos) {
  for(auto const& i: test) if(startsWith(i,pos)) return true;
  return false;
}
bool str::endsWith(const char* test, int pos) {
  if(pos==0) pos = length;
  handleFrom(pos);
  if(pos==0) return false;
  for(int i=strlen(test),j=i; i-->0;) {
    if(*(test+i)!=*(cstr+pos-j+i)) return false;
  }
  return true;
}
bool str::endsWith(std::vector<const char*> test, int pos) {
  for(auto const& i: test) if(endsWith(i,pos)) return true;
  return false;
}

str& str::pack() {
  _capacity = length+1;
  _cstr = (char*)realloc(_cstr,capacity);
  return *this;
}
str& str::growBy(size_t bytes) {
  _capacity+= bytes+1;
  _cstr = (char*)realloc(_cstr,capacity);
  return *this;
}
str& str::growTo(size_t bytes) {
  if(bytes>_capacity) {
    _capacity = bytes+1;
    _cstr = (char*)realloc(_cstr,capacity);
  }
  return *this;
}
void str::reset() {
  _length = strlen(cstr);
  _position = 0;
}

// secondary methods
const char* str::moves(const char* pattern) {
/*
  str::buffer[0] = 0;
  while(cstr[position]==' ') _position++;
  sscanf(cstr+position, pattern, str::buffer);
  markShift(strlen(str::buffer));
  return str::buffer;
*/
  str::buffer[254] = 0;
  snprintf(str::buffer,256," %s%%n",pattern);
  if(str::buffer[254]!=0) throw "str::moves buffer overflow";
  int tail = 0;
  str spattern = str::buffer;
  sscanf(cstr+position, spattern.cstr, str::buffer, &tail);
  if(tail==0) str::buffer[0] = 0;
  else markShift(tail);
  return str::buffer;
}
int str::movei(const char* pattern, int def) {
  str::buffer[254] = 0;
  snprintf(str::buffer,256," %s%%n",pattern);
  if(str::buffer[254]!=0) throw "str::move buffer overflow";
  int tail = 0, result = 0;
  sscanf(cstr+position, str::buffer, &result, &tail);
  if(tail==0) return def;
  else markShift(tail);
  return result;
}
int str::scanf(const char* pattern,...) {
  va_list args;
  va_start(args,pattern);
  return vsscanf(cstr+position, pattern, args);
}
bool str::eat(const char* pattern) {
  str::buffer[254] = 0;
  snprintf(str::buffer,256," %s%%n",pattern);
  if(str::buffer[254]!=0) throw "str::move buffer overflow";
  int tail = 0;
  sscanf(cstr+position, str::buffer, &tail);
  if(tail==0) return false;
  else markShift(tail);
  return true;
}

str& str::explodeMe(const char* separator, size_t max, bool skipempty) {
  size_t cnt = 1;
  size_t seplen = strlen(separator);
  _tokens.clear();
  _tokens.push_back(0);
  if(max==1 || separator[0]==0) return *this;
  for(char* p=_cstr; *p!=0; ++p) if(strncmp(p,separator,seplen)==0) {
    for(char* q=p; q!=_cstr; --q) if(isspace9(*q)) *q = 0; else break;
    for(const char* q=separator; *q!=0; ++q) *p++ = 0;
    while(isspace9(*p)) *p++ = 0;
    if(*p==0) break;
    if(!(skipempty && strstr(p+1,separator)==p)) {
      ++cnt;
      _tokens.push_back(p-_cstr);
      if(max>=cnt) break;
    }
  }
  return *this;
}

str str::explode(const char* separator, size_t max, bool skipempty) {
  str result = *this;
  result.explodeMe(separator,max,skipempty);
  return static_cast<str&&>(result);
}

str& str::implodeMe(const char* glue) {
  size_t gluelen = strlen(glue);
  size_t cut = 0;
  for(char* p=_cstr; p-_cstr<length; ++p) if(*p==0) {
    if(cut==0) for(const char* q=p; *q==0 && (q-p)<length; ++q) cut++;
    spliceMe(p-_cstr,cut,glue);
  }
  _tokens.clear();
  return *this;
}

str str::implode(const char* glue) {
  str result = *this;
  result.implodeMe(glue);
  return static_cast<str&&>(result);
}

str str::replace(const char* from, const char* to, size_t limit) const {
  size_t cnt = limit==0 ? count(from) : limit;
  if(cnt==0) return *this;
  size_t fromlen = strlen(from);
	if(fromlen==0) return *this;
  size_t tolen = strlen(to);
  if(limit==0) limit = 65535;
  str result("",length+cnt*(tolen-fromlen));
  const char *prev = cstr, *next = cstr;
  do {
    next = strstr(next,from);
    if(next>prev) result.append(prev,next-prev);
    if(!next) break;
    result.append(to,tolen);
    next+= fromlen;
    prev = next;
  } while(--cnt>0);
  result.append(next);
  return static_cast<str&&>(result);
}

str& str::replaceMe(const char* from, const char* to, size_t limit) {
  if(length==0) return *this;
  size_t fromlen = strlen(from);
	if(fromlen==0) return *this;
  size_t tolen = strlen(to);
  if(limit==0) limit = 65535;
  int delta = tolen-fromlen;
  if(delta>0) {
    std::vector<int> marks;
    for(const char* i=cstr; *i!=0 && limit>0; ++i) {
      if(i=strstr(i,from)); else break;
      marks.push_back(i-cstr);
      i+= fromlen;
      --limit;
    }
    if(marks.size()==0) return *this;
    int prev = length+1;
    _length+= marks.size()*delta;
    if(limit>0 && length>limit) _length = limit;
    growTo(length);
    for(int i=marks.size(); i-- > 0; ) {
      size_t mi = marks[i];
      for(int s=prev-1; s>=mi+fromlen; --s) {
        size_t newlen = s+(i+1)*delta;
        if(limit>0 && newlen>limit) continue;
        cstr[newlen] = cstr[s];
      }
      size_t thelen = tolen;
      size_t startpos = mi+i*delta;
      if(limit>0 && startpos>limit) continue;
      if(limit>0 && startpos+tolen>limit) thelen = limit - startpos;
      strncpy(cstr+startpos,to,thelen);
      prev = mi;
      if(prev==0) break;
    }
  }
  else if(delta<=0) {
    char *i,*j;
    for(i=j=_cstr; *j!=0 && limit>0; ++i,++j) {
      if(startsWith(from,j-_cstr)) {
        strncpy(i,to,tolen);
        i+= tolen-1;
        j+= fromlen-1;
        --limit;
      }
      else *i = *j;
    }
    strcpy(i,j);
    _length-= j-i;
  }
  cstr[length] = 0;
  return *this;
}
