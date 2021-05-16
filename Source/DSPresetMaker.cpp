/*
  ==============================================================================

    DSPresetMaker.cpp
    Created: 18 Oct 2020 10:30:33pm
    Author:  David Hilowitz

  ==============================================================================
*/

#include "DSPresetMaker.h"

void DSPresetMaker::parseSFZValueTree(ValueTree sfz) {
    valueTree = ValueTree("DecentSampler");
    valueTree.setProperty("pluginVersion", 1, nullptr);
    ValueTree groupsVT = ValueTree("groups");
    valueTree.appendChild(groupsVT, nullptr);
    
    for (auto sfzSection : sfz) {
        if(sfzSection.hasType("group")) {
            ValueTree dsGroup ("group");
            translateSFZGroupProperties(sfzSection, dsGroup);
            for (auto sfzRegion : sfzSection) {
                ValueTree dsSample("sample");
                translateSFZRegionProperties(sfzRegion, dsSample);
                dsGroup.appendChild(dsSample, nullptr);
            }
            groupsVT.appendChild(dsGroup, nullptr);
        }
    }
}

void DSPresetMaker::translateSFZGroupProperties(ValueTree sfzGroup, ValueTree &dsGroup) {
    for (int i = 0; i < sfzGroup.getNumProperties(); i++) {
        String key = sfzGroup.getPropertyName(i).toString();
        var value =      sfzGroup.getProperty(key);
        
        if(key == "group_label") {
            dsGroup.setProperty("name", value, nullptr);
        } else if(key == "ampeg_attack") {
            dsGroup.setProperty("attack", value, nullptr);
        } else if(key == "ampeg_release") {
            dsGroup.setProperty("release", value, nullptr);
        } else if(key == "ampeg_sustain") {
            dsGroup.setProperty("sustain", value, nullptr);
        } else if(key == "ampeg_decay") {
            dsGroup.setProperty("decay", value, nullptr);
        } else if(key == "trigger") {
            dsGroup.setProperty("trigger", value, nullptr);
        } else if(key == "volume") {
            dsGroup.setProperty("volume", value.toString() + "dB", nullptr);
        } else {
            DBG("<group> opcode " + key + " not supported.");
        }
    }
}

void DSPresetMaker::translateSFZRegionProperties(ValueTree sfzRegion, ValueTree &dsSample) {
    for (int i = 0; i < sfzRegion.getNumProperties(); i++) {
        String key = sfzRegion.getPropertyName(i).toString();
        var value =      sfzRegion.getProperty(key);
        
        if(key == "ampeg_attack") {
            dsSample.setProperty("attack", value, nullptr);
        } else if(key == "ampeg_release") {
            dsSample.setProperty("release", value, nullptr);
        } else if(key == "ampeg_sustain") {
            dsSample.setProperty("sustain", value, nullptr);
        } else if(key == "ampeg_decay") {
            dsSample.setProperty("decay", value, nullptr);
        } else if(key == "end") {
            dsSample.setProperty("end", value, nullptr);
        } else if(key == "hikey") {
            dsSample.setProperty("hiNote", value, nullptr);
        } else if(key == "hivel") {
            dsSample.setProperty("hiVel", value, nullptr);
        } else if(key == "key") {
            dsSample.setProperty("rootNote", value, nullptr);
            dsSample.setProperty("loNote", value, nullptr);
            dsSample.setProperty("hiNote", value, nullptr);
        } else if(key == "lokey") {
            dsSample.setProperty("loNote", value, nullptr);
        } else if(key == "loop_end") {
            dsSample.setProperty("loopEnd", value, nullptr);
        } else if(key == "loop_mode") {
            if(value == "loop_continuous") {
                dsSample.setProperty("loopEnabled", (value == "loop_continuous") ? "true" : "false", nullptr);
            }
        } else if(key == "loop_start") {
            dsSample.setProperty("loopStart", value, nullptr);
        } else if(key == "lovel") {
            dsSample.setProperty("loVel", value, nullptr);
        } else if(key == "offset") {
            dsSample.setProperty("start", value, nullptr);
        } else if(key == "pitch_keycenter") {
            dsSample.setProperty("rootNote", value, nullptr);
        } else if(key == "sample") {
            dsSample.setProperty("path", value, nullptr);
        } else if(key == "tune") {
            dsSample.setProperty("tuning", ((int)value)/100.0, nullptr);
        } else if(key == "volume") {
            dsSample.setProperty("volume", value.toString() + "dB", nullptr);
        } else {
            DBG("<region> opcode " + key + " not supported.");
        }
    }
}
