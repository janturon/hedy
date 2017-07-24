#include "all.h"
using namespace std;

/*
  Mod
*/
Mod::Mod(Game* g, const str _id)
  : VarContainer(g,_id) {
  type = 'm';
  ifcontext = NULL;
}

Mod* Mod::parseSingleLine(Game* g, xstr& line, char pass) {
  line.eat("mod ");
  xstr id = line.movevar();
  str text = line.movetext(); // due to compatibility with other objects
  str cond = line.eat("if ") ? ~line : "";
  Mod* mod;
  if(pass==1) {
		VarInfo vi = g->getVar(id);
    bool add = true;
    if(cond) try {
      mod = g->getMod(id); add = false;
    } catch(...) { mod = new Mod(g,id); }
    else mod = new Mod(g,id);
    if(cond) g->addIfMod(mod,cond);
    mod->objs["context"] = vi.context==g ? mod : vi.context;
 		if(vi.context!=g) vi.context->objs[vi.name] = mod;
    if(text) mod->strs["text"] = text;
    if(add) g->addMod(mod);
  }
  else {
    mod = g->getMod(id);
    if(cond) mod = g->getIfMod(mod,cond);
  }
  return mod;
}

void Mod::parseLine(xstr& line, char pass) {
  if(pass==2) {
		for(auto const& kv: g->macros) {
			line.replaceMe(kv.first,kv.second);
		}
		lines.push_back(line);
	}
}

bool Mod::evalIf(str& s) {
  if(!s.eat("if ")) return true; // no condition
  int to = s.indexOf(" then");
  if(to==-1) throw report("Mod::evalIf()" E_BADSYNTAX D_VAR,-s);
  str cond = s.slice(s.position,to);
  s.markTo(to+5); // after " then"
	bool result = evalCondOr(cond);
	return result;
}

bool Mod::evalCondOr(str& cond) {
  cond.explodeMe(" or ");
  for(long i=0; i<cond.tokens.size(); ++i) {
		xstr part = cond(i);
		if(evalCondAnd(part)) return true;
	}
  return false;
}

bool Mod::evalCondAnd(str& cond) {
  cond.explodeMe(" and ");
  for(long i=0; i<cond.tokens.size(); ++i) {
		xstr part = cond(i);
		if(!evalCondPart(part)) return false;
	}
  return true;
}

bool Mod::evalCondPart(xstr& s) {
  bool result = false;
	bool inv = s.eat("not ");
  if(s.eat("luck ")) result = evalLuck(s);
  else if(s.eat("noif")) result = noif;
  else result = evalEquation(s);
  if(inv) result = !result;
  return result;
}

bool Mod::evalLuck(xstr& s) {
  bool result = false;
  xstr ss1 = s.moves(" %[^%%:]");
  int prob = irhs(ss1);
  char next = s.movechar();
  if(next=='%') result = rand()%100<prob;
  else if(next==':') {
    ss1 = s.moves();
    int max = irhs(ss1);
    result = rand()%max<prob;
  }
  return result;
}

bool Mod::evalEquation(xstr s) {
	s.trimMe();
	xstr Lvar = s.movevar();
	VarInfo L;
	try { L = getVar(Lvar); } catch(const char*& ex) { return false; }
	if(ifcontext!=NULL) L.context = ifcontext;
	s.eat(" ");	char op = s.movechar();
	bool result = false;
  if(L.type=='@') {
		int lval = getInt(L.name,L.context);
    if(lval!=nan) { int rval = irhs(~s); result = varOp(lval,op,rval); }
	}
  else if(L.type=='~') {
		str lval = getStr(L.name,L.context);
		if(lval!=nos) { str rval = srhs(~s); result = varOp(lval,op,rval); }
	}
  else if(L.type=='$') {
		if(op!='=') throw report("Mod::evalEquation()" E_BADSYNTAX D_VAR,-s);
		VarContainer* l = getObj(L.name,L.context);
		xstr id = s.movevar();
    if(l!=none) {
  		if(id=="node") result = l->type=='n';
      else if(id=="item") result = l->type=='i';
  		else if(id=="mod") result = l->type=='m';
  		else if(id=="action") result = l->type=='a';
  		else result = l==findObj(id);
    }
    else result = id=="null";
  }
	else throw report("Mod::evalEquation()?" E_BADSYNTAX D_VAR,-s);
	return result;
}

bool Mod::varOp(int lval, char op, int rval) {
	switch(op) {
		case '=':	return lval == rval;
		case '<': return lval < rval;
		case '>': return lval > rval;
		default: throw report("Mod::varOp<int>()" E_BADSYNTAX "'%c'",op);
	}
}

bool Mod::varOp(str& lval, char op, str& rval) {
	switch(op) {
		case '=': return lval == rval;
		case '>': return lval.indexOf(-rval)>-1;
		case '<': return rval.indexOf(-lval)>-1;
		default: throw report("Mod::varOp<str>()" E_BADSYNTAX);
	}
}

