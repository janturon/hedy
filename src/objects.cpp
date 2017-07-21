#include "all.h"
using namespace std;


/*
  Node
*/
Node::Node(Game* g, const str _id)
  : VarContainer(g,_id) {
  type = 'n';
}

str Node::getLabel() {
	auto it = strs.find("label");
	return it==strs.end() ? id : it->second;
}

void Node::dump() {
  colprintf("$BROWN %s $LIGHTGRAY",-id);
  if(paths.size()) {
    printf("(paths: ");
    for(auto const& item: paths) printf("%s, ",-item.first->id);
    printf(")");
  }
  puts("");
  VarContainer::dump();
}

Node* Node::parseSingleLine(Game* g, xstr& line, char pass) {
  char attr = ' ';
  if(line.eat("initial ")) attr = 'i';
  else if(line.eat("final ")) attr = 'f';
  else if(line.eat("extern ")) attr = 'e';
  line.eat("node ");
  str id = line.moveid();
  if(!id) throw report("Node::parseSingleLine()" E_BADSYNTAX D_NODE,-line);
  if(pass==1) {
	  Node* node = new Node(g,id);
    if(attr=='f') node->ints["final"] = 1;
    else if(attr=='i') g->objs["here"] = node;
    if(attr!='e') {
      if(str text = line.movetext()) node->strs["text"] = text;
      g->addNode(node);
  	  return node;
    }
    else {
      line.eat("from ");
      str file = line.movetext();
      return g->getExternNode(id,file);
    }
  }
  else {
    return g->getNode(id);
  }
}

void Node::parseLine(xstr& line, char pass) {
  parseVar(line,pass);
}


/*
  Path
*/
Path::Path(Game* g) : g(g) { }

void Path::parseLine(xstr& line, char pass) {
  if(pass==2) {
	  str from = line.moveid();
	  if(!from) throw report("Path::parseLine() from" E_BADSYNTAX D_NODE,-line);
		char dir = 'n';
		if(line.eat("> ")) dir = '>';
		if(line.eat("+ ")) dir = '+';
		if(dir=='n') throw report("Path::parseLine()" E_BADSYNTAX,-line);
	  str to = line.moveid();
	  if(!to) throw report("Path::parseLine() to" E_BADSYNTAX D_NODE,-line);
	  Path::add(g->getNode(from),g->getNode(to),line,dir);
	}
}

void Path::add(Node* from, Node* to, xstr& line, char dir) {
	from->paths[to] = from->id;
	if(dir=='+') to->paths[from] = to->id;
	if(str target = line.movetext()) {
		str key = to->id; key-= "target_";
		from->strs[key] = target;
	}
	if(dir=='+') if(str target = line.movetext()) {
		str key = from->id; key-= "target_";
		to->strs[key] = target;
	}
}


/*
  Item
*/
Item::Item(Game* g, const str _id)
  : VarContainer(g,_id) {
  type = 'i';
}

Item* Item::parseSingleLine(Game* g, xstr& line, char pass) {
  line.eat("item ");
  str id = line.moveid();
  if(!id) throw report("Item::parseSingleLine() to" E_BADSYNTAX D_ITEM,-line);
  if(pass==1) {
    Item* item = new Item(g,id);
    if(str text = line.movetext()) item->strs["text"] = text;
    g->addItem(item);
    return item;
  }
  else {
    return g->getItem(id);
  }
}

void Item::parseLine(xstr& line, char pass) {
  parseVar(line,pass);
}

void Item::dump() {
  colprintf("$BROWN %s $LIGHTGRAY \n",-id);
  VarContainer::dump();
}

template<class T>
Array<T>* Array<T>::parseSingleLine(Game* g, xstr& line, char pass) {
	line.eat("array %*c"); // array @, array ~, array $
	str key = line.moveid();
	return g->getArray<T>(key);
}
// explicit template instantiation
template class Array<int>;
template class Array<str>;
template class Array<xstr>;

template<>
void Array<int>::parseLine(xstr& line, char pass) {
	if(pass==2) {
		int key = line.movei();
		str val = line.movetext();
		lines.insert(make_pair(key,val));
	}
}

template<>
void Array<xstr>::parseLine(xstr& line, char pass) {
	if(pass==2) {
		xstr key = line.movetext();
		str val = line.movetext();
		lines.insert(make_pair(key,val));
	}
}

template<>
void Array<str>::parseLine(xstr& line, char pass) {
	if(pass==2) {
		str key = line.movevar();
		str val = line.movetext();
		lines.insert(make_pair(key,val));
	}
}