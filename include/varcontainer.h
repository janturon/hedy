#ifndef VARCONTAINER_H
#define VARCONTAINER_H

#include "all.h"

class Item;
class Node;
class VarContainer;

struct VarInfo {
  char type;
  str name;
  VarContainer* context;
};

extern VarContainer* none;
extern int nan;
extern str nos;

class VarContainer {
  void replaceVars(xstr& result); // (in srhs)

protected:
  Game* g;

public:
	// get value from context (in getVar)
  VarContainer* getVC(const char* id);
  VarContainer* getObj(const char* id, VarContainer* context=NULL);
	str getStr(const char* id, VarContainer* context=NULL);
	int getInt(const char* id, VarContainer* context=NULL);

  const str id;
  char type;
	VarInfo getVar(xstr& line);

	// write only
  std::map<str,int> ints;
  std::map<str,str> strs;
  std::map<str,VarContainer*> objs;

	// get value of parsed objects
  int irhs(xstr rhs);
  str srhs(xstr rhs);

	// get value of non-parsed objects
  int findInt(const char* cline, int def=nan);
  str findStr(const char* cline, str def=nos);
  str findStrArr(xstr& line, str& def);
  VarContainer* findObj(const char* cline, VarContainer* def=none);

  VarContainer(Game* game, const str id);
	virtual str getLabel();
  virtual void parseLine(str& line, char pass);
  void parseVar(str& line, char pass);
  void dump();
};

#endif