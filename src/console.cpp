#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <locale.h>
#include <windows.h>
#include <conio2.h>
#include "console.h"
#include "u8char.h"

void wprint(const char* str) {
	int pos = 0;
	bool col = false;
  bool apos = false, printapos = false;
	do {
		const char* p = str;
		if(*p=='[') {
			char q = *(p+1);
			if(q=='L') textcolor(15);
			else if(q=='D') textcolor(8);
			else if(q=='B') textcolor(3);
			else if(q=='Y') textcolor(6);
			else goto NOCOLOR;
			col = true;
			str+= 2;
			continue;
			NOCOLOR:;
		}
		if(*p==']' && col==true) {
			col = false;
			textcolor(7);
			str+= 1;
			continue;
		}
		int wlen = 0; // word length in characters
		while(*p!=0 && *p!=' ' && *p!='\n' && *p!='[' && *p!=']' && *p!='\'') p=u8shiftcc(p,1), wlen++;
		int blen = p - str; // word length in bytes
    if(pos+wlen>=CONSOLE_WIDTH) { // word overflow, start new line
			printf("\n");
			pos = 0;
		}
		if(wlen>0) printf("%.*s",blen,str);
		pos+= wlen;
		if(pos==CONSOLE_WIDTH-1 && *p!=0) putchar(' '), ++p, pos=0; // skip space and newline at the end of line
		else if(*p==' ') putchar(*p++), pos++;
		else if((*p==']' || *p=='[') && col==false) putchar(*p++), pos++;
    else if(*p=='\'' && !apos) textcolor(8), putchar(*p++), pos++, apos=!apos;
    else if(*p=='\'' && apos) putchar(*p++), pos++, textcolor(7), apos=!apos;
		else if(*p=='\n') putchar(*p++), pos = 0;
		str = p;
	} while(*str);
}

int colprintf(const char* format, ...) {
  static char buffer[8192];
  static const char* colors[] = { "BLACK", "BLUE", "GREEN", "CYAN", "RED", "MAGENTA", "BROWN", "LIGHTGRAY", "DARKGRAY", "LIGHTBLUE", "LIGHTGREEN", "LIGHTCYAN", "LIGHTRED", "LIGHTMAGENTA", "YELLOW", "WHITE", 0};
  va_list args;
  va_start(args,format);
  int result = vsprintf(buffer,format,args);
  va_end(args);
  for(const char* p=buffer; *p!=0; p=u8shiftcc(p,1)) {
    if(*p!='$') { printf("%s",u8charcc(p)); continue; }
    ++p; int i;
    for(i=0; i<16; ++i) if(strstr(p,colors[i])==p) {
			textcolor(i);
			p+= strlen(colors[i]);
			break;
		}
    if(i==16) putchar(*(--p));
  }
  return result;
}

void cursor(bool on) {
  CONSOLE_CURSOR_INFO nocursor = {1,on};
  SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&nocursor);
}

//  setlocale(LC_CTYPE,"Czech_Czech Republic.852");
bool console_mode(int mode) {
  static int defaultCP = 0;
  if(!defaultCP) defaultCP = GetConsoleOutputCP();
  switch(mode) {
    case CONSOLE_IO:
      if(!setlocale(LC_CTYPE,"C")) return false;
      if(!SetConsoleCP(defaultCP)) return false;
      if(!SetConsoleOutputCP(defaultCP)) return false;
      break;
    case CONSOLE_UTF:
      if(!setlocale(LC_CTYPE,"C")) return false;
      if(!SetConsoleCP(65001)) return false;
      if(!SetConsoleOutputCP(65001)) return false;
      break;
  };
  return true;
}

int getkey() {
  DWORD cnt;
  INPUT_RECORD ir;
  HANDLE conin = GetStdHandle(STD_INPUT_HANDLE);
  if(PeekConsoleInputW(conin,&ir,1,&cnt)) {
    do ReadConsoleInputW(conin,&ir,1,&cnt); while(ir.EventType!=KEY_EVENT);
  }
  FlushConsoleInputBuffer(conin);
  do ReadConsoleInputW(conin,&ir,1,&cnt); while(ir.EventType!=KEY_EVENT);
  FlushConsoleInputBuffer(conin);
  KEY_EVENT_RECORD ke = ir.Event.KeyEvent;
  int result = ke.uChar.UnicodeChar;
  if(result==0) return 1000000+ke.wVirtualKeyCode;
}

int getchs(const char* select) {
  do {
    int c = u8tolower(getkey());
    for(const char* p=select; *p!=0; p=u8shiftcc(p,1)) {
      if(u8tointcc(p)==c) return c;
    }
  } while(1);
}

void error(const char* msg, ...) {
  va_list args;
  va_start(args,msg);
  textcolor(LIGHTRED);
  puts("");
  vprintf(msg,args);
  va_end(args);
  textcolor(LIGHTGRAY);
}

const char* report(const char* msg, ...) {
  static char buffer[8192];
  va_list args;
  va_start(args,msg);
  vsprintf(buffer,msg,args);
  va_end(args);
  return buffer;
}

void clear() {
	clrscr();
}

char* getDir(const char* input) {
  const char* pos = input;
  const char* ch = input;
  while((ch=strchr(pos+1,'\\'))!=NULL) pos = ch;
  if(pos!=input) {
    char* result = new char[pos-input+1];
    strncpy(result,input,pos-input);
    result[pos-input] = 0;
    return result;
  }
  else {
    int len = GetCurrentDirectory(0,NULL);
    char* result = new char[len];
    GetCurrentDirectory(len,result);
    return result;
  }
}

int typei = 0;