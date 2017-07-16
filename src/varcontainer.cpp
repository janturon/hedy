#include "all.h"
using namespace std;

VarContainer* none; // is created in game constructor
int nan = -1000000000;

VarContainer::VarContainer(Game* game, const str _id) : id(_id) {
  g = game;
  type = 'x';
}

void VarContainer::parseLine(str& line, char pass) {
  // empty for game, overriden in Item and Node
}


VarContainer* VarContainer::getVC(const char* id) {
	auto vcit = g->objects.find(id);
  if(vcit!=g->objects.end()) return vcit->second;
	for(auto const& kv: g->actions) {
		str sid = -kv.first->id;
		if(sid==id) return kv.first;
	}
  throw report("VarContainer::getVC()" E_NOTFOUND D_VAR,id);
}
VarContainer* VarContainer::getObj(const char* id, VarContainer* context) {
	if(context==NULL) context = this;
	auto objit = context->objs.find(id);
  if(objit!=context->objs.end()) return objit->second;
  throw report("VarContainer::getObj()" E_NOTFOUND D_VAR,id);
}
str VarContainer::getStr(const char* id, VarContainer* context) {
	if(context==NULL) context = this;
	auto strit = context->strs.find(id);
  if(strit!=context->strs.end()) return strit->second;
  throw report("VarContainer::getStr()" E_NOTFOUND D_VAR,id);
}
int VarContainer::getInt(const char* id, VarContainer* context) {
	if(context==NULL) context = this;
	auto intit = context->ints.find(id);
  if(intit!=context->ints.end()) return intit->second;
  throw report("VarContainer::getInt()" E_NOTFOUND D_VAR,id);
}
// mynode, myitem, mymod
// mynode.@myint, mynode.~mystr, mynode.$myobj
// @myint, ~mystr, $myobj
// @.myint, ~.mystr, $.myobj
// $myobj.@myint, $myobj.~mystr, $myobj.$otherobj
VarInfo VarContainer::getVar(xstr& line) {
	VarInfo vi;
	xstr var = line.movevar();
	vi.context = g;
	if(var[0]=='.') vi.context = this, var.movechar(); // local
	vi.type = var[0];
	if(vi.type!='$' && vi.type!='~' && vi.type!='@') vi.type = '^';
	else var.movechar();
  if(var.indexOf('.',var.position)>-1 && vi.type=='^') {
    str id = var.moveid(); var.markShift(1); // skip dot
		vi.context = getVC(id);
		vi.type = var.movechar();
		if(vi.type!='$' && vi.type!='~' && vi.type!='@') {
			throw report("VarContainer::getVar()" E_BADSYNTAX D_VAR,-var);
		}
	}
  while(var.indexOf('.',var.position)>-1 && vi.type=='$') {
    str id = var.moveid(); var.markShift(1); // skip dot
    vi.context = getObj(id,vi.context);
		vi.type = var.movechar();
    if(vi.type!='$' && vi.type!='~' && vi.type!='@') {
			throw report("VarContainer::getVar()" E_BADSYNTAX D_VAR,-var);
		}
  }
  vi.name = var.moveid();
	return vi;
}

int VarContainer::findInt(const char* cline, int def) {
	xstr line = cline;
	VarInfo vi = getVar(line);
	if(vi.type!='@') throw report("VarContainer::findInt()" E_MISMATCH D_VAR,cline);
	auto it = vi.context->ints.find(vi.name);
  if(it!=vi.context->ints.end()) return it->second;
	if(def!=nan) return def;
  throw report("VarContainer::findInt()" E_NOTFOUND D_VAR,cline);
}

str VarContainer::findStr(const char* cline, str def) {
	xstr line = cline;
  if(str text = line.movetext()) return text;
	VarInfo vi = getVar(line);
	try {
		if(vi.type=='~') return getStr(vi.name,vi.context);
    else throw report("VarContainer::findStr()" E_MISMATCH D_VAR, cline);
	}
	catch(const char*& ex) {
		if(def!="") return def;
		else throw ex;
	}
}

VarContainer* VarContainer::findObj(const char* cline, VarContainer* def) {
	xstr line = cline;
	VarInfo vi = getVar(line);
	try {
		if(vi.type=='^') return getVC(vi.name);
		else if(vi.type=='$') return getObj(vi.name,vi.context);
    else throw report("VarContainer::findObj()" E_MISMATCH);
	}
	catch(const char*& ex) {
		if(def!=NULL) return def;
		else throw ex;
	}
}

void VarContainer::parseVar(str& line, char pass) {
	// first pass = context (items and nodes) added
  if(pass==2) {
		line.trimMe();
	  char fst = line[0];
    if(line.indexOf(":=")==-1) throw report("VarContainer::parseVar()" E_BADSYNTAX D_VAR,-line);
    line.explodeMe(":=");
    xstr lhs = line(0), rhs = line(1);
    VarInfo L = getVar(lhs);
    if(L.type=='@') L.context->ints[L.name] = irhs(rhs);
    else if(L.type=='~') L.context->strs[L.name] = findStr(rhs);
    else if(L.type=='$') L.context->objs[L.name] = findObj(rhs);
  }
}

int VarContainer::irhs(xstr rhs) {
  int result = 0;
  char op = ' ';
  rhs.trimMe();
  do {
    int part = rhs.movei("%d",nan);
    if(part!=nan); // do nothing
    else if(rhs.eat("+")) op = '+';
    else if(rhs.eat("-")) op = '-';
    else {
			VarInfo vi = getVar(rhs);
			part = getInt(vi.name,vi.context);
		}
    if(part!=nan) switch(op) {
      case ' ': result = part; break;
      case '+': result+= part; break;
      case '-': result-= part; break;
    }
  } while(rhs.position<rhs.length);
  return result;
}

str VarContainer::srhs(xstr rhs) {
  rhs.trimMe();
  if(rhs[0]=='"') { xstr x = rhs.movetext(); rhs = x; }
  str parts = rhs.explode("|");
  size_t numparts = parts.tokens.size();
  int index = rand()%numparts;
  xstr result = parts(index);
  replaceVars(result);
  return result;
}

void VarContainer::replaceVars(xstr& result) {
	if(result[0]!=0 && result[result.length-1]=='}') result+= " ";
  for(int pos=result.indexOf('{'); pos!=-1; pos=result.indexOf('{',pos+1)) {
    result.markTo(pos+1);
 		xstr var = result.movevar();
		if(result[0]!='}') continue;
    VarInfo vi = getVar(var);
    str val;
    if(vi.type=='@') val = g->getInt(vi.name,vi.context);
		else if(vi.type=='$') val = g->getObj(vi.name,vi.context)->id;
    else if(vi.type=='~') val = g->getStr(vi.name,vi.context);
		var-= "{";
		var+= "}";
    result.replaceMe(-var,-val);
  }
}

str VarContainer::getLabel() {
	auto it = strs.find("label");
	if(it==strs.end()) it = strs.find("text");
	return it==strs.end() ? id : it->second;
}

void VarContainer::dump() {
  if(ints.size()) {
    printf("--ints: ");
    for(auto const& item: ints) printf("%s->%d, ",-item.first,item.second);
    puts("");
  }
  if(strs.size()) {
    printf("--strs: ");
    for(auto const& item: strs) printf("%s->%s, ",-item.first,-item.second);
    puts("");
  }
  if(objs.size()) {
    printf("--objs: ");
    for(auto const& item: objs) {
      if(item.second!=NULL) printf("%s->%s, ",-item.first,-item.second->id);
      else printf("%s->%s, ",-item.first,"NULL");
    }
    puts("");
  }
}
