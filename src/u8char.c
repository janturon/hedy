#include <stdarg.h>
#include <string.h>
#include "u8char.h"

char* u8trim(char* str) {
  // basespace char length is always 1
  while(u8isbasespacec(str) && *str!=0) ++str;
  char* end = str;
  while(*end!=0) end = u8shiftc(end,1);
  for(end=u8shiftc(end,-1); u8isbasespacec(end); --end) *end = 0;
  return str;
}

int u8strlenc(char* str) {
  return u8strlencc((const char*) str);
}

int u8strlencc(const char* str) {
  int len = 0;
  for(const char* p=str; *p!=0; ++p) {
    ++len;
    if(*p>0) continue;
    unsigned char c = *p;
    while((c<<=1)&128) ++p;
  }
  return len;
}

#ifdef __cplusplus
inline int u8strlen(char* str) { return u8strlenc(str); }
inline int u8strlen(const char* str) { return u8strlencc(str); }
#endif

int u8charleni(int n) {
  if(n<=0) return 0;
  if(n<0x80) return 1;
  if(n<0x800) return 2;
  if(n<0x10000) return 3;
  if(n<0x200000) return 4;
  if(n<0x4000000) return 5;
  if(n<0x80000000) return 6;
  return 0;
}

int u8charlenc(char* str) {
  return u8charlencc((const char*)str);
}

int u8charlencc(const char* str) {
  if(*str==0) return 0;
  int len = 1;
  if(*str<0) {
    unsigned char c = *str;
    while((c<<=1)&128) ++len;
  }
  return len;
}

#ifdef __cplusplus
inline int u8charlen(int n) { return u8charleni(n); }
inline int u8charlen(char* str) { return u8charlenc(str); }
inline int u8charlen(const char* str) { return u8charlencc(str); }
#endif

char* u8shiftc(char* str, int len) {
  if(len>0) for(int i=0; i<len; ++i) {
    if(*str==0) return str;
    if(*str<0) {
      unsigned char c = *str;
      while((c<<=1)&128) ++str;
    }
    ++str;
  }
  else if(len<0) do {
    --str;
  } while(*str&&192==128);
  return str;
}

const char* u8shiftcc(const char* str, int len) {
  if(len>0) for(int i=0; i<len; ++i) {
    if(*str==0) return str;
    if(*str<0) {
      unsigned char c = *str;
      while((c<<=1)&128) ++str;
    }
    ++str;
  }
  else if(len<0) for(int i=len; i<0; ++i ) do {
    --str;
  } while(*str&&192==128);
  return str;
}

#ifdef __cplusplus
inline char* u8shift(char*& str, int len) { return u8shiftc(str,len); }
inline const char* u8shift(const char* str, int len) { return u8shiftcc(str,len); }
#endif

const char* u8charc(char* str) {
  return u8charcc((const char*)str);
}

const char* u8charcc(const char* str) {
  static char bytes[7];
  bytes[0] = *str;
  if(*str>0) {
    bytes[1] = 0;
    return bytes;
  }
  int len = 1;
  unsigned char c = *str;
  while((c<<=1)&128) ++len;
  for(int i=1; i<len; ++i) bytes[i] = str[i];
  bytes[len] = 0;
  return bytes;
}

#ifdef __cplusplus
inline const char* u8char(char* str) { return u8charc(str); }
inline const char* u8char(const char* str) { return u8charcc(str); }
#endif

int u8tointc(char* str) {
  return u8tointcc((const char*)str);
}

int u8tointcc(const char* str) {
  if(!(*str&128)) return *str;
  unsigned char c = *str, bytes = 0;
  while((c<<=1)&128) ++bytes;
  int result = 0;
  for(int i=bytes; i>0; --i) result|= (*(str+i)&127)<<(6*(bytes-i));
  int mask = 1;
  for(int i=bytes; i<6; ++i) mask<<= 1, mask|= 1;
  result|= (*str&mask)<<(6*bytes);
  return result;
}

#ifdef __cplusplus
inline int u8toint(char* str) { return u8tointc(str); }
inline int u8toint(const char* str) { return u8tointcc(str); }
#endif

