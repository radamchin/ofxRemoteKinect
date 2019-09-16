#pragma once

#include "ofMain.h"
#include "ofxKinect.h"
#include "ofxZmq.h"

class ofxRemoteKinectServer {
public:
	ofxRemoteKinectServer();
	~ofxRemoteKinectServer();
	
	bool setup(bool sendRGB, bool sendData, int _kinectWidth=640, int _kinectHeight = 480);
	void update();
    void debugDraw(float x = 0, float y = 0, float sx = 1.0, float sy=1.0);
	void draw();
	
	void setPorts(int publisher, int responder);  // Should be called before calling setup().
	void setClip(float nearClip, float farClip);
	void setTiltAngle(float tiltAngle);
	void setQuality(int quality);
	
	float getNearClip() { return nearClip; }
	float getFarClip() { return farClip; }
	float getTiltAngle() { return tiltAngle; }
	int getQuality() { return quality; }
	
    
    int getPublisherPort() { return publisherPort; }
    int getResponderPort() { return responderPort; }
    
    int kinectWidth;
    int kinectHeight;
    
    bool running = false;
    
private:
	int publisherPort;
	int responderPort;
	float nearClip;
	float farClip;
	float tiltAngle;
	int quality;
	
	ofxKinect kinect;
	ofxZmqPublisher publisher;
	ofxZmqReply responder;
	
	bool subscribed;
	int activeTimer;
    
    bool rgbEnabled;
    bool dataEnabled;
};
