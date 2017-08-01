#include "all.h"
using namespace std;

xstr::xstr() : str() { }
xstr::xstr(str&& rhs) : str(rhs) { }
xstr::xstr(str rhs) : str(rhs) { }

const char* xstr::moveid() {
  int pos1 = 0, pos2 = 0;
  if(!sscanf(cstr+position," %1[a-zA-Z_]%n",str::buffer,&pos1)) return "";
  if(!sscanf(cstr+position+pos1,"%30[a-zA-Z0-9_]%n",str::buffer+1,&pos2)) return "";
  markShift(pos1+pos2);
  return str::buffer;
}

const char* xstr::movevar() {
  int pos1 = 0, pos2 = 0;
  if(!sscanf(cstr+position," %1[a-zA-Z_.$~@]%n",str::buffer,&pos1)) return "";
  if(!sscanf(cstr+position+pos1,"%62[a-zA-Z0-9_.$~@]%n",str::buffer+1,&pos2)) return "";
  markShift(pos1+pos2);
  return str::buffer;
}

const char* xstr::movetext() {
  int pos = 0;
  if(!sscanf(cstr+position," \"%8191[^\"]\"%n",str::buffer,&pos)) return "";
  markShift(pos);
  return str::buffer;
}

char xstr::movechar() {
	char result = cstr[position];
	markShift();
	return result;
}

void Parser::skipBOM() {
  unsigned char bom[4] = {0,0,0,0};
  fscanf(fd,"%3s",bom);
  if(bom[0]==0xEF && bom[1]==0xBB && bom[2]==0xBF) return;
  else fseek(fd,0,SEEK_SET);
}

void Parser::trace() {
  error(E_ERROR " " E_PASS,lineNumber,pass);
  line.trimMe();
  error("%s\n",line.cstr);
}

vector<str> Parser::loaded;
char* Parser::base;

Parser::Parser(const char* file, Game* game) : line("",8192) {
  blocked = false;
  line = file;
  if(line.indexOf(":")==-1) {
    line = Parser::base;
    line+= "\\";
    line+= file;
  }
  for(auto const& v: Parser::loaded) {
    str s = v;
    if(s==-line) {
      blocked = true;
      break;
    }
  }
  if(loaded.size()==0) Parser::base = getDir(line);
  if(!blocked) {
    Parser::loaded.push_back(-line);
    fd = fopen(line,"r");
    if(fd==NULL) {
      error(E_GAMENOTFOUND,file);
      getchar();
      exit(1);
    }
    lineNumber = 0;
    g = game;
  }
}

void Parser::skipMultilineComment(str& line) {
  int start = line.indexOf("#|");
  if(start==-1) return;
  int startLine = lineNumber;
  str fstLine = line;
  do {
    if(startLine!=lineNumber) line.reset(), line.trimMe();
    int end = line.indexOf("|#");
    if(end!=-1) {
      if(startLine==lineNumber) {
        line.spliceMe(start,end-start+2,"");
        goto TIDY;
      }
      size_t length = fstLine.length-start;
      fstLine.spliceMe(start,length,-line+end+2);
      line = fstLine;
      goto TIDY;
    }
    ++lineNumber;
  } while(fgets(-line,8191,fd)!=NULL);
  lineNumber = startLine;
  throw report("Parser::skipMultilineComment" E_BADSYNTAX);
	TIDY:
  line.trimMe();
  if(line.length==0) {
    fgets(-line,8191,fd);
    line.reset(), line.trimMe();
  }
}

void Parser::joinMultiline(str& line) {
	if(!line.endsWith(">>>")) return;
  line.replaceMe(">>>","");
  int start = line.length;
	str addline("",8192);
  ADDNEXT:
  	long int pos = ftell(fd);
    if(fgets(-addline,8191,fd)==NULL) goto DONE;
  	addline.reset(); addline.trimMe();
  	if(addline[0]!='>') { fseek(fd,pos,SEEK_SET); goto DONE; }
    addline[0] = '\n';
    addline.replaceMe("\"","''");
  	++lineNumber;
  	line.append(-addline);
  goto ADDNEXT;
  DONE:
  line[start] = '"';
  line+= "\"";
}


void Parser::parseLines(xstr& line, char pass, Parsable* src, bool(*stop)(xstr&)) {
	long int pos = ftell(fd);
  while(fgets(-line,8191,fd)!=NULL) {
    line.reset(), line.trimMe();
    ++lineNumber;
    skipMultilineComment(line);
		joinMultiline(line);
		if(line[0]=='#') continue;
    if(stop(line)) {
			--lineNumber;
			fseek(fd,pos,SEEK_SET); // unread rejected line
			break;
		}
    src->parseLine(line,pass);
		pos = ftell(fd);
  }
}

void Parser::parseSectionLines(xstr& line, char pass) {
  Parsable* src = NULL;
	static vector<const char*> npattern = {"node", "final", "initial", "extern"};
  static bool(*stopnl)(xstr&) = [](xstr& line) { return line.length==0; };
  static bool(*stopvar)(xstr&) = [](xstr& line) { return line[0]!='.'; };
  bool(*stop)(xstr&) = stopnl; // default stopper
	if(line=="map") src = g->path;
  else if(line.startsWith("item ")) src = Item::parseSingleLine(g,line,pass), stop = stopvar;
  else if(line.startsWith(npattern)) src = Node::parseSingleLine(g,line,pass), stop = stopvar;
  else if(line.startsWith("mod ")) src = Mod::parseSingleLine(g,line,pass);
  else if(line.startsWith("action ")) src = Action::parseSingleLine(g,line,pass);
  else if(line.startsWith("array @")) src = Array<int>::parseSingleLine(g,line,pass);
  else if(line.startsWith("array ~")) src = Array<str>::parseSingleLine(g,line,pass);
  else if(line.startsWith("array $")) src = Array<VarContainer*>::parseSingleLine(g,line,pass);
	if(src!=NULL) parseLines(line,pass,src,stop);
	else Game::parseSingleLine(g,line,pass);
}

void Parser::parse() {
  if(blocked) return;
	// 1st pass: VarContainers (Items, Nodes, Mods, Actions)
	// 2nd pass: Variables
  for(pass=0; pass++<2; /* pass={1,2} */ ) {
    fseek(fd,0,SEEK_SET);
    skipBOM();
    lineNumber = 0;
    while(fgets(-line,8191,fd)!=NULL) {
  		line.reset(); line.trimMe();
      ++lineNumber;
      try {
        skipMultilineComment(line);
				joinMultiline(line);
				if(line[0]=='#' || line[0]==0) continue;
        parseSectionLines(line,pass);
      } catch(const char*& ex) {
        trace();
        puts(ex);
        getchar();
        exit(1);
      }
    }
  }
	if(g->actions.size()==0) try {
		Action* go = new Action(g,"go");
    go->strs["text"] = G_GO;
		sprintf(str::buffer,"select $here from paths ask(\"%s\")",G_WHEREGO);
		xstr line = str::buffer;
		go->parseLine(line,2);
		g->addAction(go);
	} catch(const char*& ex) {
    puts(ex);
    getchar();
    exit(1);
  }
}
