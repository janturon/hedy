#ifndef PARSER_H
#define PARSER_H

#include "all.h"

#define E_ERROR "Chyba na řádku %d"
#define E_PASS "(fáze %d)"
#define E_GAMEMISSING "Nezadal jsi hru ke spuštění.\nPřetáhni .txg soubor na lamarr.exe"
#define E_GAMENOTFOUND "Hra '%s' nenalezena.\nJestli soubor fakt existuje, zkus ho spustit odjinud."

#define E_NOTFOUND " Nenalezeno: "
#define E_MISMATCH " Nekompatibilita: "
#define E_BADSYNTAX " Syntaktická chyba: "
#define E_REPEATED " Opakovaná definice: "

#define D_VAR "proměnná '%s'"
#define D_NODE "uzel '%s'"
#define D_ITEM "věc '%s'"
#define D_MOD "mod '%s'"
#define D_ACTION "akce '%s'"
#define D_MACRO "makro '%s'"

#define G_GO "Jít"
#define G_WHEREGO "Kam chceš jít?"
#define G_WAIT "Stiskni klávesu..."
#define G_GAMEOVER "Game over!"

#define R_HERE "Nemohu najít uzel, ve kterém bude hra pokračovat."

class xstr : public str {
public:
  using str::str;
  xstr();
  xstr(str&& rhs);
  xstr(const str rhs);
  const char* moveid();
  const char* movevar();
  const char* movetext();
	char movechar();
};

class Game;
class Node;

struct Parsable {
  virtual void parseLine(xstr& line, char pass)=0;
};

class Parser {
  static std::vector<str> loaded;
  static char* base;
  bool blocked;

  FILE* fd;
  Game* g;
  xstr line;
  int lineNumber;
	char pass;

  void skipBOM();
  void trace();

  void parseLines(xstr& line, char pass, Parsable* src, bool(*stop)(xstr&));
  void parseSectionLines(xstr& line, char pass);
  void skipMultilineComment(str& line);
	void joinMultiline(str& line);
public:
  Parser(const char* file, Game* game);
  void parse();
};

#endif