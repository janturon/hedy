#ifndef ACTIONS_H
#define ACTIONS_H

#include "all.h"

template <class T> class Array;

class Mod : public VarContainer, public Parsable {
protected:
  std::vector<xstr> lines;

	// arrays
	template<class T> T getKey(xstr& cmd);
	template<class T> Array<T>* getArray(str& arr);
	template<class T> void array(xstr& s);

	// conditions
	bool evalIf(str& s);
	bool evalCondOr(str& cond);
	bool evalCondAnd(str& cond);
	bool evalCondPart(xstr& s);
  bool evalLuck(xstr& s);
	bool evalEquation(VarContainer* context, xstr s);
	bool varOp(int lval, char op, int rval);
	bool varOp(str& lval, char op, str& rval);

	void executeLine(xstr& line);
  void message(xstr& cmd);
	void showdump(xstr& line);

	// select, foreach
  std::map<VarContainer*,str> select;
	void initData(xstr& cmd);
	void doFilter(xstr& cmd);
	void doLoop(xstr& cmd, char type);
	VarContainer* doPickOne(xstr& cmd, VarInfo& vi);
  VarContainer* doPickInt(char fn, str& varName);
	void initDataArr(xstr& cmd);
	void doLoopArr(xstr& cmd, VarInfo& vi, char type);
	template<class T> void doFilterArr(xstr& cmd, Array<T>& array);
  int doPickOneArr(xstr& cmd, Array<int>& array);
	xstr doPickOneArr(xstr& cmd, Array<xstr>& array);
	template<class T> bool doPickAskSkip(T);

public:
  Mod(Game* g, const str _id);
  static Mod* parseSingleLine(Game* g, xstr& line, char pass);
  void parseLine(xstr& line, char pass);
	template<class T> T doPickAsk(std::map<T,str>& src, T def, bool must=false);
  void run();
	void dump();
};

template<class T>
void Mod::array(xstr& cmd) {
	str arr = cmd.moveid();
	cmd.eat(" ");
	char op = cmd.movechar();
	if(op!='+' && op!='-') throw report("Mod::arraySingleLine()" E_BADSYNTAX);
	T key = getKey<T>(cmd);
	str val = cmd.movetext();
	Array<T>* a = getArray<T>(arr);
	if(op=='+') a->lines[key] = val;
	if(op=='-') a->lines.erase(key);
}

template<class T>
void Mod::doFilterArr(xstr& cmd, Array<T>& array) {
	if(array.lines.size()==0) return;
	while(xstr filter=cmd.moves("filter(%63[^)])")) {
		if(type(T)==type(int)) filter.eat("@.key");
		else if(type(T)==type(xstr)) filter.eat("~.key");
		else throw report("Action::doFilterArr()" E_BADSYNTAX);
		char op = filter.movechar();
		for(auto const& kv: array.lines) {
			T lhs = kv.first;
			bool erase = false;
			if(type(T)==type(int)) int rhs=filter.movei(), erase = !varOp(lhs,op,rhs);
			else if(type(T)==type(xstr)) xstr rhs=filter.movetext(), erase = !varOp(lhs,op,rhs);
      if(erase) array.lines.erase(lhs);
		}
	}
}

template<class T>
T Mod::doPickAsk(std::map<T,str>& src, T def, bool must) {
	std::map<char,T> keys;
	char letters[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	letters[0] = must ? 1 : -5; // -5=ESC, 0=end of string
	keys[-5] = def;
	size_t i = 1;
	for(auto const& kv: src) {
		if(doPickAskSkip(kv.first)) continue;
		char letter = letters[i];
		xstr word = -kv.second;
		keys.insert(std::make_pair(letter,kv.first));
		colprintf("$LIGHTGREEN %c$LIGHTGRAY : %s  ",letter,-srhs(word));
		++i;
	}
	puts("");
	letters[i] = 0;
  int letter;
	do {
		letter = getkey()+'A'-'a';
		for(const char* c=letters; *c!=0; ++c) if(*c==letter) goto CHOSEN;
	} while(1);
	CHOSEN:
	return keys[letter];
}


class Action : public Mod {
public:
	Action();
	Action(Game* g, const str _id);
  static Action* parseSingleLine(Game* g, xstr& line, char pass);
};

#endif