const char* inttou8(int n) {
  static char bytes[7];
  if(n<0x7F) {
    bytes[0] = n;
    bytes[1] = 0;
    return bytes;
  }
  int size = 0;
  for(int m=n; m>0; m>>=6) ++size;
  if(size>6) { bytes[0] = 0; return bytes; }
  bytes[size] = 0;
  for(int i=size-1; i>0; --i, n>>=6) bytes[i] = 128|(n&63);
  bytes[0] = (255>>(7-size))<<(8-size);
  bytes[0]|= n;
//  if((bytes[0]&(1<<(7-size)))!=0) bytes[0] = 0;
  return bytes;
}

int inttou8s(int n, char* target) {
  static char bytes[7];
  if(n<0x7F) {
    target[0] = n;
    target[1] = 0;
    return 1;
  }
  int size = 0;
  for(int m=n; m>0; m>>=6) ++size;
  if(size>6) return 0;
  target[size] = 0;
  for(int i=size-1; i>0; --i, n>>=6) target[i] = 128|(n&63);
  target[0] = (255>>(7-size))<<(8-size);
  target[0]|= n;
  return size;
}

int u8stowcs(wchar_t* to, const char* from) {
  int i = 0;
  for(const char* p=from; *p!=0; p=u8shiftcc(p,1)) {
    to[i++] = u8toint(p);
  }
  to[i] = 0;
  return i;
}

int wcstou8s(char* to, const wchar_t* from) {
  int i = 0;
  char* start = to;
  for(; from[i]!=0; ++i) {
    to+= inttou8s(from[i],to);
  }
  return to-start;
}

static inline int in(int n, int from, int to) { return n>=from && n<=to; }
static inline int in2(int n, const int* arr, int len) { for(int i=0; i<len; ++i) if(arr[i]==n) return 1; return 0; }

int u8toupper(int chr) {
  if(in(chr,0x61,0x7A)||in(chr,0xE0,0xFE)||in(chr,0x3B1,0x3C9)||in(chr,0x430,0x44F)) return chr-32;
  if(in(chr,0x450,0x45F)) return chr-0x50;
  if(in(chr,0x101,0x17E) && chr%2==1) return chr-1;
  if((in(chr,0x461,0x481)||in(chr,0x48B,0x52F)) && chr%2==1) return chr-1;
  return chr;
}

int u8tolower(int chr) {
  if(in(chr,0x41,0x5A)||in(chr,0xC0,0xDE)||in(chr,0x391,0x3A9)||in(chr,0x410,0x42F)) return chr+0x20;
  if(in(chr,0x400,0x40F)) return chr+0x50;
  if(in(chr,0x100,0x17D) && chr%2==0) return chr+1;
  if((in(chr,0x460,0x480)||in(chr,0x48A,0x52E)) && chr%2==0) return chr+1;
  return chr;
}

inline int u8isdigiti(int n) { return in(n,0x30,0x39); }
inline int u8isdigitc(char* c) { return u8isdigiti(u8tointc(c)); }
inline int u8isdigitcc(const char* c) { return u8isdigiti(u8tointcc(c)); }

inline int u8isupperasciii(int n) { return in(n,0x41,0x5A); }
inline int u8isupperasciic(char* c) { return u8isupperasciii(u8tointc(c)); }
inline int u8isupperasciicc(const char* c) { return u8isupperasciii(u8tointcc(c)); }
inline int u8islowerasciii(int n) { return in(n,0x61,0x7A); }
inline int u8islowerasciic(char* c) { return u8islowerasciii(u8tointcc(c)); }
inline int u8islowerasciicc(const char* c) { return u8islowerasciii(u8tointcc(c)); }
inline int u8isasciiletteri(int n) { return u8islowerasciii(n)||u8isupperasciii(n); }
inline int u8isasciiletterc(char* c) { return u8isasciiletteri(u8tointc(c)); }
inline int u8isasciilettercc(const char* c) { return u8isasciiletteri(u8tointcc(c)); }

