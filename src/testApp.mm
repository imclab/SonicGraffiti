/*
 
 LICENCE
 
 pkmSonicGraffiti "The Software" © Parag K Mital, parag@pkmital.com
 
 The Software is and remains the property of Parag K Mital
 ("pkmital") The Licensee will ensure that the Copyright Notice set
 out above appears prominently wherever the Software is used.
 
 The Software is distributed under this Licence:
 
 - on a non-exclusive basis,
 
 - solely for non-commercial use in the hope that it will be useful,
 
 - "AS-IS" and in order for the benefit of its educational and research
 purposes, pkmital makes clear that no condition is made or to be
 implied, nor is any representation or warranty given or to be
 implied, as to (i) the quality, accuracy or reliability of the
 Software; (ii) the suitability of the Software for any particular
 use or for use under any specific conditions; and (iii) whether use
 of the Software will infringe third-party rights.
 
 pkmital disclaims:
 
 - all responsibility for the use which is made of the Software; and
 
 - any liability for the outcomes arising from using the Software.
 
 The Licensee may make public, results or data obtained from, dependent
 on or arising out of the use of the Software provided that any such
 publication includes a prominent statement identifying the Software as
 the source of the results or the data, including the Copyright Notice
 and stating that the Software has been made available for use by the
 Licensee under licence from pkmital and the Licensee provides a copy of
 any such publication to pkmital.
 
 The Licensee agrees to indemnify pkmital and hold them
 harmless from and against any and all claims, damages and liabilities
 asserted by third parties (including claims for negligence) which
 arise directly or indirectly from the use of the Software or any
 derivative of it or the sale of any products based on the
 Software. The Licensee undertakes to make no liability claim against
 any employee, student, agent or appointee of pkmital, in connection
 with this Licence or the Software.
 
 
 No part of the Software may be reproduced, modified, transmitted or
 transferred in any form or by any means, electronic or mechanical,
 without the express permission of pkmital. pkmital's permission is not
 required if the said reproduction, modification, transmission or
 transference is done without financial return, the conditions of this
 Licence are imposed upon the receiver of the product, and all original
 and amended source code is included in any transmitted product. You
 may be held legally responsible for any copyright infringement that is
 caused or encouraged by your failure to abide by these terms and
 conditions.
 
 You are not permitted under this Licence to use this Software
 commercially. Use for which any financial return is received shall be
 defined as commercial use, and includes (1) integration of all or part
 of the source code or the Software into a product for sale or license
 by or on behalf of Licensee to third parties or (2) use of the
 Software or any derivative of it for research with the final aim of
 developing software products for sale or license to a third party or
 (3) use of the Software or any derivative of it for research with the
 final aim of developing non-software products for sale or license to a
 third party, or (4) use of the Software to provide any service to an
 external organisation for which payment is received. If you are
 interested in using the Software commercially, please contact pkmital to
 negotiate a licence. Contact details are: parag@pkmital.com
 
 */

#include "testApp.h"
#include <Accelerate/Accelerate.h>
#include "MyGuiViewController.h"
#include "pkmAudioWindow.h"
#include "MyGuiListController.h"
#include "UTM.h"

MyGuiViewController * myGuiViewController;
MyGuiListController * myGuiListController;

#define POS1_LATITUDE		51.4747
#define POS1_LONGITUDE		-0.0375915

//--------------------------------------------------------------
void testApp::exit() {
    if (bLogGPS) {
        fclose(gpsFileWriter);
    }
    free(currentInBuffer);
    free(currentOutBuffer);
    //    pkmBinauralSoundObject::deallocate();
    pkmBinauralizerTree::deallocate();
    pkmAudioWindow::deallocate();
    [myGuiViewController release];
    [myGuiListController release];
    
    delete coreLocation;
}


