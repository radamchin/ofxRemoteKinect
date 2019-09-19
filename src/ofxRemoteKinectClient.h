#pragma once

#include "ofMain.h"
#include "ofxZmq.h"

class ofxRemoteKinectClient {
public:
	ofxRemoteKinectClient(int kinectWidth = 640, int kinectHeight = 480);
	~ofxRemoteKinectClient();
    
	void setup();
	void update();
	void draw();

	// setServer() should be called before calling setup().
	// |server| string should follow the format 'tcp://<host>:<port>'.
	void setServers(const string& publisher, const string& responder);
	// Others should be called after calling setup().
	void setClip(float nearClip, float farClip);
	void setTiltAngle(float tiltAngle);
	void setQuality(int quality);
	
	float getNearClip() { return nearClip; }
	float getFarClip() { return farClip; }
	float getTiltAngle() { return tiltAngle; }
	int getQuality() { return quality; }
	
	unsigned char* getPixelsData() { return getPixels().getData(); }
    unsigned short* getDepthPixelsData() { return getDepthPixels().getData(); }
    //ofPixels& getPixels() { return getPixelsRef();  };
    //ofShortPixels& getDepthPixels() { return getDepthPixelsRef(); };
    
    // Thread-safe version
    unsigned short* getDepthImagePixelsData() { return depthImage.getPixels().getData(); }
    // ofPixels getDepthImagePixels() { return depthImage.getPixels(); }
    
	ofPixels& getPixels();
	ofShortPixels& getDepthPixels();
    ofShortPixels& getDepthImagePixels();
	
	//ofTexture& getTexture() { return texture; }
	//ofTexture& getDepthTexture() { return depthTexture; }

    int framesSinceLastRequest;

    ofMutex *lock;
    ofMutex *resizeLock;
    
    
    string getPublisher() { return publisher; }
    string getResponder() { return responder; }
    
    bool connected() { return ping.connected(); }
    
    bool running = false;
    bool image_set = false;
    
    void stop();
    
    int depthFrameRecievedCount = 0;
    
private:
	string publisher;
	string responder;
	float nearClip;
	float farClip;
	float tiltAngle;
	int quality;

	ofxZmqSubscriber subscriber;
	ofxZmqRequest requester;
	ofxZmqRequest ping;
	
	//ofTexture texture;
	//ofTexture depthTexture;
	
	ofPixels pixels;
	ofShortPixels depthPixels;
    
    // Added for thread-safe version
    ofShortImage depthImage;
	
    
	bool pixelsIsDirty;
	bool depthPixelsIsDirty;
    
    bool frame_parsing = false;
};
