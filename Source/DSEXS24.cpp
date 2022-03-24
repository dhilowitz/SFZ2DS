/*
  ==============================================================================

    DSEXS24.cpp
    Created: 16 Aug 2021 9:55:07pm
    Author:  David Hilowitz

  ==============================================================================
*/

#include "DSEXS24.h"

bool DSEXS24::loadExs(juce::File file) {
    if (!file.existsAsFile()) {
        DBG("Unable to open file <<%s>>" << file.getFullPathName());
        return false;
    }
    
    zones.clear();
    samples.clear();
    
    std::unique_ptr<juce::FileInputStream> inputStream = file.createInputStream();
    
    inputStream->setPosition(16);
    
    juce::MemoryBlock buffer (4);
    auto* magicRaw = static_cast<char*> (buffer.getData());
    inputStream->read(magicRaw, 4);
    juce::String magic = juce::String(magicRaw, 4);
    
    if (magic != "SOBT" && magic != "SOBJ" && magic != "TBOS" && magic != "JBOS") {
        return false;
    }

    bool bigEndian = false;
    if (magic == "SOBT" || magic == "SOBJ") {
        bigEndian = true;
    }

    bool is_size_expanded = false;
    inputStream->setPosition(4);
    int header_size = bigEndian ? inputStream->readIntBigEndian() : inputStream->readInt();
    if (header_size > 0x8000) {
        is_size_expanded = true;
    }

    juce::int64 i = 0;
    juce::int64 data_size = inputStream->getTotalLength();

    while (i + 84 < data_size) {
        inputStream->setPosition(i);
        juce::int64 sig = bigEndian ? inputStream->readIntBigEndian() : inputStream->readInt();

        inputStream->setPosition(i + 4);
        juce::int64 size = bigEndian ? inputStream->readIntBigEndian() : inputStream->readInt();

        inputStream->setPosition(i + 16);
//        local magic = fh:read(4)

        if (is_size_expanded && size > 0x8000) {
            size = size - 0x8000;
        }

        juce::int64 chunk_type = ((sig & 0x0F000000) >> 24);
        
        if (chunk_type == 0x01) {
            if (size < 104) {
                return false;
            }
            DBG("Zone encountered");
            zones.add(readZone(inputStream.get(), i, size + 84, bigEndian));
        } else if (chunk_type == 0x02) {
            DBG("Group encountered");
            groups.add(readGroup(inputStream.get(), i, size + 84, bigEndian));
        } else if (chunk_type == 0x03) {
            if (size != 336 && size != 592 && size != 600) {
                return false;
            }
            DBG("Sample encountered");
            samples.add(readSample(inputStream.get(), i, size + 84, bigEndian));
        } else {
            DBG("Blank chunk type");
        }
        i = i + size + 84;
    }
    
    readSequences();
    convertSeqNumbers();

    return true;
}

