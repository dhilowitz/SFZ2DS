/*
  ==============================================================================

    Created: 18 Oct 2020 10:30:33pm
    DSPresetConverter.cpp
    Author:  David Hilowitz

  ==============================================================================
*/

#include "DSPresetConverter.h"

void DSPresetConverter::parseSFZValueTree(juce::ValueTree sfz) {
    valueTree = juce::ValueTree("DecentSampler");
    valueTree.setProperty("pluginVersion", 1, nullptr);
    juce::ValueTree groupsVT = juce::ValueTree("groups");
    translateSFZRegionProperties(sfz, groupsVT, headerLevelGlobal);
    valueTree.appendChild(groupsVT, nullptr);
    
    for (auto sfzSection : sfz) {
        if(sfzSection.hasType("group")) {
            juce::ValueTree dsGroup ("group");
            translateSFZRegionProperties(sfzSection, dsGroup, headerLevelGroup);
            for (auto sfzRegion : sfzSection) {
                juce::ValueTree dsSample("sample");
                translateSFZRegionProperties(sfzRegion, dsSample, headerLevelRegion);
                dsGroup.appendChild(dsSample, nullptr);
            }
            groupsVT.appendChild(dsGroup, nullptr);
        }
    }
}

void DSPresetConverter::translateSFZRegionProperties(juce::ValueTree sfzRegion, juce::ValueTree &dsEntity, HeaderLevel level) {
    for (int i = 0; i < sfzRegion.getNumProperties(); i++) {
        juce::String key = sfzRegion.getPropertyName(i).toString();
        juce::var value =      sfzRegion.getProperty(key);
        
        if(level == headerLevelGroup && key == "group_label") {
            dsEntity.setProperty("name", value, nullptr);
        } else if(key == "amp_veltrack") {
            dsEntity.setProperty("ampVelTrack", value, nullptr);
        } else if(key == "ampeg_attack") {
            dsEntity.setProperty("attack", value, nullptr);
        } else if(key == "ampeg_release") {
            dsEntity.setProperty("release", value, nullptr);
        } else if(key == "ampeg_sustain") {
            dsEntity.setProperty("sustain", value, nullptr);
        } else if(key == "ampeg_decay") {
            dsEntity.setProperty("decay", value, nullptr);
        } else if(key == "group") {
            dsEntity.setProperty("tags", "voice-group-" + value.toString(), nullptr);
        } else if(key == "end") {
            dsEntity.setProperty("end", value, nullptr);
        } else if(key == "hikey") {
            dsEntity.setProperty("hiNote", value, nullptr);
        } else if(key == "hivel") {
            dsEntity.setProperty("hiVel", value, nullptr);
        } else if(key == "key") {
            dsEntity.setProperty("rootNote", value, nullptr);
            dsEntity.setProperty("loNote", value, nullptr);
            dsEntity.setProperty("hiNote", value, nullptr);
        } else if(key == "lokey") {
            dsEntity.setProperty("loNote", value, nullptr);
        } else if(key == "loop_end") {
            dsEntity.setProperty("loopEnd", value, nullptr);
        } else if(key == "loop_mode") {
            if(value == "loop_continuous") {
                dsEntity.setProperty("loopEnabled", (value == "loop_continuous") ? "true" : "false", nullptr);
            }
        } else if(key == "loop_start") {
            dsEntity.setProperty("loopStart", value, nullptr);
        } else if(key == "lovel") {
            dsEntity.setProperty("loVel", value, nullptr);
        } else if(key == "off_by") {
            dsEntity.setProperty("silencedByTags", "voice-group-" + value.toString(), nullptr);
        } else if(key == "off_mode") {
            dsEntity.setProperty("silencingMode", value, nullptr);
        } else if(key == "offset") {
            dsEntity.setProperty("start", value, nullptr);
        } else if(key == "pitch_keycenter") {
            dsEntity.setProperty("rootNote", value, nullptr);
        } else if(key == "sample") {
            dsEntity.setProperty("path", value, nullptr);
        } else if(key == "sw_previous") {
            dsEntity.setProperty("previousNote", value, nullptr);
        } else if(key == "trigger") {
            dsEntity.setProperty("trigger", value, nullptr);
        } else if(key == "tune") {
            dsEntity.setProperty("tuning", ((int)value)/100.0, nullptr);
        } else if(key == "volume") {
            dsEntity.setProperty("volume", value.toString() + "dB", nullptr);
        } else {
            switch (level) {
                case headerLevelGlobal:
                    DBG("<global> opcode " + key + " not supported.");
                    break;
                case headerLevelGroup:
                    DBG("<group> opcode " + key + " not supported.");
                    break;
                case headerLevelRegion:
                default:
                    DBG("<region> opcode " + key + " not supported.");
                    break;
            }
        }
    }
}