inline int u8isuppergreeki(int n) { return in(n,0x391,0x3A9);}
inline int u8isuppergreekc(char* c) { return u8isuppergreeki(u8tointc(c)); }
inline int u8isuppergreekcc(const char* c) { return u8isuppergreeki(u8tointcc(c)); }
inline int u8islowergreeki(int n) { return in(n,0x3B1,0x3C9); }
inline int u8islowergreekc(char* c) { return u8islowergreeki(u8tointc(c)); }
inline int u8islowergreekcc(const char* c) { return u8islowergreeki(u8tointcc(c)); }
inline int u8isgreeki(int n) { return u8islowergreeki(n)||u8isuppergreeki(n); }
inline int u8isgreekc(char* c) { return u8isgreeki(u8tointc(c)); }
inline int u8isgreekcc(const char* c) { return u8isgreeki(u8tointcc(c)); }

int u8islowerlatini(int n) {
  if(n!=0xF7&&in(n,0xE0,0xFE)||in(n,0x3B1,0x3C9)) return 1;
  if(in(n,0x101,0x17E) && n%2==1) return 1;
  return 0;
}
inline int u8islowerlatinc(char* c) { return u8islowerlatini(u8tointc(c)); }
inline int u8islowerlatincc(const char* c) { return u8islowerlatini(u8tointcc(c)); }
int u8isupperlatini(int n) {
  if(n!=0xD7&&in(n,0xC0,0xDE)||in(n,0x391,0x3A9)) return 1;
  if(in(n,0x100,0x17D) && n%2==0) return 1;
  return 0;
}
inline int u8isupperlatinc(char* c) { return u8isupperlatini(u8tointc(c)); }
inline int u8isupperlatincc(const char* c) { return u8isupperlatini(u8tointcc(c)); }
inline int u8islatini(int n) { return u8islowerlatini(n)||u8isupperlatini(n); }
inline int u8islatinc(char* c) { return u8islatini(u8tointc(c)); }
inline int u8islatincc(const char* c) { return u8islatini(u8tointcc(c)); }

inline int u8isbasesigni(int n) { return in(n,0x21,0x2F)||in(n,0x3A,0x40)||in(n,0x5B,0x60)||in(n,0x7B,0x7E); }
inline int u8isbasesignc(char* c) { return u8isbasesigni(u8tointc(c)); }
inline int u8isbasesigncc(const char* c) { return u8isbasesigni(u8tointcc(c)); }
inline int u8islatinsigni(int n) { return in(n,0xA1,0xBF)||n==0xD7||n==0xF7; }
inline int u8islatinsignc(char* c) { return u8islatinsigni(u8tointc(c)); }
inline int u8islatinsigncc(const char* c) { return u8islatinsigni(u8tointcc(c)); }
inline int u8issigni(int n) { return u8isbasesigni(n)||u8islatinsigni(n); }
inline int u8issignc(char* c) { return u8issigni(u8tointc(c)); }
inline int u8issigncc(const char* c) { return u8issigni(u8tointcc(c)); }

inline int u8isuppercyrilici(int n) { return in(n,0x400,0x42F)||((in(n,0x460,0x480)||in(n,0x48A,0x52E)) && n%2==0); }
inline int u8isuppercyrilicc(char* c) { return u8isuppercyrilici(u8tointc(c)); }
inline int u8isuppercyriliccc(const char* c) { return u8isuppercyrilici(u8tointcc(c)); }
inline int u8islowercyrilici(int n) { return in(n,0x430,0x45F)||((in(n,0x461,0x481)||in(n,0x48B,0x52F)) && n%2==1); }
inline int u8islowercyrilicc(char* c) { return u8islowercyrilici(u8tointc(c)); }
inline int u8islowercyriliccc(const char* c) { return u8islowercyrilici(u8tointcc(c)); }
inline int u8iscyrilici(int n) { return u8islowercyrilici(n)||u8isuppercyrilici(n); }
inline int u8iscyrilicc(char* c) { return u8iscyrilici(u8tointc(c)); }
inline int u8iscyriliccc(const char* c) { return u8iscyrilici(u8tointcc(c)); }