//--------------------------------------------------------------
void testApp::setup(){	
    
    // control variables
    bSelectedRecord = false;
    bSelectedCamera = false;
    bSelectedPageCurl = false; 
    bSelectedPlay = false;
    bSelectedAnnotation = true;
    bPlaySound = false;
    bPlaySpatialization = false;
    bUseLoggedGPS = false;
    bLogGPS = false;
    bUseDebugPath = false;
    bUserInputingSoundAnnotation = false;
    
    // app setup
    ofSetVerticalSync(true);
	ofRegisterTouchEvents(this);
	ofxAccelerometer.setup();
	ofxiPhoneAlerts.addListener(this);
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofEnableAlphaBlending();
	font.loadFont("verdana.ttf", 12);
    
    lat = POS1_LATITUDE;
    lon = POS1_LONGITUDE;
    
    // open the mapview
	mapKit.open();
	mapKit.setRegionWithMeters(POS1_LATITUDE, POS1_LONGITUDE, 500, 500);
	mapKit.addListener(this);
	mapKit.setType(OFXIPHONE_MAPKIT_HYRBID);
    
    pinImg.loadImage(ofToDataPath("pin.png"));
	
	// setup gps
    // setup gps logging
    if(bLogGPS)
    {
        stringstream ss;
        ss << ofxiPhoneGetDocumentsDirectory() << "gpslog.txt";
        gpsFileWriter = fopen(ss.str().c_str(), "w");
        
        mapKit.setShowUserLocation(true);
        
    }
    else if(bUseLoggedGPS)
    {
        currentCoord = 0;
        stringstream ss;
        ss << ofxiPhoneGetDocumentsDirectory() << "gpslog.txt";
        gpsFileReader = fopen(ss.str().c_str(), "r");
        char buf[256];
        float x,y;
        while(fgets(buf, 256, gpsFileReader) != NULL)
        {
            sscanf(buf, "%f,%f\n",
                   &x, &y);
            debugGPSCoords.push_back(ofVec2f(x,y));
        }
        fclose(gpsFileReader);
        cout << "[OK] Read " << debugGPSCoords.size() << " coordinates." << endl;
        
        
        mapKit.setShowUserLocation(false);
    }
    else if(bUseDebugPath)
    {
        int numPositions = 360*10;
        float numPedals = 2;
        for (int i = 0; i < numPositions; i++) {
            float x = POS1_LATITUDE + cos((float)i/100.0)*cos(numPedals*(float)i/100.0)/10000.0;
            float y = POS1_LONGITUDE + sin((float)i/100.0)*cos(numPedals*(float)i/100.0)/10000.0;
            debugGPSCoords.push_back(ofVec2f(x,y));
        }
        
        mapKit.setShowUserLocation(false);
        
    }
    else {
        
        mapKit.setShowUserLocation(true);
    }
        
    coreLocation = new ofxiPhoneCoreLocation();
    bCompass = coreLocation->startHeading();
    bGPS = coreLocation->startLocation();
     
    
    listener_x = listener_y = listener_z = UTM_x = UTM_y = 0.0;
	
	// send the OpenGL View to the front (on top of the mapview)
	ofxiPhoneSendGLViewToFront();
    ofBackground(0,0,0,0);
    ofEnableAlphaBlending();
	ofxiPhoneSetGLViewTransparent(true);
	ofxiPhoneSetGLViewUserInteraction(false);
    
    
    // image capture
    imagePicker.setMaxDimension(480);
    currentImg = currentDrawnImg = 0;
    

    // toolbar at top
	myGuiViewController	= [[MyGuiViewController alloc] initWithNibName:@"MyGuiView" bundle:nil];
	[ofxiPhoneGetGLParentView() addSubview:myGuiViewController.view];
    
    // list view for annotations
	myGuiListController	= [[MyGuiListController alloc] initWithNibName:@"MyGuiList" bundle:nil];
	[ofxiPhoneGetGLParentView() addSubview:myGuiListController.view];
    myGuiListController.view.hidden = YES;
    
    
    // audio setup
    recordImg.loadImage(ofToDataPath("rec.png"));
    currentSound = currentPlayingSound = 0;
    currentRecSample = 0;
    currentPlaySample = 0;
    addedSoundIdx = 0;
    bufferSize = 512;
    nChannels = 1;
    currentInBuffer = (float *)malloc(sizeof(float) * bufferSize * nChannels);
    currentOutBuffer = (float *)malloc(sizeof(float) * bufferSize * 2);
    pkmBinauralizerTree::initialize();
    pkmAudioWindow::initializeWindow(bufferSize);
    //	pkmBinauralSoundObject::fftInitialize();
	ofSoundStreamSetup(2, nChannels, this, 44100, bufferSize, 4);
    
    
    
    keyboard = new ofxiPhoneKeyboard(0,80,320,32);
    //keyboard->openKeyboard();
	keyboard->setVisible(false);
	keyboard->setBgColor(255, 255, 255, 255);
	keyboard->setFontColor(0,0,0, 255);
	keyboard->setFontSize(26);
    
    mapUpdateTimer.setStartTime();
    
}


