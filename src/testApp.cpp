#include "testApp.h"
//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
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

		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		ofxOpenNIUser usuario = openNiTracker.getTrackedUser(s);
		ofxOpenNILimb hombroDerecho = usuario.getLimb(usuario.RightUpperArm); ///hombro derecho 
		ofxOpenNILimb manoDerecha = usuario.getLimb(usuario.RightLowerArm); ///mano derecha
		ofxOpenNILimb torso = usuario.getLimb(usuario.RightLowerTorso); ///mano derecha
		
		/// comprobamos que este dentro del area de interes, sino lo borramos del tracker y salimos
		if(torso.begin.x<adeX || torso.begin.x>(adeX+adeAncho)){
			openNiTracker.mataUser(s);
			return;
		}
		
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
		}else{
			activa=false;
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