inline int u8isloweri(int n) { return u8islowerasciii(n)||u8islowerlatini(n)||u8islowergreeki(n)||u8islowercyrilici(n); }
inline int u8islowerc(char* c) { return u8isloweri(u8tointc(c)); }
inline int u8islowercc(const char* c) { return u8isloweri(u8tointcc(c)); }
inline int u8isupperi(int n) { return u8isupperasciii(n)||u8isupperlatini(n)||u8isuppergreeki(n)||u8isuppercyrilici(n); }
inline int u8isupperc(char* c) { return u8isupperi(u8tointc(c)); }
inline int u8isuppercc(const char* c) { return u8isupperi(u8tointcc(c)); }

int u8isbasespacei(int n) {
  static int spaces[] = {0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x20, 0xA0};
  return in2(n,spaces,sizeof(spaces)/sizeof(int));
}
inline int u8isbasespacec(char* c) { return u8isbasespacei(u8tointc(c)); }
inline int u8isbasespacecc(const char* c) { return u8isbasespacei(u8tointcc(c)); }
int u8isspacei(int n) {
  static int spaces[] = {0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x20, 0xA0, 0x1680, 0x180E, 0x2000, 0x202F, 0x205F, 0x3000, 0xFEFF};
  return in2(n,spaces,sizeof(spaces)/sizeof(int))||in(n,0x2000,0x200B);
}
inline int u8isspacec(char* c) { return u8isspacei(u8tointc(c)); }
inline int u8isspacecc(const char* c) { return u8isspacei(u8tointcc(c)); }

#ifdef __cplusplus
int u8isdigit(int n) { return u8isdigiti(n); }
int u8isdigit(char* c) { return u8isdigitc(c); }
int u8isdigit(const char* c) { return u8isdigitcc(c); }
int u8isupperascii(int n) { return u8isupperasciii(n); }
int u8isupperascii(char* c) { return u8isupperasciic(c); }
int u8isupperascii(const char* c) { return u8isupperasciicc(c); }
int u8islowerascii(int n) { return u8islowerasciii(n); }
int u8islowerascii(char* c) { return u8islowerasciic(c); }
int u8islowerascii(const char* c) { return u8islowerasciicc(c); }
int u8isasciiletter(int n) { return u8isasciiletteri(n); }
int u8isasciiletter(char* c) { return u8isasciiletterc(c); }
int u8isasciiletter(const char* c) { return u8isasciilettercc(c); }
int u8isuppergreek(int n) { return u8isuppergreeki(n); }
int u8isuppergreek(char* c) { return u8isuppergreekc(c); }
int u8isuppergreek(const char* c) { return u8isuppergreekcc(c); }
int u8islowergreek(int n) { return u8islowergreeki(n); }
int u8islowergreek(char* c) { return u8islowergreekc(c); }
int u8islowergreek(const char* c) { return u8islowergreekcc(c); }
int u8isgreek(int n) { return u8isgreeki(n); }
int u8isgreek(char* c) { return u8isgreekc(c); }
int u8isgreek(const char* c) { return u8isgreekcc(c); }
int u8isbasespace(int n) { return u8isbasespacei(n); }
int u8isbasespace(char* c) { return u8isbasespacec(c); }
int u8isbasespace(const char* c) { return u8isbasespacecc(c); }
int u8isspace(int n) { return u8isspacei(n); }
int u8isspace(char* c) { return u8isspacec(c); }
int u8isspace(const char* c) { return u8isspacecc(c); }
int u8islowerlatin(int n) { return u8islowerlatini(n); }
int u8islowerlatin(char* c) { return u8islowerlatinc(c); }
int u8islowerlatin(const char* c) { return u8islowerlatincc(c); }
int u8isupperlatin(int n) { return u8isupperlatini(n); }
int u8isupperlatin(char* c) { return u8isupperlatinc(c); }
int u8isupperlatin(const char* c) { return u8isupperlatincc(c); }
int u8islatin(int n) { return u8islatini(n); }
int u8islatin(char* c) { return u8islatinc(c); }
int u8islatin(const char* c) { return u8islatincc(c); }
int u8isbasesign(int n) { return u8isbasesigni(n); }
int u8isbasesign(char* c) { return u8isbasesignc(c); }
int u8isbasesign(const char* c) { return u8isbasesigncc(c); }
int u8islatinsign(int n) { return u8islatinsigni(n); }
int u8islatinsign(char* c) { return u8islatinsignc(c); }
int u8islatinsign(const char* c) { return u8islatinsigncc(c); }
int u8issign(int n) { return u8issigni(n); }
int u8issign(char* c) { return u8issignc(c); }
int u8issign(const char* c) { return u8issigncc(c); }
int u8isuppercyrilic(int n) { return u8isuppercyrilici(n); }
int u8isuppercyrilic(char* c) { return u8isuppercyrilicc(c); }
int u8isuppercyrilic(const char* c) { return u8isuppercyriliccc(c); }
int u8islowercyrilic(int n) { return u8islowercyrilici(n); }
int u8islowercyrilic(char* c) { return u8islowercyrilicc(c); }
int u8islowercyrilic(const char* c) { return u8islowercyriliccc(c); }
int u8iscyrilic(int n) { return u8iscyrilici(n); }
int u8iscyrilic(char* c) { return u8iscyrilicc(c); }
int u8iscyrilic(const char* c) { return u8iscyriliccc(c); }
int u8islower(int n) { return u8isloweri(n); }
int u8islower(char* c) { return u8islowerc(c); }
int u8islower(const char* c) { return u8islowercc(c); }
int u8isupper(int n) { return u8isupperi(n); }
int u8isupper(char* c) { return u8isupperc(c); }
int u8isupper(const char* c) { return u8isuppercc(c); }
#endif