//--------------------------------------------------------------
void testApp::update() {
	if(mapKit.isOpen()) {

#ifdef DEBUG_MODE
        lat = mapKit.getCenterLocation().latitude;
        lon = mapKit.getCenterLocation().longitude;

        UTM::LLtoUTM(lat, lon, UTM_y, UTM_x, UTMzone);
        listener_x = UTM_x;//ratio * listener_x + (1.0-ratio) * UTM_x;
        listener_y = UTM_y;//ratio * listener_y + (1.0-ratio) * UTM_y;
        listener_z = 0.0;
#else
        // re-center our map every update
        updateListenerPosition();
        
        if(mapUpdateTimer.getElapsedSeconds() > 1)
        {
            mapKit.setCenter(lat, lon);
            mapUpdateTimer.setStartTime();
        }
#endif
        
        soundLibrary.setListenersAbsolutePosition(listener_x, listener_y, listener_z, coreLocation->getTrueHeading());
        
        // see if we should add an annotation to the map
        if (bSelectedCamera) {
            // did the user finish taking a picture?
            if (imagePicker.imageUpdated) {
                imagePicker.close();
                
                currentImg ++;
                char buf[256];
                sprintf(buf, "Image %d", currentImg);
                string filename = string(buf) + ".png";
                pictureItem *it = new pictureItem();
                it->label = buf;
                it->sublabel = ofGetTimestampString();
                it->position = ofVec2f(lat, lon);
                it->image.setFromPixels(imagePicker.pixels, imagePicker.width, imagePicker.height, OF_IMAGE_COLOR_ALPHA);
                it->image.saveImage(ofxiPhoneGetDocumentsDirectory() + filename);
                database.push_back(it);
                
                annotationLookupTable[buf] = database.size();
                
                mapKit.addAnnotation(lat, lon, 
                                     it->label, 
                                     it->sublabel);
                
                bSelectedCamera = false;
                imagePicker.imageUpdated = false;
            }
        }
        
        if (bUserInputingSoundAnnotation) {
            if (!keyboard->isKeyboardShowing()) {
                // user finished annotation input
                bUserInputingSoundAnnotation = false;
                cout << "Finished annotation: " << keyboard->getText();
                keyboard->setVisible(false);
                
                char buf[256];
                sprintf(buf, "Sound %d", currentSound);
                
                soundItem *it = new soundItem;
                it->label = buf;
                it->sublabel = keyboard->getText();
                it->position = ofVec2f(lat, lon);
                it->filename = currentSoundFilename;
                it->addedSoundIdx = addedSoundIdx;      // corresponds to soundLibrary's internal idx of sounds
                database.push_back(it);
                
                annotationLookupTable[buf] = database.size();
                
                currentSound_x = listener_x;
                currentSound_y = listener_y;
                currentSound_z = listener_z;
                
                mapKit.addAnnotation(lat, 
                                     lon, 
                                     it->label,
                                     it->sublabel);
                
            }
        }
	}
	
}

//--------------------------------------------------------------
void testApp::draw() {
    if (bSelectedAnnotation) {
        if (bDrawImage) {
            pictureItem *it = (pictureItem *)database[currentDrawnImg - 1];
            it->image.draw(0,0);
        }
    }
    
    if (bSelectedRecord) {
        recordImg.draw(85, 160, 150, 150);
        ofPushMatrix();
        ofTranslate(0, 160+75);
        float width = 320;
        float width_step = width / (float)bufferSize;
        float amplitude = 100;
        ofBeginShape();
        ofVertex(0, 0);
        for (int i = 0; i < bufferSize; i++) {
            ofVertex(i*width_step, currentInBuffer[i]*amplitude);
        }
        ofVertex(width, 0);
        ofEndShape();
    }
    
    if(bUseLoggedGPS || bUseDebugPath)
    {
        ofSetColor(255);
        ofSetRectMode(OF_RECTMODE_CENTER);
        //ofCircle(160, 240, 5);
        pinImg.draw(160, 230, 40, 40);
        ofSetRectMode(OF_RECTMODE_CORNER);
    }
    
    
#if DEBUG_MODE
    //ofSetColor(0, 0, 0, 200);
    //ofRect(0, 415, 320, 60);
    ofSetColor(255);
    stringstream ss;
    ss << "framerate: " << ofGetFrameRate() << endl;
    ss << "latitude: " << lat << " (" << listener_x << ")" << endl;
    ss << "longitude: " << lon << " (" << listener_y << ")" << endl;
    ss << "compass: " << -coreLocation->getTrueHeading();
    ofDrawBitmapString(ss.str(), 20, 430);
#endif
    
}