template<> int Mod::getKey<int>(xstr& cmd) { return cmd.movei(); }
template<> str Mod::getKey<str>(xstr& cmd) { return cmd.movetext(); }
template<> VarContainer* Mod::getKey<VarContainer*>(xstr& cmd) { return getVC(cmd.moveid()); }

template<> Array<int>* Mod::getArray(str& arr) { return g->iarrays[arr]; }
template<> Array<str>* Mod::getArray(str& arr) { return g->sarrays[arr]; }
template<> Array<VarContainer*>* Mod::getArray(str& arr) { return g->oarrays[arr]; }

template<> int Mod::empty<int>() { return nan; }
template<> str Mod::empty<str>() { return nos; }
template<> VarContainer* Mod::empty<VarContainer*>() { return none; }

template<> bool Mod::doFilterArrErase(int& lhs, xstr filter) {
	char op = filter.movechar();
  int rhs = filter.movei();
  return !varOp(lhs,op,rhs);
}
template<> bool Mod::doFilterArrErase(str& lhs, xstr filter) {
	char op = filter.movechar();
  str rhs=filter.movetext();
  return !varOp(lhs,op,rhs);
}
template<> bool Mod::doFilterArrErase(VarContainer*& lhs, xstr filter) {
	char op = filter.movechar();
  str rhs=filter.moveid();
  return g->objects[rhs] == lhs;
}

void Mod::run() {
  noif = true;
  for(auto const& cmd: lines) {
		xstr line = cmd;
    if(line.eat("check ")) {
			line.sliceMe(6); // skip "check "
      bool passed = evalCondOr(line);
		 	if(!passed) break;
		}
		executeLine(line);
	}
}

void Mod::executeLine(xstr& line) {
  bool hasif = line.startsWith("if ");
	if(!evalIf(line)) return;
  if(hasif) noif = false;
	xstr cmd = ~line;
  if(cmd.eat("message! ")) message(cmd,true);
  else if(cmd.eat("message ")) message(cmd);
  else if(cmd.eat("text ")) text(cmd);
	else if(cmd.eat("set @")) array<int>(cmd);
	else if(cmd.eat("set ~")) array<str>(cmd);
	else if(cmd.eat("set $")) array<VarContainer*>(cmd);
  else if(cmd.indexOf(":=")>-1) parseVar(cmd,2);
	else if(cmd.eat("?")) showdump(cmd);
	else if(cmd.eat("path ")) g->path->parseLine(cmd,2);
	else if(cmd.eat("select ")) doLoop(cmd,'s');
	else if(cmd.eat("foreach ")) doLoop(cmd,'f');
	else if(cmd.eat("run ")) {
		xstr id = cmd.movevar();
		try {
			Mod* nextMod = g->getMod(id,this);
			nextMod->run();
		}
		catch(const char*& ex) {
			error(ex);
			getkey();
		}
	}
}

void Mod::showdump(xstr& line) {
  clear();
	if(line.eat("vars")) g->dump();
	if(line.eat("commands")) g->dumpMore();
  clear();
}

void Mod::message(xstr& cmd, bool strict) {
	xstr msg = cmd.movetext();
	if(strict) clear(); else puts("");
  wprint(srhs(msg));
  if(strict) colprintf("\n\n$GREEN %s $LIGHTGRAY", G_WAIT);
	getkey();
  puts("");
}

void Mod::text(xstr& cmd) {
  sprintf(str::buffer,"%s %s",-g->addtext,-srhs(cmd));
  g->addtext = str::buffer;
}

void Mod::dump() {
  colprintf("$BROWN %s $LIGHTGRAY \n",-id);
  VarContainer::dump();
}

void Mod::dumpMore() {
	colprintf("$WHITE (%s) $LIGHTGRAY \n",-id);
	for(auto const& i: lines) printf("  %s\n",-i);
}

template<class T> bool Mod::doPickAskSkip(T) { return false; }
template<> bool Mod::doPickAskSkip(Action* action) { return action->findInt(".@hide",0)==1; }

void Mod::doLoop(xstr& cmd, char type) {
	select.clear();
	VarInfo vi = getVar(cmd);
  cmd.eat("from ");
  if(doLoopArr(cmd,vi,type)) return;
	initData(cmd);
  doFilter(cmd);
 	if(type=='s') {
  	auto objit = vi.context->objs.find(vi.name);
    if(objit==vi.context->objs.end()) vi.context->objs[vi.name] = none;
		VarContainer* result = doPickOne(cmd,vi);
		if(result!=none) vi.context->objs[vi.name] = result;
	}
	else if(type=='f') {
		xstr subcmd = ~cmd;
		for(auto const& kv: select) {
			vi.context->objs[vi.name] = kv.first;
			Mod::executeLine(subcmd);
		}
	}
}