DSEXS24Zone DSEXS24::readZone(juce::FileInputStream *inputStream, juce::int64 i, juce::int64 size, bool bigEndian) {
    DSEXS24Zone zone;

    inputStream->setPosition(i + 8);
    zone.id  = bigEndian ? inputStream->readIntBigEndian() : inputStream->readInt();

    inputStream->setPosition(i + 20);

    zone.name = readFixedLengthString(inputStream, 64);

    inputStream->setPosition(i + 84);
    char zoneOpts = inputStream->readByte();
    zone.pitch = (zoneOpts & (1 << 1)) == 0;
    zone.oneShot = (zoneOpts & (1 << 0)) != 0;
    zone.reverse = (zoneOpts & (1 << 2)) != 0;

    inputStream->setPosition(i + 85);
    zone.key = inputStream->readByte();

    inputStream->setPosition(i + 86);
    zone.fineTuning = twosComplement(inputStream->readByte(), 8);

    inputStream->setPosition(i + 87);
    zone.pan = twosComplement(inputStream->readByte(), 8);

    inputStream->setPosition(i + 88);
    zone.volume = twosComplement(inputStream->readByte(), 8);
    inputStream->setPosition(i + 164);
    zone.coarseTuning = twosComplement(inputStream->readByte(), 8);

    inputStream->setPosition(i + 90);
    zone.keyLow = inputStream->readByte();

    inputStream->setPosition(i + 91);
    zone.keyHigh = inputStream->readByte();

    zone.velocityRangeOn = (zoneOpts & (1 << 3)) != 0;

    inputStream->setPosition(i + 93);
    zone.loVel = inputStream->readByte();

    inputStream->setPosition(i + 94);
    zone.hiVel = inputStream->readByte();

    inputStream->setPosition(i + 96);
    zone.sampleStart = bigEndian ? inputStream->readIntBigEndian() : inputStream->readInt();

    inputStream->setPosition(i + 100);
    zone.sampleEnd = bigEndian ? inputStream->readIntBigEndian() : inputStream->readInt();

    inputStream->setPosition(i + 104);
    zone.loopStart = bigEndian ? inputStream->readIntBigEndian() : inputStream->readInt();

    inputStream->setPosition(i + 108);
    zone.loopEnd = bigEndian ? inputStream->readIntBigEndian() : inputStream->readInt();

    inputStream->setPosition(i + 112);
    zone.loopCrossfade = bigEndian ? inputStream->readIntBigEndian() : inputStream->readInt();

    inputStream->setPosition(i + 117);
    char loopOpts = inputStream->readByte();
    zone.loopEnabled = (loopOpts & (1 << 0)) != 0;
    zone.loopEqualPower = (loopOpts & (1 << 1)) != 0;

    if ((zoneOpts & (1 << 6)) == 0) {
        zone.output = -1;
    } else {
        inputStream->setPosition(i + 166);
        zone.output = inputStream->readByte();
    }

    inputStream->setPosition(i + 172);
    zone.groupIndex = bigEndian ? inputStream->readIntBigEndian() : inputStream->readInt();

    inputStream->setPosition(i + 176);
    zone.sampleIndex = bigEndian ? inputStream->readIntBigEndian() : inputStream->readInt();

    zone.sampleFade = 0;
    if (size > 188) {
        inputStream->setPosition(i + 188);
        zone.sampleFade = bigEndian ? inputStream->readIntBigEndian() : inputStream->readInt();
    }

    zone.offset = 0;
    if (size > 192) {
        inputStream->setPosition(i + 192);
        zone.offset = bigEndian ? inputStream->readIntBigEndian() : inputStream->readInt();
    }
    return zone;
}


DSEXS24Group DSEXS24::readGroup(juce::FileInputStream *inputStream, juce::int64 i, juce::int64 size, bool bigEndian) {
    DSEXS24Group group;

//    inputStream->setPosition(i + 8);
//    group.id  = groupIndex;
    
    inputStream->setPosition(i + 20);
    group.name = readFixedLengthString(inputStream, 64);
    
    inputStream->setPosition(i + 84);
    group.volume = inputStream->readByte();;
    
    inputStream->setPosition(i + 85);
    group.pan = inputStream->readByte();;
    
    inputStream->setPosition(i + 86);
    group.polyphony = inputStream->readByte();
    
    // This has a value of 0. Not sure what it is
//    inputStream->setPosition(i + 87);
//    char test = inputStream->readByte();
    
    // This has a value of 1. not clue what it is
//    inputStream->setPosition(i + 88);
//    char test2 = inputStream->readByte();
    
    inputStream->setPosition(i + 89);
    group.velRangeLow = inputStream->readByte();
    inputStream->setPosition(i + 90);
    group.velRangeHigh = inputStream->readByte();
    
//    inputStream->setPosition(i + 91);
//    char test3 = inputStream->readByte();
//    int test3int = bigEndian ? inputStream->readIntBigEndian() : inputStream->readInt();
//    inputStream->setPosition(i + 92);
//    char test4 = inputStream->readByte();
//    inputStream->setPosition(i + 93);
//    char test5 = inputStream->readByte();
//    inputStream->setPosition(i + 94);
//    char test6 = inputStream->readByte();
//    inputStream->setPosition(i + 95);
//    char test7 = inputStream->readByte();
//    inputStream->setPosition(i + 96);
//    char test8 = inputStream->readByte();
//    inputStream->setPosition(i + 97);
//    char test9 = inputStream->readByte();
//    inputStream->setPosition(i + 98);
//    char test10 = inputStream->readByte();
//    inputStream->setPosition(i + 99);
//    test10 = inputStream->readByte();
    
    inputStream->setPosition(i + 157);
    group.trigger = inputStream->readByte();
    inputStream->setPosition(i + 158);
    group.output = inputStream->readByte();
    
    inputStream->setPosition(i + 164);
    group.exsSequence = bigEndian ? inputStream->readIntBigEndian() : inputStream->readInt();

    return group;
}

