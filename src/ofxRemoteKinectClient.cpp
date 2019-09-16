#include "ofxRemoteKinectClient.h"

#include <sstream>

#include "ofxRemoteKinectProtocol.h"

//--------------------------------------------------------------
ofxRemoteKinectClient::ofxRemoteKinectClient(int kinectWidth, int kinectHeight): publisher("tcp://localhost:8989"),
						responder("tcp://localhost:8990"),
						nearClip(500.0),
						farClip(4000.0),
						tiltAngle(0.0),
						quality(2),
						pixelsIsDirty(true),
						depthPixelsIsDirty(true) {
	//texture.allocate(640, 480, GL_RGB);
	//depthTexture.allocate(640, 480, GL_LUMINANCE16);
    
    depthImage.setUseTexture(false);
    depthImage.allocate(kinectWidth,kinectHeight,OF_IMAGE_GRAYSCALE);
    depthImage.getPixelsRef().set(0);
    image_set = false;

}

ofxRemoteKinectClient::~ofxRemoteKinectClient() {
    ofLogNotice("ofxRemoteKinectClient") << "DISPOSE!!";
    
}

//--------------------------------------------------------------
void ofxRemoteKinectClient::setup() {
	// Start ZMQ clients.
	subscriber.connect(publisher);
	requester.connect(responder);
	ping.connect(responder);
    framesSinceLastRequest = 0;
    running = true;
}

//--------------------------------------------------------------
void ofxRemoteKinectClient::update() {
    
    framesSinceLastRequest++;
	try {
		string data;
		while (ping.hasWaitingMessage()) {
			ping.getNextMessage(data);
			// TODO(toyoshim): Check if the response is ping.
		}
		while (requester.hasWaitingMessage()) {
			requester.getNextMessage(data);
			// TODO(toyoshim): Check if the response is ack.
		}
		while (subscriber.hasWaitingMessage(0.1)) {
			subscriber.getNextMessage(data);
            framesSinceLastRequest = 0;
			ofxRemoteKinectProtocol::type type;
			float f1;
			float f2;
			int i1;
			ofxRemoteKinectProtocol::parse(data, &type, &f1, &f2, &i1);
			switch (type) {
				case ofxRemoteKinectProtocol::TYPE_CLIP:
					nearClip = f1;
					farClip = f2;
					break;
				case ofxRemoteKinectProtocol::TYPE_TILT_ANGLE:
					tiltAngle = f1;
					break;
				case ofxRemoteKinectProtocol::TYPE_QUALITY:
					quality = i1;
					break;
//				case ofxRemoteKinectProtocol::TYPE_VIDEO_FRAME:
//					ofxRemoteKinectProtocol::parseFrame(data, &texture);
//					pixelsIsDirty = true;
//					break;
				case ofxRemoteKinectProtocol::TYPE_DEPTH_FRAME:
//					ofxRemoteKinectProtocol::parseFrame(data, &depthTexture);
                    lock->lock();                   
                    if(running) {
                        frame_parsing = true;
                        // TODO: could wrap this in a try catch thing to catch exception and ensure frame_parsing is switched off
                        try {
                            ofxRemoteKinectProtocol::parseFrame(data, &depthImage);
                        } catch (int e) {
                            ofLogError("ofxRemoteKinectProtocol::parseFrame") << "An exception occurred. #" << e << '\n';
                            // could possibly zero depthImage here incase it gets blanked.
                        }
                        
                        frame_parsing = false;
                        image_set = true;
                    }
                    lock->unlock();
                    depthFrameRecievedCount++;
					depthPixelsIsDirty = true;
					break;
				default:
					// TODO(toyoshim): Handle unexpected data.
					break;
			}
		}
	} catch (zmq::error_t e) {
		ofLog(OF_LOG_ERROR) << "ZMQ catch exception: " << e.what();
	}
	// TODO(toyoshim): Implement an internal send queue to wait for reply for previous request.
	// Currently, requests and pings may drop.
	string data;
	ofxRemoteKinectProtocol::buildPing(&data);
	ping.send(data);
    
}

//--------------------------------------------------------------
void ofxRemoteKinectClient::draw() {
	//texture.draw(0.0, 0.0);
    depthImage.draw(0, 200);
}

//--------------------------------------------------------------
void ofxRemoteKinectClient::setServers(const string& newPublisher, const string& newResponder) {
	publisher = newPublisher;
	responder = newResponder;
}

//--------------------------------------------------------------
void ofxRemoteKinectClient::setClip(float newNearClip, float newFarClip) {
	nearClip = newNearClip;
	farClip  = newFarClip;
	string data;
	ofxRemoteKinectProtocol::buildClip(&data, nearClip, farClip);
	requester.send(data, true);
}

//--------------------------------------------------------------
void ofxRemoteKinectClient::setTiltAngle(float newTiltAngle) {
	tiltAngle = newTiltAngle;
	string data;
	ofxRemoteKinectProtocol::buildTiltAngle(&data, tiltAngle);
	requester.send(data, true);
}

//--------------------------------------------------------------
void ofxRemoteKinectClient::setQuality(int newQuality) {
	quality = newQuality;
	string data;
	ofxRemoteKinectProtocol::buildQuality(&data, quality);
	requester.send(data, true);
}

////--------------------------------------------------------------
//ofPixels& ofxRemoteKinectClient::getPixelsRef() {
//	if (pixelsIsDirty) {
//		texture.readToPixels(pixels);
//		pixelsIsDirty = false;
//	}
//	return pixels;
//}

//--------------------------------------------------------------
ofShortPixels& ofxRemoteKinectClient::getDepthImagePixelsRef() {
	return depthImage.getPixelsRef();
}

//--------------------------------------------------------------
//ofShortPixels& ofxRemoteKinectClient::getDepthPixelsRef() {
//	if (depthPixelsIsDirty) {
//		depthTexture.readToPixels(depthPixels);
//		depthPixelsIsDirty = false;
//	}
//	return depthPixels;
//}

//--------------------------------------------------------------
void ofxRemoteKinectClient::stop() {
    
    ofLogNotice("ofxRemoteKinectClient:stop");
    
    running = false;
    image_set = false;
    
    if(frame_parsing) {
        ofLogNotice("ofxRemoteKinectClient:stop while frame_parsing");
        // need to do somethhing to stop it.
    }
    
 /*   while(frame_parsing) {
        // wait here until thats finished to avoid locky locks
        
    }
    */
    
    depthImage.getPixelsRef().set(0);
    //Need to clean up the zmq shit better as getting a too many files open, os forced app exit on this
    //ERR is in zmq src/signaler.cpp
    //subscriber.
	//requester.
	//ping.
    
}