bool Mod::doLoopArr(xstr cmd, VarInfo& vi, char type) {
  bool result = true;
  cmd.eat(" ");
	char vitype = cmd.movechar();
	if(vitype!=vi.type) return false;
	str arrid = cmd.moveid();
	if(vitype=='@') doFilterArr<int>(cmd,vi,*g->getArray<int>(arrid),type);
	else if(vitype=='~') doFilterArr<str>(cmd,vi,*g->getArray<str>(arrid),type);
	else if(vitype=='$') doFilterArr<VarContainer*>(cmd,vi,*g->getArray<VarContainer*>(arrid),type);
	else result = false;
  return result;
}

void Mod::initData(xstr& cmd) {
	if(cmd.eat("items ")) for(auto const& kv: g->objects) {
		VarContainer* vc = kv.second;
		if(vc->type=='i') select.insert(make_pair(vc,vc->getLabel()));
	}
	else if(cmd.eat("nodes ")) for(auto const& kv: g->objects) {
		VarContainer* vc = kv.second;
		if(vc->type=='n') select.insert(make_pair(vc,vc->getLabel()));
	}
	else if(cmd.eat("paths ")) {
		Node* here = (Node*)g->objs["here"];
		for(auto const& kv: here->paths) {
			VarContainer* dest = kv.first;
			str key = dest->id; key-= "target_";
      if(!here->strs[key]) here->strs[key] = dest->id;
			select.insert(make_pair(dest,here->strs[key]));
		}
	}
	else if(cmd.movechar()=='$') initDataArr(cmd);
	else throw report("Mod::initData()" E_BADSYNTAX D_VAR,~cmd);
}

void Mod::initDataArr(xstr& cmd) {
	str arrid = cmd.moveid();
	Array<str>* array = g->getArray<str>(arrid);
	for(auto const& kv: array->lines) {
		xstr key = -kv.first;
		VarInfo vi = getVar(key);
		VarContainer* dest = vi.context;
		if(vi.name!=dest->id) {
			auto it = dest->objs.find(vi.name);
		  if(it!=dest->objs.end()) dest = it->second;
			else throw report("Mod::initDataArr()" E_BADSYNTAX);
		}
		select.insert(make_pair(dest,kv.second));
	}
}

void Mod::doFilter(xstr& cmd) {
  int i=0;
	while(xstr filter=cmd.moves("filter(%63[^)]) ")) {
    for(auto it=select.cbegin(); it!=select.cend(); ) {
      ifcontext = it->first;
      xstr stepfilter = -filter;
			if(!evalCondOr(stepfilter)) it = select.erase(it);
      else it++;
		}
	}
  ifcontext = NULL;
}

VarContainer* Mod::doPickOne(xstr& cmd, VarInfo& vi) {
	if(select.size()==0) return none;
	xstr brackets;
	char fn = ' ';
	if(brackets=cmd.moves("min( %[^)])")) fn = '<';
	else if(brackets=cmd.moves("max( %[^)])")) fn = '>';
	else if(brackets=cmd.moves("ask( \"%[^\"]\" )")) fn = '?';
	else if(brackets=cmd.moves("ask!( \"%[^\"]\" )")) fn = '!';
	if(fn=='<' || fn=='>') {
		xstr var = brackets.movevar();
		VarInfo L = getVar(var);
		if(L.type!='@') throw report("Mod::doPickOne()" E_MISMATCH D_VAR,-var);
		return doPickInt(fn,L.name);
	}
	else if(fn=='?' || fn=='!') {
		puts("");
		wprint(brackets);
		puts("\n");
    bool must = fn=='!';
		VarContainer* result = doPickAsk<VarContainer*,VCLess<VarContainer> >(select,none,must);
		return result;
	}
	else { // pick a random item
		auto rnd = select.begin();
    int index = rand()%select.size();
		advance(rnd,index);
		return rnd->first;
	}
}

VarContainer* Mod::doPickInt(char fn, str& varName) {
	int* best = NULL;
	VarContainer* result = none;
	for(auto const& kv: select) {
		VarContainer* key = kv.first;
		map<str,int>* pool = &(kv.first->ints);
		auto xval = pool->find(varName);
		if(xval!=pool->end()) {
			int* value = &(xval->second);
			if(best==NULL) result = key, best = value;
			else if(fn=='<' && *value<*best) result = key, best = value;
			else if(fn=='>' && *value>*best) result = key, best = value;
		}
	}
	return result;
}


/*
	Action
*/
Action::Action(Game* g, const str _id) : Mod(g,_id) {
	type = 'a';
	ints["hide"] = 0;
}
Action::Action() : Mod(NULL,"") { }

Action* Action::parseSingleLine(Game* g, xstr& line, char pass) {
  line.eat("action ");
  str id = line.moveid();
  if(!id) throw report("Action::parseSingleLine()" E_BADSYNTAX);
  if(pass==1) {
    Action* action = new Action(g,id);
    if(str text = line.movetext()) action->strs["text"] = text;
		else action->strs["text"] = id;
    g->addAction(action);
    return action;
  }
  else {
    return g->getAction(id);
  }
}