DSEXS24Sample DSEXS24::readSample(juce::FileInputStream *inputStream, juce::int64 i, juce::int64 size, bool bigEndian) {
    
    DSEXS24Sample sample;

    inputStream->setPosition(i + 8);
    sample.id  = bigEndian ? inputStream->readIntBigEndian() : inputStream->readInt();

    inputStream->setPosition(i + 20);
    sample.name = readFixedLengthString(inputStream, 64);

    inputStream->setPosition(i + 88);
    sample.length = bigEndian ? inputStream->readIntBigEndian() : inputStream->readInt();

    inputStream->setPosition(i + 92);
    sample.sampleRate = bigEndian ? inputStream->readIntBigEndian() : inputStream->readInt();

    inputStream->setPosition(i + 96);
    sample.bitDepth = inputStream->readByte();

    inputStream->setPosition(i + 112);
    sample.type = bigEndian ? inputStream->readIntBigEndian() : inputStream->readInt();

    inputStream->setPosition(i + 164);
    sample.filePath = readFixedLengthString(inputStream, 256);

    if (size > 420) {
        inputStream->setPosition(i + 420);
        sample.fileName = readFixedLengthString(inputStream, 256);
    } else {
        inputStream->setPosition(i + 20);
        sample.fileName = readFixedLengthString(inputStream, 64);
    }

    return sample;
}

  // if sign bit is set (128 - 255 for 8 bit)
short DSEXS24::twosComplement(short value, short bits) {
    if ((value & (1 << (bits - 1))) != 0) {
        return value - (1 << bits);
    }
    return value;
}

juce::String DSEXS24::readFixedLengthString(juce::FileInputStream *inputStream, int length) {
    juce::MemoryBlock buffer (length);
    auto* data = static_cast<char*> (buffer.getData());
    if(inputStream->read(data, length) < length) {
        return {};
    }
    return juce::String(data).trimEnd();
}

void DSEXS24::convertSeqNumbers() {
    for (int groupIndex = 0; groupIndex < groups.size(); groupIndex++) {
        DSEXS24Group &group = groups.getReference(groupIndex);
        group.seqNumber = 0;
        for (juce::Array<int> sequence: sequences) {
            if(sequence.contains(groupIndex)) {
                group.seqNumber = sequence.indexOf(groupIndex) + 1;
            }
        }
    }
}

void DSEXS24::readSequences() {
    
    // exs handles round robin samples by using groups that point to the next group, and so on
    //    until the sequence is reset by pointing to group -1;
    //    here we trace each of those chains for simple processing later

    sequences.clear();

    for (int groupIndex = 0; groupIndex < groups.size(); groupIndex++) {
        DSEXS24Group group = groups[groupIndex];
    
//        if (!group.exsSequence) {
//            continue;
//        }

        
        bool foundInSequence = false;
        for (juce::Array<int> sequence : sequences) {
            if(sequence.contains(groupIndex)) {
                foundInSequence = true;
                break;
            }
        }
        if(foundInSequence) {
            continue;
        }
        
        // trace back to the first group in the chain by looking for a group that points to this chain,
        // and repeating the process until we end up at a group that's not pointed to

        int gid = groupIndex;
        juce::Array<int> sequence;

        int cont = true;
        while (cont) {
            cont = false;
            for(int currentGroupIndex = 0; currentGroupIndex < groups.size(); currentGroupIndex++) {
                DSEXS24Group g = groups[currentGroupIndex];
                
                if (g.exsSequence == gid && (currentGroupIndex != g.exsSequence) && !sequence.contains(gid)) {
                    sequence.add(gid);
                    gid = currentGroupIndex;
                    cont = true;
                    break;
                }
            }
        }
        
        // now that we're at the start of the chain, simply follow it to the end
        sequence.clear();
        while (gid != -1 && !sequence.contains(gid)) {
//            sequence.add(gid);
            sequence.insert(0, gid);
            gid = groups[gid].exsSequence;
        }

        if (sequence.size() > 1) {
            
            sequences.add(sequence);
        }

    }
    
}
