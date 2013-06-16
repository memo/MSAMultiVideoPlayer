#include "testApp.h"
#include "ofxXmlSettings.h"

//--------------------------------------------------------------
//--------------------------------------------------------------
class VideoEntry {
public:
    //--------------------------------------------------------------
    VideoEntry(string path, bool bAutoPlay, bool bLoop, char triggerKey, float volume, ofRectangle rect)
    :path(path), bAutoPlay(bAutoPlay), bLoop(bLoop), triggerKey(triggerKey), volume(volume), rect(rect)
    {
        if(!video.loadMovie(path)) {
            ofLogError() << "VideoEntry: error loading " << path;
        }
        reset();
        stop();
    }
    
    //--------------------------------------------------------------
    ~VideoEntry() {
        printf("Killing video %s\n", path.c_str());
    }
    
    //--------------------------------------------------------------
    void draw() {
        video.update();
        if(video.getCurrentFrame() > 0) video.draw(rect);
    }
    
    //--------------------------------------------------------------
    void trigger() {
        if(video.isPlaying()) video.stop();
        else video.play();
    }
    
    //--------------------------------------------------------------
    void reset() {
        stop();
        video.setPosition(0);
        video.setLoopState(bLoop ? OF_LOOP_NORMAL : OF_LOOP_NONE);
        video.setVolume(volume);
        
        if(rect.width == 0) rect.width = video.getWidth();
        if(rect.height == 0) rect.height = video.getHeight();
        
        start();
    }
    
    //--------------------------------------------------------------
    void start() {
        if(bAutoPlay) video.play();
    }
    
    //--------------------------------------------------------------
    void stop() {
        video.stop();
    }
    
    ofVideoPlayer video;
    string path;
    bool bAutoPlay;
    bool bLoop;
    char triggerKey;
    float volume;
    ofRectangle rect;
    
};
typedef shared_ptr<VideoEntry> VideoEntryPtr;

//--------------------------------------------------------------
//--------------------------------------------------------------


vector<VideoEntryPtr> videos;
map<char, VideoEntryPtr> videoTriggers;

//--------------------------------------------------------------
void testApp::setup(){
    ofxXmlSettings xml;
    if(!xml.load("settings.xml")) {
        ofSystemAlertDialog("Could not load settings.xml");
        ofExit();
    }
    
    if(!xml.pushTag("MSAMultiVideoPlayer")) {
        ofSystemAlertDialog("settings.xml missing 'MSAMultiVideoPlayer' tag");
        ofExit();
    }
    
    xml.pushTag("settings");
    ofColor c;
    c.setHex(ofHexToInt(xml.getValue("bgcolor", "0x000000)")));
    ofBackground(c);
    ofSetVerticalSync(xml.getValue("vsync", 1));
    ofSetFrameRate(xml.getValue("fps", 60));
    xml.popTag();
    
    
    
    if(!xml.pushTag("videos")) {
        ofSystemAlertDialog("settings.xml missing 'videos' tag");
        ofExit();
    }
    
    int numVideos = xml.getNumTags("video");
    for(int i=0; i<numVideos; i++) {
        xml.pushTag("video", i);
        string file = xml.getValue("file", "");
        bool bAutoPlay = xml.getValue("autoplay", 1);
        bool bLoop = xml.getValue("loop", 1);
        string triggerString = xml.getValue("trigger", "p");
        char triggerKey;
        if(triggerString == "SPACE") triggerKey = ' ';
        else triggerKey = triggerString[0];
        float volume = xml.getValue("volume", 1.0f);
        ofRectangle rect(
                         xml.getAttribute("position", "left", 0.0f), xml.getAttribute("position", "top", 0.0f),
                         xml.getAttribute("position", "width", 0.0f), xml.getAttribute("position", "height", 0.0f));
        
        
        videos.push_back(VideoEntryPtr(new VideoEntry(file, bAutoPlay, bLoop, triggerKey, volume, rect)));
        videoTriggers[triggerKey] = videos.back();
        
        xml.popTag();
    }
    
    xml.popTag(); // videos;
    
    
    for(int i=0; i<videos.size(); i++) videos[i]->start();
    
    
}

//--------------------------------------------------------------
void testApp::draw(){
    for(int i=0; i<videos.size(); i++) videos[i]->draw();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    switch(key) {
        case 'f':
            ofToggleFullscreen();
            break;
            
        case 'r':
            for(int i=0; i<videos.size(); i++) videos[i]->reset();
            break;
            
            
        default:
            if(videoTriggers.count(key) > 0) {
                videoTriggers[key]->trigger();
            } else {
                ofLogNotice() << "Unknown key pressed " << key;
            }
    }
}
