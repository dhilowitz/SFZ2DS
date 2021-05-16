/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "DSSFZ.h"
#include "DSPresetMaker.h"

//==============================================================================
int main (int argc, char* argv[])
{
    // Check the number of parameters
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <sfz-file> <ds-file>" << std::endl;
        return 1;
    }

    File inputFile = File(argv[1]);
    if(!inputFile.existsAsFile()) {
        std::cerr << "\"" << argv[1] << "\" is not a file." << std::endl;
        return 2;
    }
    
    DSSFZ sfz;
    sfz.parseFromFile(inputFile);
    
    DSPresetMaker presetMaker;
    presetMaker.parseSFZValueTree(sfz.getValueTree());
    DBG(presetMaker.getXML());
    
    File outputFile = File::getCurrentWorkingDirectory().getChildFile(argv[2]);
    if(outputFile.existsAsFile()) {
        outputFile.deleteFile();
//        std::cerr << "File \"" << argv[1] << "\" exists already." << std::endl;
//        return 3;
    }

    outputFile.create();
    outputFile.appendText(presetMaker.getXML());

    return 0;
}
