#include "all.h"
using namespace std;

/*
  Game
*/
Game::Game() : VarContainer(this,"game") {
	none = new VarContainer(this,"null");
  objects.insert(make_pair("null",none));
  strs.insert(make_pair("statusbar",""));
	path = new Path(this);
  intro = "";
  addtext = "";
  modcnt = 0;
}

Item* Game::getItem(str key) {
  VarContainer* result = findObj(key,none);
  if(result==none) throw report("Game::getItem()" E_NOTFOUND D_ITEM, -key);
  if(result->type!='i') throw report("Game::getItem()" E_MISMATCH D_ITEM, -key);
  return static_cast<Item*>(result);
}

void Game::addItem(Item* item) {
  auto result = objects.insert(make_pair(item->id,item));
  if(!result.second) throw report("Game::addItem()" E_REPEATED D_ITEM,-item->id);
}

Node* Game::getNode(str key) {
  VarContainer* result = findObj(key,none);
  if(result==none) throw report("Game::getNode()" E_NOTFOUND D_NODE, -key);
  if(result->type!='n') throw report("Game::getNode()" E_MISMATCH D_NODE, -key);
  return static_cast<Node*>(result);
}

Node* Game::getExternNode(str key, str& file) {
  Parser(file,g).parse();
  return getNode(key);
}

void Game::addNode(Node* node) {
  auto result = objects.insert(make_pair(node->id,node));
  if(!result.second) throw report("Game::addNode()" E_REPEATED D_NODE,-node->id);
}

Mod* Game::getMod(str key, VarContainer* context) {
  if(context==NULL) context = this;
  VarContainer* result = context->findObj(key,none);
  if(result==none) throw report("Game::getMod()" E_NOTFOUND D_MOD, -key);
  if(result->type!='m') throw report("Game::getMod()" E_MISMATCH D_MOD, -key);
  return static_cast<Mod*>(result);
}

void Game::addMod(Mod* mod) {
  auto result = objects.insert(make_pair(mod->id,mod));
  if(!result.second) throw report("Game::addMod()" E_REPEATED D_MOD,-mod->id);
}

Mod* Game::textMod(xstr& text) {
  str key = "_tmod";
  key+= ++modcnt;
  Mod* result = new Mod(this,key);
  sprintf(str::buffer,"message \"%s\"",-text);
	xstr line = str::buffer;
	result->parseLine(line,2);
	g->addMod(result);
  return result;
}

Mod* Game::fromMod(xstr& rhs) {
  str key = "_fmod";
  key+= ++modcnt;
  Mod* result = new Mod(this,key);
  xstr mod = rhs.movevar();
  sprintf(str::buffer,"if %s=mod then run %s",-mod,-mod);
	xstr line = str::buffer;
	result->parseLine(line,2);
  if(rhs.eat("default ")) {
    line = "check noif";
  	result->parseLine(line,2);
    if(xstr text=rhs.movetext()) {
      sprintf(str::buffer,"message \"%s\"",-text);
    	line = str::buffer;
    	result->parseLine(line,2);
    }
    else if(xstr var=rhs.movevar()) {
      sprintf(str::buffer,"run %s",-var);
    	line = str::buffer;
    	result->parseLine(line,2);
    }
  }
	g->addMod(result);
  return result;
}

Action* Game::getAction(str key) {
	for(auto const& kv: actions) if(key==kv.first->id) return kv.first;
	throw report("Game::getAction()" E_NOTFOUND D_ACTION,-key);
}

void Game::addAction(Action* action) {
  auto result = objects.insert(make_pair(action->id,action));
  if(!result.second) throw report("Game::addAction()" E_REPEATED D_ACTION,-action->id);
	str label = action->getStr("text");
  actions.insert(make_pair(action,label));
}


void Game::parseSingleLine(Game* g, xstr& line, char pass) {
  if(line.indexOf(":=")>-1) g->parseVar(line,pass);
  else if(line.eat("intro ")) g->parseIntro(line,pass);
	else if(line.eat("macro ")) g->parseMacro(line,pass);
  else throw report("Game::parseSingleLine()" E_BADSYNTAX);
}

void Game::parseIntro(xstr& line, char pass) {
  if(pass==1) g->intro = line.movetext(); // may be omitted
}

