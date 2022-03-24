/*
  ==============================================================================

    DSEXS24.h
    Created: 16 Aug 2021 9:55:07pm
    Author:  David Hilowitz

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct DSEXS24Zone {
    int id;
    juce::String name;
    bool pitch;
    bool oneShot;
    bool reverse;
    short key;
    short fineTuning;
    short pan;
    short volume;
    short coarseTuning;
    short keyLow;
    short keyHigh;
    bool velocityRangeOn = false;
    short loVel;
    short hiVel;
    int sampleStart;
    int sampleEnd;
    int loopStart;
    int loopEnd;
    int loopCrossfade;
    bool loopEnabled;
    bool loopEqualPower;
    short output = -1;
    int groupIndex;
    int sampleIndex;
    int sampleFade;
    int offset;
};

struct DSEXS24Group {
//    int id;
    juce::String name;
    short volume =0;
    short pan =0;
    short polyphony;
    short velRangeLow;
    short velRangeHigh;
    short trigger;
    short output;
    int exsSequence;
    int seqNumber;
};

struct DSEXS24Sample {
    int id;
    juce::String name;
    int length;
    int sampleRate;
    short bitDepth;
    int type;
    juce::String filePath;
    juce::String fileName;
};

class DSEXS24 {
public:
    DSEXS24() {}
    bool loadExs(juce::File file);
    juce::Array<DSEXS24Zone> & getZones() { return zones; }
    juce::Array<DSEXS24Group> & getGroups() { return groups; }
    juce::Array<DSEXS24Sample> & getSamples() { return samples; }
private:
    juce::Array<DSEXS24Zone> zones;
    juce::Array<DSEXS24Group> groups;
    juce::Array<DSEXS24Sample> samples;
    juce::Array<juce::Array<int>> sequences;
    
    juce::String readFixedLengthString(juce::FileInputStream *inputStream, int length);
    short twosComplement(short value, short bits);
    DSEXS24Zone readZone(juce::FileInputStream *inputStream, juce::int64 i, juce::int64 size, bool bigEndian);
    DSEXS24Group readGroup(juce::FileInputStream *inputStream, juce::int64 i, juce::int64 size, bool bigEndian);
    DSEXS24Sample readSample(juce::FileInputStream *inputStream, juce::int64 i, juce::int64 size, bool bigEndian);
    void readSequences();
    void convertSeqNumbers();
};


