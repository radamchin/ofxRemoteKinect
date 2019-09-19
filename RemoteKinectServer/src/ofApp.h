#pragma once

#include "ofMain.h"
#include "ofxControlPanel.h"
#include "ofxRemoteKinect.h"

#define KINECT_320 1

class ofApp : public ofBaseApp {
public:
	ofApp();
	
    int publisherPort = 8989;
	int responderPort = 8990;
    
	void setup();
	void update();
	void draw();
	
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	
    string GetMyIPAddress();
    bool tryServerSetup();
    
private:
	ofxControlPanel gui;
	ofxRemoteKinectServer server;
    
	float nearClip;
	float farClip;
	float tiltAngle;
	int quality;
    
    long last_setup_time = 0;
};
