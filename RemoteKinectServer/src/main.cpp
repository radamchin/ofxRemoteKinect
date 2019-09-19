#include "ofMain.h"
#include "ofApp.h"
//#include "ofAppGlutWindow.h"

//========================================================================
int main( ){
	//ofAppGlutWindow window;
	//ofSetupOpenGL(&window, 580, 240, OF_WINDOW);
    ofSetupOpenGL(580, 240,OF_WINDOW);            // <-------- setup the GL context
	ofRunApp(new ofApp());
}