void Game::parseMacro(xstr& line, char pass) {
	if(pass==1) {
		str key = line.moveid();
		line.eat(" ");
		auto result = macros.insert(make_pair(key,~line));
		if(!result.second) throw report("Game::parseMacro()" E_REPEATED D_MACRO,-key);
	}
}

str& Game::getMacro(str key) {
  auto result = macros.find(key);
  if(result==macros.end()) throw report("Game::getMacro()" E_NOTFOUND D_MACRO,-key);
	return result->second;
}

template <>
Array<int>* Game::getArray(str& key) {
  auto result = iarrays.find(key);
	if(result!=iarrays.end()) return result->second;
	auto val = new Array<int>();
	iarrays.insert(make_pair(key,val));
	return val;
}

template <>
Array<xstr>* Game::getArray(str& key) {
  auto result = sarrays.find(key);
	if(result!=sarrays.end()) return result->second;
	auto val = new Array<xstr>();
	sarrays.insert(make_pair(key,val));
	return val;
}

template <>
Array<str>* Game::getArray(str& key) {
  auto result = oarrays.find(key);
	if(result!=oarrays.end()) return result->second;
	auto val = new Array<str>();
	oarrays.insert(make_pair(key,val));
	return val;
}

void Game::dump() {
	if(intro) {
		colprintf("$WHITE GAME$LIGHTGRAY \n");
    printf("--intro: %.64s\n",-intro);
    VarContainer::dump();
	}
	if(macros.size()>0) {
		colprintf("$WHITE MACROS$LIGHTGRAY \n");
		for(pair<str,str> const& kv: macros) {
			printf("--%s: %s\n", -kv.first, -kv.second);
		}
	}
  VarContainer::dump();
	bool bnodes = false;
	for(pair<str,VarContainer*> const& kv: objects) {
    if(kv.second->type!='n') continue;
		if(!bnodes) colprintf("$WHITE NODES$LIGHTGRAY \n"), bnodes = true;
    Node* n = static_cast<Node*>(kv.second);
		n->dump();
	}
  bool bitems = false;
	for(pair<str,VarContainer*> const& kv: objects) {
    if(kv.second->type!='i') continue;
		if(!bitems) colprintf("$WHITE ITEMS$LIGHTGRAY \n"), bitems = true;
    Item* i = static_cast<Item*>(kv.second);
		i->dump();
	}
	colprintf("$WHITE ACTIONS$LIGHTGRAY \n");
	for(pair<Action*,str> const& kv: actions) {
		kv.first->dump();
	}
	puts("");
	bool bmods = false;
	for(pair<str,VarContainer*> const& kv: objects) {
    if(kv.second->type!='m') continue;
		if(!bmods) colprintf("$WHITE MODS$LIGHTGRAY \n"), bmods = true;
		Mod* m = static_cast<Mod*>(kv.second);
		m->dump();
	}
	if(bmods) puts("");
	if(iarrays.size()>0 || sarrays.size()>0 || oarrays.size()>0) {
		colprintf("$WHITE ARRAYS$LIGHTGRAY \n");
		for(pair<str,Array<int>*> const& kv: iarrays) {
			printf("--%s: ", -kv.first);
			Array<int>* iarray = kv.second;
			for(pair<int,str> const& ch: iarray->lines) printf("(%d=%s) ", ch.first, -ch.second);
			puts("");
		}
		for(pair<str,Array<xstr>*> const& kv: sarrays) {
			printf("--%s: ", -kv.first);
			Array<xstr>* sarray = kv.second;
			for(pair<xstr,str> const& ch: sarray->lines) printf("(%s=%s) ", -ch.first, -ch.second);
			puts("");
		}
		for(pair<str,Array<str>*> const& kv: oarrays) {
			printf("--%s: ", -kv.first);
			Array<str>* oarray = kv.second;
			for(pair<str,str> const& ch: oarray->lines) printf("(%s=%s) ", -ch.first, -ch.second);
			puts("");
		}
	}
	getkey();
}

void Game::dumpMore() {
	for(pair<Action*,str> const& kv: actions) {
		kv.first->dumpMore();
	}
	for(pair<str,VarContainer*> const& kv: objects) {
    if(kv.second->type!='m') continue;
		Mod* m = static_cast<Mod*>(kv.second);
		m->dumpMore();
	}
	getkey();
}
