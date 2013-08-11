#pragma once

#include "ofMain.h"
#include "ofxiPhoneExtras.h"
#include "pkmEXTAudioFileReader.h"
#include "pkmEXTAudioFileWriter.h"
#include "pkmGPSSoundLibrary.h"
#include "UTM.h"
#include <map.h>
#include "ofxMSATimer.h"

#define DEBUG_MODE 1

using namespace std;


//--------------------------------------------------------------
class baseItem
{
public:
    string label;
    string sublabel;
    ofVec2f position;
};

//--------------------------------------------------------------
class pictureItem : public baseItem
{
public:
    ofImage image;
};

//--------------------------------------------------------------
class soundItem : public baseItem
{
public:
    string filename;
    int addedSoundIdx;  // for deleting from the database
};

//--------------------------------------------------------------
class testApp : public ofxiPhoneApp, ofxiPhoneMapKitListener {
	
public:
    
    //--------------------------------------------------------------
    // main app callbacks
	void setup();
	void update();
	void draw();
	void exit();
	
    
    //--------------------------------------------------------------
    // ui callbacks
	void touchDown(ofTouchEventArgs &touch);
	void touchMoved(ofTouchEventArgs &touch);
	void touchUp(ofTouchEventArgs &touch);
	void touchDoubleTap(ofTouchEventArgs &touch);
	void touchCancelled(ofTouchEventArgs &touch);
	    
    
    //--------------------------------------------------------------
    // nav bar callbacks
    void selectedRecord();
    void selectedCamera();
    void selectedPageCurl();
    void selectedPlay();
    
    
    //--------------------------------------------------------------
    // list callbacks
    void selectedCancelList();
    void selectedIndex(int i);
    
    
    //--------------------------------------------------------------
    // audio callbacks
    void audioIn(float *buf, int size, int ch);
    void audioOut(float *buf, int size, int ch);
    
    
    //--------------------------------------------------------------
    // other app callbacks
	void lostFocus();
	void gotFocus();
	void gotMemoryWarning();
	void deviceOrientationChanged(int newOrientation);
    
    
    //--------------------------------------------------------------
    // GPS callbacks
	// and optional callbacks for Map related events
    void selectedAnnotation(string title, string subtitle);
    void unselectedAnnotation(string title, string subtitle);
	void regionWillChange(bool animated);
	void regionDidChange(bool animated);
	void willStartLoadingMap();
	void didFinishLoadingMap();
	void errorLoadingMap(string errorDescription);
    void updateListenerPosition();
	
    
    //--------------------------------------------------------------
	// font for writing
	ofTrueTypeFont font;

    
    //--------------------------------------------------------------
	// instance of ofxiPhoneMapKit
	// all MapKit related functionality is through this object
	ofxiPhoneMapKit mapKit;
	ofxiPhoneCoreLocation * coreLocation;
    bool bCompass, bGPS;
    ofImage pinImg;
    
    
    //--------------------------------------------------------------
    // image picker
    ofxiPhoneImagePicker imagePicker;
    int currentImg, currentDrawnImg;
    
    
    //--------------------------------------------------------------
    // sound recording
    pkmEXTAudioFileWriter soundWriter;
    pkmEXTAudioFileReader soundReader;
    ofImage recordImg, stopImg;
    int currentSound, currentPlayingSound;
    string currentSoundFilename;
    int bufferSize, nChannels;
    unsigned long currentRecSample, currentPlaySample;
    float *currentInBuffer, *currentOutBuffer;
    
    
    //--------------------------------------------------------------
    // spatialization
    pkmGPSSoundLibrary soundLibrary;
    unsigned int addedSoundIdx;
    char UTMzone[1];
    double UTM_x, UTM_y;
    double currentSound_x, currentSound_y, currentSound_z;
    double listener_x, listener_y, listener_z;
    double lat, lon;

    
    //--------------------------------------------------------------
    // url file loader for getting mp3s/xml stuff
    ofURLFileLoader urlFileLoader;
    
    
    //--------------------------------------------------------------
    // for debugging with GPS, writing GPS and storing a path, and reading the path
    FILE *gpsFileWriter, *gpsFileReader;
    vector<ofVec2f> debugGPSCoords;
    unsigned int currentCoord;
    bool bLogGPS,bUseLoggedGPS,bUseDebugPath;
    
    
    //--------------------------------------------------------------
    // database of images/sounds
    vector<baseItem *> database;
    vector<float> distances;
    vector<int> sortedDistancesLookupTable;
    map<string, int> annotationLookupTable;
    
    
    //--------------------------------------------------------------
    // text input for annotation
    ofxiPhoneKeyboard *keyboard;
    
    //--------------------------------------------------------------
    // timer for updating map
    ofxMSATimer mapUpdateTimer;
    
    //--------------------------------------------------------------
    // control variables
    bool bSelectedCamera, bSelectedPageCurl, bSelectedPlay, bSelectedRecord, bSelectedAnnotation;
    bool bDrawImage, bDrawSound, bDrawList, bPlaySound, bPlaySpatialization, bUserInputingSoundAnnotation;
	
    
};

