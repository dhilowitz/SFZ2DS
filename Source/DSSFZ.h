/*
  ==============================================================================

    DSSFZ.h
    Created: 13 Oct 2020 1:28:26pm
    Author:  David Hilowitz

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class DSSFZ {
public:
    void parseFromFile(juce::File file);
    
    ValueTree getValueTree() { return valueTree; }
    String getXML() { return valueTree.toXmlString();}
private:
    ValueTree valueTree;
    ValueTree currentGroup;
    ValueTree currentRegion;
    String fileString;
    int position;
    
    enum sectionType { UNKNOWN, GROUP, REGION, GLOBAL };
    sectionType currentSection;
    
    void pushHeader(String);
    void pushOpcode(String);
    void skipNewLines(bool andSpaces);
    void skipComment();
};
