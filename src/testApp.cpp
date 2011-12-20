#include "testApp.h"
//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
ofxCvKalman *tuioPointSmoothed[32];

TuioPoint updateKalman(int id, TuioPoint tp) {
	if (id>=16) return NULL;
	if(tuioPointSmoothed[id*2] == NULL) {
		tuioPointSmoothed[id*2] = new ofxCvKalman(tp.getX());
		tuioPointSmoothed[id*2+1] = new ofxCvKalman(tp.getY());
		//tuioPointSmoothed[id*2+2] = new ofxCvKalman(tp.getZ());
	} else {
		tp.update(tuioPointSmoothed[id*2]->correct(tp.getX()),tuioPointSmoothed[id*2+1]->correct(tp.getY()),tp.getZ());
	}
	
	return tp;
}

void clearKalman(int id) {
	if (id>=16) return;
	if(tuioPointSmoothed[id*2]) {
		delete tuioPointSmoothed[id*2];
		tuioPointSmoothed[id*2] = NULL;
		delete tuioPointSmoothed[id*2+1];
		tuioPointSmoothed[id*2+1] = NULL;
		//delete tuioPointSmoothed[id*2+2];
		//tuioPointSmoothed[id*2+2] = NULL;
	}
}

void testApp::setup(){
	ofSetLogLevel(ofxOpenNI::LOG_NAME,OF_LOG_VERBOSE);
	openNI.setupFromXML("openni/config/ofxopenni_config.xml",false);
	openNiTracker.setup(openNI);
	openNiTracker.setSmoothing(0.5);
	activa = false;
	//////////////////////////////////////////////////////////////////
	anchoArea = 640/2.5;
	altoArea = 480/2.5;
	//////////////////////////////////////////////////////////////////
	cursorX = 0.0f;
	cursorY = 0.0f;
	posAreaX = 0.0f;
	posAreaY = 0.0f;
	filtro = 5.9;
	posAreaZ = 0.0f;
	
	fondoGui.loadImage("fondoJUI.png");
	jui.addFloat("area deteccion X", adeX, 0, 640);
	jui.addFloat("area deteccion y", adeY, 0, 480);
	jui.addFloat("area deteccion Width", adeAncho, 0, 640);
	jui.addFloat("area deteccion Height", adeAlto, 0, 480);
	jui.y = 50;
	
	TuioTime::initSession();	
	tuioServer = new TuioServer();
	tuioServer->setSourceName("tuioTrackerAutoskeleton");
	tuioServer->enableObjectProfile(false);
	tuioServer->enableBlobProfile(false);
	
	dentroArea = false;
	representacion = 0.0;
	
	for (int i=0;i<32;i++)
		tuioPointSmoothed[i] = NULL;
}

