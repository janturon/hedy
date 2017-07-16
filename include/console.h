#ifndef CONSOLE_H
#define CONSOLE_H

#define CONSOLE_WIDTH 80
#define CONSOLE_IO 1
#define CONSOLE_UTF 2

bool isupper(const char* ch);
const char* nextch(const char*& str);
void wprint(const char* str);
int colprintf(const char* format, ...);
void cursor(bool on);
bool console_mode(int mode);
int getkey();
int getchs(const char* select);
void error(const char* msg, ...);
const char* report(const char* msg, ...);
void clear();
char* getDir(const char* input);

extern int typei;
template<class T> int type_of(T) { static int i = ++typei; return i; }
#define type(T) ([]()->int { static T t; return type_of(t); })()

#endif