const char* u8input;
static int u8scanf_s(const char* input, char* arg, const char* set, int maxlen, int nostore, const int* app, int(*fn)(int), int(*FN)(const char*)) {
  int pos = 0;
  u8input = input;
  int posset = app[0]!='^';
  if(!nostore && arg!=NULL) arg[0] = 0;
  int a=strchr(set,'a')!=NULL, A=strchr(set,'A')!=NULL,
    b=strchr(set,'b')!=NULL, B=strchr(set,'B')!=NULL,
    l=strchr(set,'l')!=NULL, L=strchr(set,'L')!=NULL,
    g=strchr(set,'g')!=NULL, G=strchr(set,'G')!=NULL,
    c=strchr(set,'c')!=NULL, C=strchr(set,'C')!=NULL,
    f=strchr(set,'f')!=NULL, F=strchr(set,'F')!=NULL,
    s=strchr(set,'s')!=NULL, S=strchr(set,'S')!=NULL,
    w=strchr(set,'w')!=NULL, W=strchr(set,'w')!=NULL,
    u=strchr(set,'u')!=NULL, d=strchr(set,'d')!=NULL;
  while(pos<maxlen) {
    if(*input==0) break;
    int chr = u8tointcc(input);
    const char* input1 = u8shiftcc(input,1);
    if(app[0]>0) for(int i=0; app[i]!=0; ++i) {
      if(i==0 && !posset) continue;
      if(posset && chr==app[i]) goto add;
      if(!posset && chr==app[i]) goto noadd;
    }
    if(u) goto add;
    if(d&&u8isdigiti(chr)) goto add;
    if((a||b)&&u8islowerasciii(chr)) goto add;
    if((A||B)&&u8isupperasciii(chr)) goto add;
    if((a||l)&&u8islowerlatini(chr)) goto add;
    if((A||L)&&u8isupperlatini(chr)) goto add;
    if((a||g)&&u8islowergreeki(chr)) goto add;
    if((A||G)&&u8isuppergreeki(chr)) goto add;
    if((a||c)&&u8islowercyrilici(chr)) goto add;
    if((A||C)&&u8isuppercyrilici(chr)) goto add;
    if(s&&u8isbasesigni(chr)) goto add;
    if(S&&u8islatinsigni(chr)) goto add;
    if(w&&u8isbasespacei(chr)) goto add;
    if(w&&u8isspacei(chr)) goto add;
    if(f&&fn(chr)) goto add;
    if(F&&FN(input)) goto add;
    noadd:
    break;
    add:
    if(!nostore && arg!=NULL) {
      strncat(arg,input,input1-input);
      arg[input1-input] = 0;
    }
    pos+= input1-input;
    if(pos<maxlen) input = input1;
  }
  u8input = input;
  return pos;
}

