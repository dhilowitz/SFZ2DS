/*
  ==============================================================================

    DSPresetMaker.h
    Created: 18 Oct 2020 10:30:32pm
    Author:  David Hilowitz

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class DSPresetConverter {
public:
    void parseSFZValueTree(ValueTree valueTree);
    String getXML() {
        return valueTree.toXmlString();
    }
    
    enum HeaderLevel {
        headerLevelGlobal,
        headerLevelGroup,
        headerLevelRegion
    };
private:
    ValueTree valueTree;
    void translateSFZRegionProperties(ValueTree sfzRegion, ValueTree &dsSample, HeaderLevel level);
};
