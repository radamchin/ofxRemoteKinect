#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
	//ofAppGlutWindow window;
	//ofSetupOpenGL(&window, 640, 480, OF_WINDOW);
	ofRunApp(new ofApp("tcp://localhost:8989", "tcp://localhost:8990"));
}
