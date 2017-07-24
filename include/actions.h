#ifndef ACTIONS_H
#define ACTIONS_H

#include "all.h"

template<class T>
struct VCLess {
  bool operator()(T* a, T* b) { return a->id < b->id; }
};

template <class T> class Array;

class Mod : public VarContainer, public Parsable {
  bool noif;
  VarContainer* ifcontext; // for filters
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
	bool evalEquation(xstr s);
	bool varOp(int lval, char op, int rval);
	bool varOp(str& lval, char op, str& rval);

	void executeLine(xstr& line);
  void message(xstr& cmd, bool strict=false);
  void text(xstr& cmd);
	void showdump(xstr& line);

	// select, foreach
  std::map<VarContainer*,str, VCLess<VarContainer> > select;
	void initData(xstr& cmd);
	void doFilter(xstr& cmd);
	void doLoop(xstr& cmd, char type);
	VarContainer* doPickOne(xstr& cmd, VarInfo& vi);
  VarContainer* doPickInt(char fn, str& varName);
	void initDataArr(xstr& cmd);
	bool doLoopArr(xstr cmd, VarInfo& vi, char type);
  template<class T> T empty();
  template<class T> void doFilterArr(xstr& cmd, VarInfo& vi, Array<T> array, char type); // works with Array<T> copy intentionally
  template<class T> void doFilterArrKey(xstr& filter, Array<T>& array);
  template<class T> void doFilterArrVal(xstr& filter, Array<T>& array);
  template<class T> bool doFilterArrErase(T& lhs, xstr filter);
  template<class T> T doPickOneArr(xstr& cmd, Array<T>& array);
	template<class T> bool doPickAskSkip(T);

public:
  Mod(Game* g, const str _id);
  static Mod* parseSingleLine(Game* g, xstr& line, char pass);
  void parseLine(xstr& line, char pass);
	template<class T, class U=std::less<T> >
    T doPickAsk(std::map<T,str,U>& src, T def, bool must=false);
  void run();
	void dump();
  void dumpMore();
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
void Mod::doFilterArr(xstr& cmd, VarInfo& vi, Array<T> array, char type) {
	if(array.lines.size()==0) return;
	while(xstr filter=cmd.moves("filter(%63[^)])")) {
    bool byval = filter.eat("~val");
		if(type(T)==type(int)) filter.eat("@key");
		else if(type(T)==type(str)) filter.eat("~key");
		else if(type(T)==type(VarContainer*)) filter.eat("$key");
		else if(!byval) throw report("Action::doFilterArr()" E_BADSYNTAX);
    if(byval) doFilterArrVal(filter,array);
    else doFilterArrKey(filter,array);
	}
  if(type=='s') {
		T result = doPickOneArr(cmd,array);
		if(result!=empty<T>()) *getPtr<T>(vi) = result;
	}
	else if(type=='f') for(auto const& kv: array.lines) {
		*getPtr<T>(vi) = kv.first;
		Mod::executeLine(cmd);
	}
}

template<class T>
void Mod::doFilterArrVal(xstr& filter, Array<T>& array) {
	char op = filter.movechar();
  xstr rhs = filter.movetext();
  for(auto it=array.lines.cbegin(); it!=array.lines.cend() ;) {
    xstr lhs = -it->second;
    bool erase = !varOp(lhs,op,rhs);
		if(erase) it = array.lines.erase(it);
    else it++;
	}
}

template<class T>
void Mod::doFilterArrKey(xstr& filter, Array<T>& array) {
  for(auto it=array.lines.cbegin(); it!=array.lines.cend() ;) {
		T lhs = it->first;
		if(doFilterArrErase(lhs,filter)) it = array.lines.erase(it);
    else it++;
	}
}


template<class T, class U>
T Mod::doPickAsk(std::map<T,str,U>& src, T def, bool must) {
	std::map<char,T> keys;
	char letters[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	letters[0] = must ? 1 : -5; // -5=ESC, 0=end of string
	keys[-5] = def;
	size_t i = 1;
	for(auto const& kv: src) {
		if(doPickAskSkip(kv.first)) continue;
		char letter = letters[i];
		xstr word = -kv.second;
    const char* space = word.indexOf("\\n")>-1 ? "" : "  ";
		keys.insert(std::make_pair(letter,kv.first));
		colprintf("$LIGHTGREEN %c$LIGHTGRAY : %s%s",letter,-srhs(word),space);
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

template<class T>
T Mod::doPickOneArr(xstr& cmd, Array<T>& array) {
	if(array.lines.size()==0) return empty<T>();
  auto fst = array.lines.begin();
	T result = fst->first;
  if(type(T)==type(int)) {
  	if(cmd.eat("min( @.key )")) {
  		for(auto const& kv: array.lines) if(kv.first<result) result = kv.first;
  	}
  	else if(cmd.eat("min( @.key )")) {
  		for(auto const& kv: array.lines) if(result<kv.first) result = kv.first;
  	}
  }
	if(xstr question=cmd.moves("ask!( \"%[^\"]\" )")) {
		puts(question);
		puts("");
		result = doPickAsk<T>(array.lines,empty<T>(),true);
	}
	else if(xstr question=cmd.moves("ask( \"%63[^\"]\" )")) {
		puts(question);
		puts("");
		result = doPickAsk<T>(array.lines,empty<T>());
	}
	else { // pick a random item
		auto rnd = array.lines.begin();
    int index = rand()%array.lines.size();
		advance(rnd,index);
		return rnd->first;
	}
	return result;
}

class Action : public Mod {
public:
	Action();
	Action(Game* g, const str _id);
  static Action* parseSingleLine(Game* g, xstr& line, char pass);
};

#endif