void testApp::selectedPlay() {

    
}

void testApp::selectedCamera() {
    bSelectedAnnotation = false;
    bDrawImage = false;
    bSelectedRecord = false;
    bPlaySound = false;
    
    imagePicker.openCamera();
    bSelectedCamera = true;
}


void testApp::selectedRecord() {
    if (bSelectedRecord) {
        return;
    }
    
    bSelectedAnnotation = false;
    bDrawImage = false;
    bSelectedRecord = false;
    bPlaySound = false;
    
    currentSound++;
    currentRecSample = 0;
    char buf[256];
    sprintf(buf, "Sound %d", currentSound);
    currentSoundFilename = ofxiPhoneGetDocumentsDirectory() + string(buf);
    soundWriter.open(currentSoundFilename);
    
    myGuiViewController.view.hidden = YES;
    bSelectedRecord = true;
    ofxiPhoneSetGLViewUserInteraction(true);
}

void testApp::selectedCancelList() {
    bSelectedAnnotation = false;
    bDrawImage = false;
    bSelectedRecord = false;
    bPlaySound = false;
    
    myGuiViewController.view.hidden = NO;
    myGuiListController.view.hidden = YES;
    
    bDrawList = false;
}

void testApp::selectedIndex(int i) {
    cout << "index selected: " << i << endl;
    
    myGuiViewController.view.hidden = NO;
    myGuiListController.view.hidden = YES;
    
    bDrawList = false;
    
    bSelectedAnnotation = true;
    
    selectedAnnotation(database[i]->label,  database[i]->sublabel);
}

void testApp::selectedPageCurl() {
    bSelectedAnnotation = false;
    bDrawImage = false;
    bSelectedRecord = false;
    bPlaySound = false;
    
    
    [[myGuiListController table] reloadData];
    myGuiViewController.view.hidden = YES;
    myGuiListController.view.hidden = NO;

    bDrawList = true;
}

void testApp::selectedAnnotation(string title, string subtitle){
	printf("testApp::selectedAnnotation | title: %s, subtitle: %s\n", title.c_str(), subtitle.c_str());
    bDrawList = false;
    bDrawImage = false;
    bSelectedRecord = false;
    bPlaySound = false;
    
    bSelectedAnnotation = true;
    string image_str = "Image";
    string sound_str = "Sound";
    if (title.find(image_str) != string::npos) {
        bDrawImage = true;
        currentDrawnImg = annotationLookupTable[title];
        ofxiPhoneSetGLViewUserInteraction(true);
        myGuiViewController.view.hidden = YES;
    }
    else if(title.find(sound_str) != string::npos) {
        currentPlayingSound = annotationLookupTable[title];
        currentPlaySample = 0;
        soundItem *it = (soundItem *)database[currentPlayingSound];
        soundReader.open(it->filename);
        bPlaySound = true;
    }
}


void testApp::unselectedAnnotation(string title, string subtitle){
	printf("testApp::unselectedAnnotation | title: %s, subtitle: %s\n", title.c_str(), subtitle.c_str());
    
    bSelectedRecord = false;
    bPlaySound = false;
    bDrawImage = false;
    bPlaySound = false; 
}

void testApp::regionWillChange(bool animated) {
	//printf("testApp::regionWillChange | animated: %i\n", animated);
}

void testApp::regionDidChange(bool animated) {
	//printf("testApp::regionDidChange | animated: %i\n", animated);
}

void testApp::willStartLoadingMap() {
	printf("testApp::willStartLoadingMap\n");
}

void testApp::didFinishLoadingMap() {
	printf("testApp::didFinishLoadingMap\n");
}

void testApp::errorLoadingMap(string errorDescription) {
	printf("testApp::errorLoadingMap : %s\n", errorDescription.c_str());
}