void DSPresetConverter::parseDSEXS24(DSEXS24 exs24, juce::String samplePath, juce::File outputDir) {
    juce::Array<DSEXS24Zone> &zones = exs24.getZones();
    juce::Array<DSEXS24Group> &groups = exs24.getGroups();
    juce::Array<DSEXS24Sample> &samples = exs24.getSamples();
    valueTree = juce::ValueTree("DecentSampler");
    valueTree.setProperty("pluginVersion", 1, nullptr);
    juce::ValueTree groupsVT = juce::ValueTree("groups");
    valueTree.appendChild(groupsVT, nullptr);
    
    for (DSEXS24Zone zone : zones) {
        if(zone.groupIndex < 0) {
            continue;
        } else if(zone.groupIndex > 100) {
            printf("This zone's group index is greater than 100. This converter may not support this file.");
            continue;
        }
        while (zone.groupIndex >= groups.size()) {
            DSEXS24Group newGroup;
            newGroup.name = "Couldn't find group index " + juce::String(zone.groupIndex);
            groups.add(newGroup);
        }
    }
    
    // Iterate through the groups and add their respective samples
    for(int groupIndex = -1; groupIndex < groups.size(); groupIndex++) {
        bool hasSamples = false;
        DSEXS24Group group = (groupIndex >= 0) ? groups[groupIndex] : DSEXS24Group();
        juce::ValueTree dsGroup ("group");
        if(group.name != "") {
            dsGroup.setProperty("name", group.name, nullptr);
        }
//        dsGroup.setProperty("_exsGroupIndex", groupIndex, nullptr);
        if(group.pan != 0) {
            dsGroup.setProperty("pan", group.pan, nullptr);
        }
        if(group.volume != 0) {
            dsGroup.setProperty("volume", juce::String(group.volume) + "dB", nullptr);
        }
        
        if(group.seqNumber != 0) {
            dsGroup.setProperty("seqPosition", group.seqNumber, nullptr);
        }
//        dsGroup.setProperty("exsSequence", group.exsSequence, nullptr);
        
        for (DSEXS24Zone zone : zones) {
            if (zone.groupIndex == groupIndex) {
                juce::ValueTree dsSample("sample");
                
                int sampleIndex = zone.sampleIndex;
                if(sampleIndex < 0 || sampleIndex >= samples.size()) {
                    continue;
                }
                
                DSEXS24Sample &sample = samples.getReference(sampleIndex);
                if(samplePath.isNotEmpty()) {
                    juce::File sampleFile = juce::File::getCurrentWorkingDirectory().getChildFile (samplePath).getChildFile(sample.fileName);
                    dsSample.setProperty("path", sampleFile.getRelativePathFrom(outputDir), nullptr);
                } else {
                    juce::File sampleFile = juce::File(sample.filePath).getChildFile(sample.fileName);
                    dsSample.setProperty("path", sampleFile.getRelativePathFrom(outputDir), nullptr);
                }
                    
                dsSample.setProperty("name", zone.name, nullptr);
                if(zone.pitch == false) {
                    dsSample.setProperty("pitchKeyTrack", 0, nullptr);
                }
                dsSample.setProperty("rootNote", zone.key, nullptr);
                dsSample.setProperty("loNote", zone.keyLow, nullptr);
                dsSample.setProperty("hiNote", zone.keyHigh, nullptr);
                dsSample.setProperty("loVel", zone.velocityRangeOn ? zone.loVel : 0, nullptr);
                dsSample.setProperty("hiVel", zone.velocityRangeOn ? zone.hiVel : 127, nullptr);
                
                float tuning = (float) zone.coarseTuning + (((float)zone.fineTuning)/100.0f);
                if(tuning != 0) {
                    dsSample.setProperty("tuning", tuning, nullptr);
                }
                if(zone.pan != 0) {
                    dsSample.setProperty("pan", zone.pan, nullptr);
                }
                if(zone.volume != 0) {
                    dsSample.setProperty("volume", juce::String(zone.volume) + "dB", nullptr);
                }
                
                if(zone.sampleStart != 0) {
                    dsSample.setProperty("start", zone.sampleStart, nullptr);
                }
                
                if(zone.sampleEnd != 0) {
                    dsSample.setProperty("end", zone.sampleEnd, nullptr);
                }
                
                if(zone.loopEnabled) {
                    dsSample.setProperty("loopEnabled", zone.loopEnabled, nullptr);
                    dsSample.setProperty("loopStart", zone.loopStart, nullptr);
                    dsSample.setProperty("loopEnd", zone.loopEnd, nullptr);
                
                    if(zone.loopCrossfade != 0) {
                        dsSample.setProperty("loopCrossfade", zone.loopCrossfade, nullptr);
                    }
                    
                    dsSample.setProperty("loopCrossfadeMode", zone.loopEqualPower ? "equal_power" : "linear", nullptr);
                }
                
                
                hasSamples = true;
                dsGroup.appendChild(dsSample, nullptr);
            }
        }
        if(hasSamples) {
            groupsVT.appendChild(dsGroup, nullptr);
        }
    }
    
}

