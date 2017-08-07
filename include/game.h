#ifndef GAME_H
#define GAME_H

#include "all.h"

class Saver {
  Game* g;
  void addStr(char*& to, const char* from, int len);
public:
  Saver(Game* g);
  static char buffer[8192];
  std::map<str,char> cmds;
  std::map<str,str> vars;
  void save();
  void load();
};

struct Game : VarContainer {
  Saver saver;
	Path* path;
	std::map<Action*,str,VCLess<Action> > actions;
  std::map<str,VarContainer*> objects;
  std::map<std::pair<VarContainer*,str>,str> imods;
	std::map<str,str> macros;
	std::map<str,Array<int>*> iarrays;
	std::map<str,Array<str>*> sarrays;
	std::map<str,Array<VarContainer*>*> oarrays;
  str intro;
  xstr addtext;
  int modcnt; // counter for text mods id
  bool loading;

  Game();
  Item* getItem(str key);
  Node* getNode(str key);
  Node* getExternNode(str key, str& file);
  Mod* getMod(str key, VarContainer* context=NULL);
  Action* getAction(str key);
  str& getMacro(str key);
	template <class T> Array<T>* getArray(str& key);
  void addNode(Node* node);
  void addItem(Item* node);
  void addAction(Action* action);
  void addMod(Mod* mod);
  void addIfMod(Mod* owner, str& cond);
  Mod* getIfMod(Mod* owner, str& cond);
  Mod* textMod(xstr& text);
  Mod* fromMod(xstr& text);
  static void parseSingleLine(Game* g, xstr& line, char pass);
  void parseIntro(xstr& line, char pass);
  void parseMacro(xstr& line, char pass);
  void dump();
  void dumpMore();
};

#endif