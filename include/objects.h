#ifndef OBJECTS_H
#define OBJECTS_H

#include "all.h"

class Item;
class Node;

template <class T>
class Array : public Parsable {
public:
	std::map<T,str> lines;
  static Array<T>* parseSingleLine(Game* g, xstr& line, char pass);
  void parseLine(xstr& line, char pass);
};

class Node : public VarContainer, public Parsable {
public:
  str text;
  std::map<Node*,str> paths;
  Node(Game* g, const str _id);
  void dump();
  static Node* parseSingleLine(Game* g, xstr& line, char pass);
  void parseLine(xstr& line, char pass);
	str getLabel();
};

class Path : public Parsable {
	Game* g;
  static void add(Node* from, Node* to, xstr& line, char dir);
public:
  Path(Game* g);
  void parseLine(xstr& line, char pass);
};

class Item : public VarContainer, public Parsable {
public:
  Item(Game* g, const str _id);
  void dump();
  static Item* parseSingleLine(Game* g, xstr& line, char pass);
  void parseLine(xstr& line, char pass);
};

#endif