void testApp::audioIn(float *buf, int size, int ch) {
    if (bSelectedRecord) {
        soundWriter.write(buf, currentRecSample, size*ch);
        currentRecSample += size*ch;
        cblas_scopy(size, buf, ch, currentInBuffer, 1);
    }
}
void testApp::audioOut(float *buf, int size, int ch) {
    if (bPlaySound) {
        soundReader.read(currentOutBuffer, currentPlaySample, size);
        cblas_scopy(size, currentOutBuffer, 1, buf, 2);
        cblas_scopy(size, currentOutBuffer, 1, buf+1, 2);
        currentPlaySample += size*ch;
        if (currentPlaySample > soundReader.mNumSamples) {
            currentPlaySample = 0;
        }
    }
    else if (bPlaySpatialization)
    {
        soundLibrary.audioRequested();
        cblas_scopy(size*ch, soundLibrary.getCurrentBuffer(), 1, buf, 1);
    }
    else {
        vDSP_vclr(buf, 1, size*ch);
    }
}

void testApp::updateListenerPosition()
{
    if(bSelectedRecord)
        return;
    float ratio = 0.5;
    
    if(bLogGPS)
    {
        lat = lat * ratio + coreLocation->getLatitude() * (1.0-ratio);
        lon = lon * ratio + coreLocation->getLongitude() * (1.0-ratio);
        float lat1, lon1;
        fprintf(gpsFileWriter, "%f,%f\n", lat , lon);
    }
    else if(bUseLoggedGPS || bUseDebugPath)
    {
        lat = lat * ratio + debugGPSCoords[currentCoord].x * (1.0-ratio);
        lon = lon * ratio + debugGPSCoords[currentCoord].y * (1.0-ratio);
        currentCoord = (currentCoord + 1) % debugGPSCoords.size();
    }
    else {
        lat = lat * ratio + coreLocation->getLatitude() * (1.0-ratio);
        lon = lon * ratio + coreLocation->getLongitude() * (1.0-ratio);
    }
    //cout << lat << " " << lon << endl;
    //update_velocity2d(kalman_filter, lat, lon, -last_update_total);
    //get_lat_long(kalman_filter, &lat, &lon);
    /*
     UTF_x = lon * 6371000.0 * M_PI / 180.0;
     listener_x = 0.4 * listener_x + 0.6 * UTF_x;
     UTF_y = log(tan(M_PI_4 + lat * M_PI / 360.0)) * 6371000.0; 
     listener_y = 0.4 * listener_y + 0.6 * (-UTF_y);
     listener_z = 0.0;
     */
    UTM::LLtoUTM(lat, lon, UTM_y, UTM_x, UTMzone);
    listener_x = UTM_x;//ratio * listener_x + (1.0-ratio) * UTM_x;
    listener_y = UTM_y;//ratio * listener_y + (1.0-ratio) * UTM_y;
    listener_z = 0.0;
}


//--------------------------------------------------------------
void testApp::touchDown(ofTouchEventArgs &touch){

    if (bSelectedAnnotation) {
        bSelectedAnnotation = false;
        myGuiViewController.view.hidden = NO;
        ofxiPhoneSetGLViewUserInteraction(false);
    }
    if (bSelectedRecord) {
        soundWriter.close();
        ofxiPhoneSetGLViewUserInteraction(false);
        bUserInputingSoundAnnotation = true;
        keyboard->setText("");
        keyboard->setVisible(true);
        keyboard->openKeyboard();
        
        myGuiViewController.view.hidden = NO;
        soundLibrary.addSound(currentSoundFilename, addedSoundIdx, currentSound_x, currentSound_y, currentSound_z);
        addedSoundIdx++;
        bPlaySpatialization = true;
        bSelectedRecord = false;
    }

}

//--------------------------------------------------------------
void testApp::touchMoved(ofTouchEventArgs &touch){

}

//--------------------------------------------------------------
void testApp::touchUp(ofTouchEventArgs &touch){

}

//--------------------------------------------------------------
void testApp::touchDoubleTap(ofTouchEventArgs &touch){
}


//--------------------------------------------------------------
void testApp::lostFocus() {
}

//--------------------------------------------------------------
void testApp::gotFocus() {
}

//--------------------------------------------------------------
void testApp::gotMemoryWarning() {
}

//--------------------------------------------------------------
void testApp::deviceOrientationChanged(int newOrientation){
}



//--------------------------------------------------------------
void testApp::touchCancelled(ofTouchEventArgs& args){

}

