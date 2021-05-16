/*
  ==============================================================================

    DSPresetMaker.h
    Created: 18 Oct 2020 10:30:32pm
    Author:  David Hilowitz

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class DSPresetMaker {
public:
    void parseSFZValueTree(ValueTree valueTree);
    String getXML() {
        return valueTree.toXmlString();
    }
private:
    ValueTree valueTree;
    
    void translateSFZGroupProperties(ValueTree sfzGroup, ValueTree &dsGroup);
    void translateSFZRegionProperties(ValueTree sfzRegion, ValueTree &dsSample);
};
