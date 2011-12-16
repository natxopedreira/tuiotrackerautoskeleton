#pragma once

#include "ofxOpenNI.h"
#include "ofxOpenNITracker.h"
#include "ofMain.h"
#include "GGui.h"
#include "TuioServer.h"
#include "ofxOpenCv.h"
#include "ofxCvKalman.h"
using namespace TUIO;

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void keyPressed  (int key);
		void keyReleased(int key);
		void filtraPunto(ofPoint puntako);

	ofxOpenNI openNI;
	ofxOpenNITracker openNiTracker;

	int px,py;
	bool activa;
	
	float multiplicarX,multiplicarY;
	
	ofPoint	projectPos;		// position on screen
	ofPoint	progPos;		// position from 0.0 to 1.0
	
	GGui jui;
	ofImage fondoGui;
	
	TuioServer *tuioServer;
	////// cursor
	float orX, orY;
	float manoX,manoY;
	float cursorX,cursorY,fpx,fpy;
	int anchoArea,altoArea,posAreaX,posAreaY,posAreaZ;
	float filtro;
	
	ofPoint posmanoFiltrada[20];
	//////area para deteccion de esqueleto
	float adeX,adeY,adeAncho,adeAlto;
};
