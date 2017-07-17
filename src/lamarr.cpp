#include <time.h>
#include "all.h"

void move(Game& g, Node* here);

int main(int argc, char** argv) {
	// init console
  srand(time(NULL));
  console_mode(CONSOLE_UTF);
  cursor(false);
  clear();

	// load game
  if(argc<2) error(E_GAMEMISSING),exit(1);
  Game game;
  Parser(argv[1],&game).parse();
//	game.dump(); getchar(); return 0;

	// show intro
	if(game.intro) try {
		xstr intro = -game.intro;
		intro.replaceMe("''","\"");
		wprint(game.srhs(intro));
	  colprintf("\n\n$GREEN %s $LIGHTGRAY", G_WAIT);
	  getkey();
	}
	catch(const char*& ex) {
	  error("%s\n",ex);
	}

	// run init
	try {
		Mod* init = game.getMod("init");
		init->run();
	}
	catch(const char*& ex) {
		// no init, never mind
	}

	try {
    Node* nodeHere;
		bool isFinal = false;
		do {
			VarContainer* here = game.findObj("$here",none);
			if(here->type!='n') throw report(R_HERE);
			nodeHere = static_cast<Node*>(here);
			isFinal = nodeHere->findInt(".@final",0)==1;
      if(!isFinal) move(game,nodeHere);
		} while(!isFinal);
		clear();
		wprint(nodeHere->findStr(".~text"));
		colprintf("\n\n$WHITE %s $LIGHTGRAY ", G_GAMEOVER);
	}
	catch(const char*& ex) {
	  error("%s\n",ex);
	}
  getchar();
  return 0;
}

void move(Game& game, Node* here) {
	clear();
	xstr templ = -here->findStr(".~text");
	templ.replaceMe("''","\"");
	wprint(here->srhs(templ));
  xstr statusbar = -game.findStr("~statusbar");
  if(statusbar) { puts("\n"); wprint(here->srhs(statusbar)); }
	puts("\n");
	Action* chosen;
	if(game.actions.size()>1) chosen = Action().doPickAsk<Action*>(game.actions,NULL);
	else chosen = game.actions.begin()->first;
	if(chosen!=NULL) chosen->run();
	else game.objs["here"] = here;
}