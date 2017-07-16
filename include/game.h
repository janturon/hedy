#ifndef GAME_H
#define GAME_H

#include "all.h"

struct Game : VarContainer {
	Path* path;
	std::map<Action*,str> actions;
  std::map<str,VarContainer*> objects;
	std::map<str,str> macros;
	std::map<str,Array<int>*> iarrays;
	std::map<str,Array<xstr>*> sarrays;
	std::map<str,Array<str>*> oarrays;
  str intro;

  Game();
  Item* getItem(str key);
  Node* getNode(str key);
  Node* getExternNode(str key, str& file);
  Mod* getMod(str key);
  Action* getAction(str key);
  str& getMacro(str key);
	template <class T> Array<T>* getArray(str& key);
  void addNode(Node* node);
  void addItem(Item* node);
  void addMod(Mod* mod);
  void addAction(Action* action);
  static void parseSingleLine(Game* g, xstr& line, char pass);
  void parseIntro(xstr& line, char pass);
  void parseMacro(xstr& line, char pass);
  void dump(char what=' ');
};

#endif