#include "ofApp.h"

#include <sstream>

static const char kNearClip[]  = "rk_nc";
static const char kFarClip[]   = "rk_fc";
static const char kTiltAngle[] = "rk_ta";
static const char kQuality[]   = "rk_qu";

//--------------------------------------------------------------
ofApp::ofApp(const string& publisher, const string& responder) : publisher(publisher), responder(responder) {
	nearClip  = client.getNearClip();
	farClip   = client.getFarClip();
	tiltAngle = client.getTiltAngle();
	quality   = client.getQuality();
	
	mode = 0;
}

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetFrameRate(60);
	ofEnableSmoothing();
	ofBackground(0, 0, 0);
	
	gui.setup("Client", ofGetWidth() - 260, 5, 255, 400);
	gui.addPanel("Settings", 0);
	gui.addSlider("Near Clip [mm]",
		      kNearClip,
		      nearClip,
		      0.0,
		      10000.0,
		      false);
	gui.addSlider("Far clip [mm]",
		      kFarClip,
		      farClip,
		      0.0,
		      10000.0,
		      false);
	gui.addSlider("Tilt Angle [deg]",
		      kTiltAngle,
		      tiltAngle,
		      -30.0,
		      30.0,
		      false);
	gui.addSlider("Quality",
		      kQuality,
		      quality,
		      0,
		      4,
		      true);
	gui.loadSettings("settings.xml");
	
	client.setServers(publisher, responder);
	client.setup();

	client.setClip(nearClip, farClip);
	client.setTiltAngle(tiltAngle);
	client.setQuality(quality);
}

//--------------------------------------------------------------
void ofApp::update() {
	gui.update();
	
	// Reflect settings from GUI to ofxRemoteKinect.
	float newNearClip  = gui.getValueF(kNearClip);
	float newFarClip   = gui.getValueF(kFarClip);
	float newTiltAngle = gui.getValueF(kTiltAngle);
	int newQuality     = gui.getValueI(kQuality);
	if (nearClip != newNearClip || farClip != newFarClip) {
		nearClip = newNearClip;
		farClip  = newFarClip;
		client.setClip(nearClip, farClip);
	}
	if (tiltAngle != newTiltAngle) {
		tiltAngle = newTiltAngle;
		client.setTiltAngle(tiltAngle);
	}
	if (quality != newQuality) {
		quality = newQuality;
		client.setQuality(quality);
	}
	
	client.update();
	
	// Reflect settings from ofxRemoteKinect to GUI.
	gui.setValueF(kNearClip, client.getNearClip());
	gui.setValueF(kFarClip, client.getFarClip());
	gui.setValueF(kTiltAngle, client.getTiltAngle());
	gui.setValueF(kQuality, client.getQuality());
}

//--------------------------------------------------------------
void ofApp::draw() {
	switch (mode) {
		case 0:
			draw3d();
			break;
		case 1:
            display_img = ofImage(client.getPixels());
            display_img.draw(0,0);
			//client.getTexture().draw(0.0, 0.0);
			break;
		default:
            display_img = ofImage(client.getDepthPixels());
            display_img.draw(0,0);
			//client.getDepthTexture().draw(0.0, 0.0);
			break;
	}
	
	gui.draw();

	// Show framerate.
	stringstream ss;
	ss << "FPS: " << ofGetFrameRate();
	ofDrawBitmapString(ss.str(), 520, 460);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (key == OF_KEY_RETURN)
		mode = (mode + 1) % 3;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ) {
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
	gui.mouseDragged(x, y, button);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
	gui.mousePressed(x, y, button);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	gui.mouseReleased();
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {
}

//--------------------------------------------------------------
void ofApp::draw3d() {
	ofPushMatrix();

	rotate += ofVec3f(1.0, 2.0, 3.0);
	ofTranslate(320, 240);
	ofRotateXRad(rotate.x);
	ofRotateYRad(rotate.y);
	ofRotateZRad(rotate.z);
	ofTranslate(-320, -240);
	
	ofPixels & pixels = client.getPixels();
	ofShortPixels & depthPixels = client.getDepthPixels();
	
	for (int x = 0; x < 640; x += 4) {
		for (int y = 0; y < 480; y += 4) {
            float z = (depthPixels.getColor(x, y).getBrightness() - 127) / 2.0;
			ofSetColor(pixels.getColor(x, y));
			ofDrawCircle(x, y, z, 2);
		}
	}
	
	ofSetColor(255, 255, 255, 255);
	ofPopMatrix();
}
