#include "ofApp.h"

#include <sstream>

#ifndef _MSC_VER
    #include <ifaddrs.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <CFNetwork/CFNetwork.h>
#endif

static const char kTiltAngle[] = "rk_ta";
static const char kShowImage[] = "show_image";

static const char kNearClip[]  = "rk_nc";
static const char kFarClip[]   = "rk_fc";
// static const char kQuality[]   = "rk_qu";

//--------------------------------------------------------------
ofApp::ofApp() {
    
	nearClip  = server.getNearClip();
	farClip   = server.getFarClip();
	tiltAngle = server.getTiltAngle();
	quality   = server.getQuality();
    
	server.setPorts(publisherPort, responderPort);
}

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetFrameRate(30);
	//ofEnableSmoothing();
    
    ofxControlPanel::topSpacing  = 0;
    
	gui.setup("Server", ofGetWidth() - 260, 0, 260, 220);
    gui.setXMLFilename("gui.config.xml");
	gui.addPanel("Settings", 0);
	gui.addSlider("Tilt Angle [deg]",
		      kTiltAngle,
		      tiltAngle,
		      -30.0,
		      30.0,
		      false);
    
    gui.addToggle("Show Image", kShowImage, true);    

    gui.addSlider("Near Clip [mm]",
                  kNearClip,
                  nearClip,
                  0.0,
                  10000.0,
                  false);
    
    gui.addSlider("Far clip [mm]",
                  kFarClip,
                  farClip,
                  1.0,
                  10000.0,
                  false);
    
  /*  gui.addSlider("Quality",
                  kQuality,
                  quality,
                  0,
                  4,
                  true);*/

	bool success = tryServerSetup();
    
    gui.loadSettings("gui_config.xml");
    
    string title = "RemoteKinect v:1.5.0, Server[" + GetMyIPAddress() + "] publish:" + ofToString(server.getPublisherPort()) + ", respond:" + ofToString(server.getResponderPort());
    ofLogNotice() << title;
    
    ofSetWindowTitle(title);
    
}

//--------------------------------------------------------------
bool ofApp::tryServerSetup() {
    
    last_setup_time = ofGetElapsedTimeMillis();
    
    ofLogNotice("tryServerSetup:") << last_setup_time;
    
    if(KINECT_320) {
        return server.setup(false, true, 320, 240); // send_rgb, send_depth
    }else{
        return server.setup(false, true, 640, 480);
    }
    
}

//--------------------------------------------------------------
void ofApp::update(){
	gui.update();
	
    if(server.running) {
        // Reflect settings from GUI to ofxRemoteKinect.
        float newTiltAngle = gui.getValueF(kTiltAngle);
        if (tiltAngle != newTiltAngle) {
            tiltAngle = newTiltAngle;
            server.setTiltAngle(tiltAngle);
        }
        
        // Reflect settings from GUI to ofxRemoteKinect.
        float newNearClip  = gui.getValueF(kNearClip);
        float newFarClip   = gui.getValueF(kFarClip);
      //  int newQuality     = gui.getValueI(kQuality);
        
        if (nearClip != newNearClip || farClip != newFarClip) {
            nearClip = newNearClip;
            farClip  = newFarClip;
            server.setClip(nearClip, farClip);
        }
        if (tiltAngle != newTiltAngle) {
            tiltAngle = newTiltAngle;
            server.setTiltAngle(tiltAngle);
        }
      /*  if (quality != newQuality) {
            quality = newQuality;
            server.setQuality(quality);
        }*/

        // Client requests may change server settings.
        server.update();
	
        // Reflect settings from ofxRemoteKinect to GUI.
        gui.setValueF(kTiltAngle, server.getTiltAngle());

    }else{
        
        if(ofGetElapsedTimeMillis() - last_setup_time > 10000) { // try every ten seconds
            // if enough time has elapsed try to get the kinect to the again
            tryServerSetup();
        }
        
    }
    
}

//--------------------------------------------------------------
void ofApp::draw(){
	
    server.draw();
    
    if(gui.getValueB(kShowImage)) {
        server.debugDraw(0,0,.5,.5);
    }else{
        ofBackground(64);
    }
    
	gui.draw();
	
	// Show framerate.
	stringstream ss;
    ss << "kw=" << (KINECT_320 ? 320 : 640) << " running=" << server.running << " fps=" << ofToString(ofGetFrameRate(), 2);
    ofDrawBitmapString(ss.str(), 50, 12);
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	gui.mouseDragged(x, y, button);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	gui.mousePressed(x, y, button);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	gui.mouseReleased();
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 
}

//--------------------------------------------------------------
string ofApp::GetMyIPAddress() {
    
    // see http://osxdaily.com/2010/11/21/find-ip-address-mac/
    string ip = ofSystem("ipconfig getifaddr en1");
    ip = ip.substr(0,ip.size()-1); // remove the trailing \n

    ofStringReplace(ip,  "\n",  "");
    
    return ip;
    
    // https://github.com/trentbrooks/ofxBonjourIp/blob/master/ofxBonjourIp/src/ofxBonjourIp.h
    
#ifdef _MSC_VER
    return "T.O.D.O";
#else
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *temp_addr = NULL;
    string networkAddress = "";
    string cellAddress = "";
    
    // retrieve the current interfaces - returns 0 on success
    if(!getifaddrs(&interfaces)) {
        // Loop through linked list of interfaces
        temp_addr = interfaces;
        while(temp_addr != NULL) {
            sa_family_t sa_type = temp_addr->ifa_addr->sa_family;
            if(sa_type == AF_INET || sa_type == AF_INET6) {
                string name = temp_addr->ifa_name; //en0
                string addr = inet_ntoa(((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr); // pdp_ip0
                
                // ignore localhost "lo0" addresses 127.0.0.1, and "0.0.0.0"
                //if(!ofIsStringInString(name, "lo") && addr != "0.0.0.0") {
                if(addr != "127.0.0.1" && addr != "0.0.0.0") {
                    
                    // can assume here it's "en0" or "en3" or "wlan0" or "pdp_ip0" (cell address)
                    // may need to add in a check to match the name (used to be matched to "en0")
                    ofLog() << "interface name / ip address: " << name << " / " << addr;
                    if(name == "pdp_ip0") {
                        // Interface is the cell connection on the iPhone
                        cellAddress = addr;
                    } else {
                        // if(name == "en0") - ignoring the name as this can be different
                        networkAddress = addr;
                    }
                }
                
            }
            temp_addr = temp_addr->ifa_next;
        }
        // Free memory
        freeifaddrs(interfaces);
    }
    
    // will return 0.0.0.0 of it hasn't found address
    string address = (networkAddress != "") ? networkAddress : cellAddress;
    return (address != "") ? address : "0.0.0.0";
#endif
}

