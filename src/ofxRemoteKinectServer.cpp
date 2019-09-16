#include "ofxRemoteKinectServer.h"

#include <sstream>

#include "ofxRemoteKinectProtocol.h"

//--------------------------------------------------------------
ofxRemoteKinectServer::ofxRemoteKinectServer(): publisherPort(8989),
						responderPort(8990),
						nearClip(500.0),
						farClip(4000.0),
						tiltAngle(0.0),
						quality(2),
						subscribed(false),
						activeTimer(0) {
}

ofxRemoteKinectServer::~ofxRemoteKinectServer() {
	// Reset kinect device configurations.
	kinect.setDepthClipping();
	kinect.setCameraTiltAngle(0.0);
}

//--------------------------------------------------------------
bool ofxRemoteKinectServer::setup(bool sendRGB, bool sendData, int _kinectWidth, int _kinectHeight) {
	// Initialize kinect device.
        
    int kinectsAvailable = kinect.numAvailableDevices();
    
    if(!kinectsAvailable) {
        ofLogError("Kinect") << " No Kinects Available.";
        return false;
    }
    
	//kinect.setRegistration(true);  // enable depth->video image calibration
    
	if(!kinect.init(sendData, sendRGB, true)){ // false,false,true)) { // disable video image (faster fps)
          // init(nfrared=false, bool video=true, bool texture=true)
        ofLogError("Kinect") << " Failed to init().";
        return false;
    }
	//_kinect.init(true); // shows infrared instead of RGB video image
	//_kinect.init(false, false); // disable video image (faster fps)
	
	if(!kinect.open()){ // opens first available _kinect
        ofLogError("Kinect") << " Failed to open().";
        return false;
    }
    
	//_kinect.open(1);	// open a _kinect by id, starting with 0 (sorted by serial # lexicographically))
	//_kinect.open("A00362A08602047A");	// open a kinect using it's unique serial #
	
	// print the intrinsic IR sensor values
	if(kinect.isConnected()) {
		ofLogNotice("Kinect") << "sensor-emitter dist: " << kinect.getSensorEmitterDistance() << "cm";
		ofLogNotice("Kinect") << "sensor-camera dist:  " << kinect.getSensorCameraDistance() << "cm";
		ofLogNotice("Kinect") << "zero plane pixel size: " << kinect.getZeroPlanePixelSize() << "mm";
		ofLogNotice("Kinect") << "zero plane dist: " << kinect.getZeroPlaneDistance() << "mm";
	}else{
        ofLogError("Kinect") << " Not Connected.";
        return false;
    }
    
	/*kinect.setRegistration(true);
	kinect.init();	
     kinect.open();*/
	
    rgbEnabled = sendRGB;
    dataEnabled = sendData;
    
    kinectWidth = _kinectWidth;
    kinectHeight = _kinectHeight;
	// TODO(toyoshim): Revive movie fallback mode.
	
	// Start ZMQ servers.
	stringstream ss;
	ss << "tcp://*:" << publisherPort;
	publisher.bind(ss.str());
	ofLog(OF_LOG_NOTICE) << "Start remote kinect publisher at tcp://*: " << publisherPort;
	
	ss.str("");
	ss << "tcp://*:" << responderPort;
	responder.bind(ss.str());
    
	ofLog(OF_LOG_NOTICE) << "Start remote kinect responder at tcp://*: " << responderPort;
    
    running = true;
    
    return true;
}

//--------------------------------------------------------------
void ofxRemoteKinectServer::update() {
	if (subscribed) {
		activeTimer--;
		if (activeTimer == 0)
			subscribed = false;
	}
	try {
		while (responder.hasWaitingMessage()) {
			string data;
			responder.getNextMessage(data);

			ofxRemoteKinectProtocol::type type;
			float f1;
			float f2;
			int i1;
			ofxRemoteKinectProtocol::parse(data, &type, &f1, &f2, &i1);
			string response;
			switch (type) {
				case ofxRemoteKinectProtocol::TYPE_PING:
					subscribed = true;
					activeTimer = 30;
					ofxRemoteKinectProtocol::buildPong(&response);
					break;
				case ofxRemoteKinectProtocol::TYPE_CLIP:
					setClip(f1, f2);
					ofxRemoteKinectProtocol::buildAck(&response);
					break;
				case ofxRemoteKinectProtocol::TYPE_TILT_ANGLE:
					setTiltAngle(f1);
					ofxRemoteKinectProtocol::buildAck(&response);
					break;
				case ofxRemoteKinectProtocol::TYPE_QUALITY:
					setQuality(i1);
					ofxRemoteKinectProtocol::buildAck(&response);
					break;
				default:
					ofxRemoteKinectProtocol::buildNack(&response);
					break;
			}
			responder.send(response);
		}
	} catch (zmq::error_t e) {
		ofLog(OF_LOG_ERROR) << "unexpected ZMQ exception: " << e.what();
	}
}