//--------------------------------------------------------------
void testApp::update(){
	ofSetWindowTitle(ofToString(ofGetFrameRate()));
	//////////////////////////////////////////////////////////////////
	openNI.update();
	openNiTracker.update();
	//// cojemos el punto del hombro que marcamos como el 0,0 en el medio de la pantalla
	//// posicion X del puntero es la distancia desde el hombro hasta la mano
	//// para calcular la Y cojemos la pos y del torso
	
	for (int s = 0; s<openNiTracker.getNumberOfTrackedUsers(); s++) {
		dentroArea = true;
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		ofxOpenNIUser usuario = openNiTracker.getTrackedUser(s);
		ofxOpenNILimb hombroDerecho = usuario.getLimb(usuario.RightUpperArm); ///hombro derecho 
		ofxOpenNILimb manoDerecha = usuario.getLimb(usuario.RightLowerArm); ///mano derecha
		ofxOpenNILimb torso = usuario.getLimb(usuario.RightLowerTorso); ///mano derecha
		
		/// comprobamos que este dentro del area de interes, sino lo borramos del tracker y salimos
		if(torso.begin.x<adeX || torso.begin.x>(adeX+adeAncho)){
			openNiTracker.mataUser(s);
			dentroArea = false;
			return;
		}
		
		anchoArea = (640/2.5) * (2300/torso.begin.z);
		//altoArea = altoArea * (torso.begin.z/1000);
		cout << "INFO::: " << torso.begin.z/1500 << endl;
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		//// actualizamos el area con la profundidad del user
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		//// el centro es el hombro derecho
		//// el hombro derecho es el 0,0
		//// lo que es lo mismo ofGetWidth()/2,ofGetHeight()/2
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		orX = hombroDerecho.begin.x;
		orY = hombroDerecho.begin.y;		
		manoX = manoDerecha.end.x;
		manoY = manoDerecha.end.y;
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		//// cursorX inicia en orX-anchoArea/2 ---- cursorY
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		posAreaX = (orX-anchoArea/2);
		posAreaY = (orY-altoArea/2);
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		filtraPunto(ofPoint((manoX - posAreaX),(manoY - posAreaY)));
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		/////controlamos si la bola esta o no en zon activa
		if((torso.begin.z - manoDerecha.end.z)>300){
			activa=true;
			representacion = 1.0;
		}else{
			activa=false;
			representacion = 0.0;
		}
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		posAreaZ = hombroDerecho.begin.z+100;
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		
	}
	///// jerky movimiento
	///// hay que poner un threshold para comprobar si se ha movido lo suficiente para actualizar el pto
	cursorX = ofMap(fpx, 0.0f, anchoArea, 0.0, 640.0, true);
	cursorY = ofMap(fpy, 0.0f, altoArea, 0.0, 480.0, true);
	//////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////
	TuioTime frameTime = TuioTime::getSessionTime();
	tuioServer->initFrame(frameTime);
	
	if(openNiTracker.getNumberOfTrackedUsers()>0 && dentroArea == true) {
		TuioPoint tp(cursorX,cursorY,representacion);
		
		TuioCursor *tcur = tuioServer->getClosestTuioCursor(tp.getX(),tp.getY());
		if ((tcur==NULL) || (tcur->getDistance(&tp)>0.2)) { 
			tcur = tuioServer->addTuioCursor(tp.getX(), tp.getY(), tp.getZ());
			updateKalman(tcur->getCursorID(),tcur);
		} else {
			TuioPoint kp = updateKalman(tcur->getCursorID(),tp);
			tuioServer->updateTuioCursor(tcur, kp.getX(), kp.getY(), kp.getZ());
		}
	}
	
	tuioServer->stopUntouchedMovingCursors();
	
	std::list<TuioCursor*> dead_cursor_list = tuioServer->getUntouchedCursors();
	std::list<TuioCursor*>::iterator dead_cursor;
	for (dead_cursor=dead_cursor_list.begin(); dead_cursor!= dead_cursor_list.end(); dead_cursor++) {
		clearKalman((*dead_cursor)->getCursorID());
	}
	
	tuioServer->removeUntouchedStoppedCursors();
	tuioServer->commitFrame();
	
}
void testApp::filtraPunto(ofPoint puntako){
	float _x = 0;
	float _y = 0;
	for (int i = 19; i > 0; i--)
	{
		posmanoFiltrada[i] = posmanoFiltrada[i - 1];
	}
	posmanoFiltrada[0] = puntako;
	
	for (int i = 0; i < 20; i++)
	{
		if (posmanoFiltrada[i] != NULL)
		{
			_x = _x + posmanoFiltrada[i].x;
			_y = _y + posmanoFiltrada[i].y;
		}
	}
	fpx = _x/20;
	fpy = _y/20;
}
//--------------------------------------------------------------
void testApp::draw(){
	ofSetColor(255, 255, 255);
	ofPushMatrix();
	ofTranslate(0, 0, 0);
	ofEnableAlphaBlending();
	openNI.draw(433, 0);
	fondoGui.draw(0, 0);
	jui.draw();
	ofSetColor(178, 178, 178);
	//jui.drawLabel("zona para detectar si el usuario esta dentro\r\ndel area de interes", 10, 50);
	///dibujar el area
	ofNoFill();
	ofSetColor(200, 200, 200);
	ofSetLineWidth(1);
	ofRect(adeX+433, adeY, adeAncho, adeAlto);
	ofFill();
	ofDisableAlphaBlending();
	ofPopMatrix();
	
	ofPushMatrix();
	ofTranslate(433, 0, 0);
	
	
	if (openNiTracker.getNumberOfTrackedUsers()>0) {
		ofSetLineWidth(1);
		openNiTracker.drawUser(0);
		
		ofSetColor(255, 255, 255);
		ofCircle(orX, orY, 10);
		ofCircle(manoX, manoY, 10);
		
		ofNoFill();
		ofSetLineWidth(3);
		ofSetColor(0, 255, 0);
		ofRect(posAreaX, posAreaY, anchoArea, altoArea);
		ofSetLineWidth(2);
		ofSetColor(0, 255, 0);
		ofLine(orX, orY,manoX, manoY);
		ofFill();
		
		if(activa){
			///mano activa es roja
			ofSetColor(255, 0, 0);
		}else {
			///mano inactiva es blanca
			ofSetColor(0, 0, 255);
		}
		
		ofCircle(cursorX, cursorY, 15);
	}
	
	std::list<TuioCursor*> alive_cursor_list = tuioServer->getTuioCursors();
	std::list<TuioCursor*>::iterator alive_cursor;
	for (alive_cursor=alive_cursor_list.begin(); alive_cursor!= alive_cursor_list.end(); alive_cursor++) {
		TuioCursor *ac = (*alive_cursor);
		
		int xpos = ac->getX();
		int ypos = ac->getY();
		float activaOno = ac->getZ();
		if(activaOno == 0){
			ofSetColor(0, 200, 0);
		}else{
			ofSetColor(200, 0, 0);
		}	
		ofCircle(xpos,ypos ,15);
		char idStr[32];
		sprintf(idStr,"%d",ac->getCursorID());
		ofDrawBitmapString(idStr, xpos+15, ypos+20);
		
	}
	ofPopMatrix();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	switch (key) {
		case 's':
			jui.save("gguisettings0.ini");
			break;
		default:
			break;
	}
}
//--------------------------------------------------------------
void testApp::keyReleased(int key){}