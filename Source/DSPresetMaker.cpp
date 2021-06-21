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
    translateSFZRegionProperties(sfz, groupsVT, headerLevelGlobal);
    valueTree.appendChild(groupsVT, nullptr);
    
    for (auto sfzSection : sfz) {
        if(sfzSection.hasType("group")) {
            ValueTree dsGroup ("group");
            translateSFZRegionProperties(sfzSection, dsGroup, headerLevelGroup);
            for (auto sfzRegion : sfzSection) {
                ValueTree dsSample("sample");
                translateSFZRegionProperties(sfzRegion, dsSample, headerLevelRegion);
                dsGroup.appendChild(dsSample, nullptr);
            }
            groupsVT.appendChild(dsGroup, nullptr);
        }
    }
}

void DSPresetMaker::translateSFZRegionProperties(ValueTree sfzRegion, ValueTree &dsEntity, HeaderLevel level) {
    for (int i = 0; i < sfzRegion.getNumProperties(); i++) {
        String key = sfzRegion.getPropertyName(i).toString();
        var value =      sfzRegion.getProperty(key);
        
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