//--------------------------------------------------------------

void ofxRemoteKinectServer::debugDraw(float x, float y, float sx, float sy) {
    
    if(!running) return;
    
    ofPushMatrix();
    ofTranslate(x, y);
    ofScale(-1*sx, sy); // lazy flip but faster than stuffing around with the images
    
    
	if(rgbEnabled) {
        kinect.getTextureReference().draw(-1*640, 0.0);
    }else if(dataEnabled) {
        //TODO: draw the IR image here.
    }
    
	if(dataEnabled) kinect.getDepthTextureReference().draw((rgbEnabled ? -2 : -1)*640.0, 0.0);

    ofPopMatrix();
}

void ofxRemoteKinectServer::draw() {
    
    if(!running) return;
    
	// Draw actual kinect images.
	kinect.update();
 	
	// TODO(toyoshim): Revive recording feature.
	
	// Skip following process if there is no subscriber.
	if (!subscribed)
		return;

    string data;
    	
	if(rgbEnabled){
        
        ofImageQualityType qualityType = static_cast<ofImageQualityType>(quality);
        ofBuffer videoBuffer;
        ofSaveImage(kinect.getPixelsRef(), videoBuffer, OF_IMAGE_FORMAT_JPEG, qualityType);
        ofxRemoteKinectProtocol::buildVideoFrame(&data, videoBuffer);
        publisher.send(data, true);
    
        ofImage videoImage;
        videoImage.loadImage(videoBuffer);
        videoImage.draw(0.0, 480);
        
    }
    
    if(dataEnabled){
        
        ofBuffer depthBuffer;
        
        
        if(kinectWidth != 640 || kinectHeight != 480 ){
        
            ofShortImage resizedImage;
            resizedImage.allocate(320, 240, OF_IMAGE_GRAYSCALE);
            
            int i = 0;
            for(int y = 0; y < 480; y+=2){
                for(int x = 0; x < 640; x+=2){
                
                    resizedImage.getPixelsRef()[i] = kinect.getRawDepthPixels()[y*640+x];
                    
                    i++;

                }
            }

            ofSaveImage(resizedImage.getPixelsRef(), depthBuffer, OF_IMAGE_FORMAT_TIFF, OF_IMAGE_QUALITY_LOW);
        
        }else{
            
            ofSaveImage(kinect.getRawDepthPixelsRef(), depthBuffer, OF_IMAGE_FORMAT_TIFF, OF_IMAGE_QUALITY_LOW);
            
        }
        
        ofxRemoteKinectProtocol::buildDepthFrame(&data, depthBuffer);
        publisher.send(data, true);

//        ofShortImage depthImage;
//        depthImage.loadImage(depthBuffer);
//        depthImage.draw(640.0, 480.0);
    
    }
}

//--------------------------------------------------------------
void ofxRemoteKinectServer::setPorts(int publisher, int responder) {
	publisherPort = publisher;
	responderPort = responder;
}

//--------------------------------------------------------------
void ofxRemoteKinectServer::setClip(float newNearClip, float newFarClip) {
	nearClip = newNearClip;
	farClip  = newFarClip;
	
	string data;
	ofxRemoteKinectProtocol::buildClip(&data, nearClip, farClip);
	publisher.send(data, true);
	
	kinect.setDepthClipping(nearClip, farClip);
}

//--------------------------------------------------------------
void ofxRemoteKinectServer::setTiltAngle(float newTiltAngle) {
	tiltAngle = newTiltAngle;
	
	string data;
	ofxRemoteKinectProtocol::buildTiltAngle(&data, tiltAngle);
	publisher.send(data, true);

	kinect.setCameraTiltAngle(tiltAngle);
}

//--------------------------------------------------------------
void ofxRemoteKinectServer::setQuality(int newQuality) {
	quality = newQuality;
	
	string data;
	ofxRemoteKinectProtocol::buildQuality(&data, quality);
	publisher.send(data, true);
}