/**
 * modifiers
 * %b, %B - lower/upper ASCII
 * %l, %L - lower/upper Latin1
 * %g, %G - lower/upper Greek
 * %c, %C - lower/upper Cyrilic
 * %a, %A - lower/upper letter (any)
 * %d - digit
 * %s - special sign (ASCII)
 * %S - special sign (Unicode)
 * %w - white-space (ASCII)
 * %W - white-space (Unicode)
 * %u - any utf-8
 * %f - custom function int fn(int)
 * %F - custom function int fn(const char*)
 * %n - get string position (in characters)
 * %N - get string position (in bytes)
 */
int u8scanf_chars;
int u8scanf_bytes;
int u8scanf(const char* input, const char* format, ...) {
  static int(*fn)(int);
  static int(*FN)(const char*);
  va_list args;
  va_start(args,format);
  const char* start = input;
  int charspassed = 0;
  fn = NULL;
  int nostore = 0;
  int inchar = 0, fchar = 0;
  int result = 0;
  do {
    inchar = u8tointcc(input);
    fchar = u8tointcc(format);
    if(fchar==0) break;
    if(u8isspacei(fchar)) {
      while(u8isspacecc(input)) input = u8shiftcc(input,1);
      format = u8shiftcc(format,1);
      continue;
    }
    if(fchar=='%') {
      format = u8shiftcc(format,1);
      int fchar1 = u8tointcc(format);
      int maxlen = 0;
      if(fchar1=='*') {
        nostore = 1;
        format = u8shiftcc(format,1);
        fchar1 = u8tointcc(format);
      }
      else if(fchar1=='N') {
        format = u8shiftcc(format,1);
        int* arg = va_arg(args,int*);
        *arg = input - start;
        //printf("(:%s:%d,%c)\n",start,*arg,start[*arg]);
        ++result;
        continue;
      }
      else if(fchar1=='n') {
        format = u8shiftcc(format,1);
        int* arg = va_arg(args,int*);
        *arg = charspassed;
        ++result;
        continue;
      }
      if(inchar==0) break;
      while(u8isdigiti(fchar1)) {
        maxlen = 10*maxlen+(fchar1-0x30);
        format = u8shiftcc(format,1);
        fchar1 = u8tointcc(format);
      }
      if(fchar1=='f') {
         fn = va_arg(args,int(*)(int));
      }
      else if(fchar1=='F') {
         FN = va_arg(args,int(*)(const char*));
      }
      char set[10] = "u";
      if(fchar1=='(') for(int i=0;i<10;++i) {
        format = u8shiftcc(format,1);
        fchar1 = u8tointcc(format);
        if(fchar1==')') {
          format = u8shiftcc(format,1);
          fchar1 = u8tointcc(format);
          break;
        }
        set[i] = (char)fchar1;
      }
			else if(fchar1!='[') {
			  set[0] = (char)fchar1;
			  format = u8shiftcc(format,1);
			  fchar1 = u8tointcc(format);
			}
      int app[128];
      int i=0;
      if(fchar1=='[') for(;i<127;++i) {
        format = u8shiftcc(format,1);
        fchar1 = u8tointcc(format);
        if(fchar1==']') {
          format = u8shiftcc(format,1);
          fchar1 = u8tointcc(format);
          break;
        }
        app[i] = fchar1;
      }
      app[i] = 0;
      if(maxlen==0) maxlen = 8192;
      char* arg = NULL;
      if(!nostore) va_arg(args,char*);
      int charsread = u8scanf_s(input,arg,set,maxlen,nostore,app,fn,FN);
      if(charsread==0) break;
      input = u8input;
      charspassed+= charsread;
      ++result;
      continue;
    }
    notoken:
    if(inchar!=fchar) break;
    format = u8shiftcc(format,1);
    input = u8shiftcc(input,1);
    ++charspassed;
  } while(inchar>0);
  va_end(args);
	u8scanf_chars = charspassed;
	u8scanf_bytes = input-start;
  return result;
}
