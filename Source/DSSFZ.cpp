/*
  ==============================================================================

    DSSFZ.cpp
    Created: 13 Oct 2020 1:28:26pm
    Author:  David Hilowitz

  ==============================================================================
*/

#include "DSSFZ.h"

void DSSFZ::skipComment() {
    while(position < fileString.length()) {
        if(position+1 >= fileString.length()) {
            return;
        }
        
        if(fileString[position] != '/' || fileString[position+1] != '/') {
            return;
        }
        
        while(position < fileString.length()) {
            wchar_t thisChar = fileString[position];
            if(thisChar == '\r' || thisChar == '\n' ) {
                break;
            }
            position++;
        }
        
        skipNewLines(true);
    }
}

void DSSFZ::skipNewLines(bool andSpaces) {
    for (; position < fileString.length(); position++) {
        wchar_t curChar = fileString[position];
        if((andSpaces && curChar == ' ') || curChar == '\r' || curChar == '\n' ) {
            continue;
        }
        break;
    }
}

void DSSFZ::parseFromFile(File file) {
    valueTree = ValueTree("sfz");
    
    std::unique_ptr<FileInputStream> inputStream = file.createInputStream();
    
    fileString = inputStream->readEntireStreamAsString();
    
    String tokenString = {};
    enum token_type_t { HEADER, OPCODE };
    token_type_t tokenType = HEADER;
    
    for (position = 0; position < fileString.length(); position++) {
        wchar_t curChar = fileString[position];
        
        skipNewLines(true);
        skipComment();
        
        if(position >= fileString.length()) { return; }
        
        String token;
        
        curChar = fileString[position];
        switch(curChar) {
            case '<':
                if(tokenString.isNotEmpty()) {
                    switch (tokenType)
                    {
                    case HEADER:
                        pushHeader(tokenString.trim());
                        break;
                    case OPCODE:
                        pushOpcode(tokenString.trim());
                        break;
                    }
                    tokenString.clear();
                }
                
                // Get the entire next token
                for (; position < fileString.length(); position++) {
                    curChar = fileString[position];
                    if(curChar == '>') {
                        token << curChar;
                        break;
                    }
                    
                    if(curChar == '\r' || curChar == '\n') {
                        break;
                    }
                    token << curChar;
                    
                }
                // process the header token
                tokenString = tokenString + token;
                tokenType = HEADER;
                break;
            default:
                if (tokenString.isNotEmpty())
                {
                    switch (tokenType) {
                        case HEADER:
                            pushHeader(tokenString.trim());
                            break;
                        case OPCODE:
                            pushOpcode(tokenString.trim());
                            break;
                    }
                    tokenString.clear();
                }
                
                // Get the entire next opcode
                String opcodeString = {};
                for (; position < fileString.length(); position++) {
                    curChar = fileString[position];
                    if(curChar == '<' /*|| curChar == '\r' || curChar == '\n'*/) {
                        position--;
                        break;
                    }
                    opcodeString << curChar;
                }
                
                if(opcodeString.indexOf("=") == -1) {
                    DBG("Opcode is missing an '=' in it: " + opcodeString);
                    break;
                }
                
                StringArray opcodeStringTokens; opcodeStringTokens.addTokens(opcodeString, "=", "");
                
                if (opcodeStringTokens.size() <= 2) {
                    token = opcodeString;
                } else if(opcodeStringTokens.size() >= 2){
                    String key = opcodeStringTokens[0];
                    int lastSpace = opcodeStringTokens[1].lastIndexOfChar(' ');
                    int lastNNewLineChar = opcodeStringTokens[1].lastIndexOfChar('\n');
                    int lastRNewlineChar = opcodeStringTokens[1].lastIndexOfChar('\r');
                    
                    
                    auto chooseFirstViableIndex = [](int num1, int num2){
                        if(num1 == -1) return num2;
                        if(num2 == -1) return num1;
                        return juce::jmin(num1, num2);
                    };
                    
                    int endOfValue = lastSpace;
                    endOfValue = chooseFirstViableIndex(endOfValue, lastNNewLineChar);
                    endOfValue = chooseFirstViableIndex(endOfValue, lastRNewlineChar);
                    
                    if(endOfValue < 1) {
                        fprintf(stderr, "ERROR: Unable to determine the end of the opcode's value.\n");
                        break;
                    }
                    
                    String value = opcodeStringTokens[1].substring(0, endOfValue);
                    token = key + "=" + value;
                }
                
                // process the header token
                tokenString = tokenString + token;
                tokenType = OPCODE;
                position = position - opcodeString.length() + token.length();
                break;
        }
    }
    
    if (!tokenString.isEmpty())
    {
        switch (tokenType)
        {
        case HEADER:
            pushHeader(tokenString.trim());
            break;
        case OPCODE:
            pushOpcode(tokenString.trim());
            break;
        }
        tokenString.clear();
    }
}

void DSSFZ::pushHeader(String token) {
    if (token == "<global>")
    {
        currentSection = GLOBAL;
    } else if (token == "<group>")
    {
        currentSection = GROUP;
        currentGroup = ValueTree("group");
        valueTree.appendChild(currentGroup, nullptr);
    }
    else if (token == "<region>")
    {
        currentSection = REGION;
        currentRegion = ValueTree("region");
        if(!currentGroup.isValid()) {
            currentSection = GROUP;
            currentGroup = ValueTree("group");
            valueTree.appendChild(currentGroup, nullptr);
        }
        currentGroup.appendChild(currentRegion, nullptr);
    } else
    {
        currentSection = UNKNOWN;
        std::cerr << "The header '" << token << "' is unsupported by SFZ2DS!" << std::endl;
    }
}

void DSSFZ::pushOpcode(String token) {
    if (currentSection == UNKNOWN)
        return;
    
    int equalsIndex = token.indexOf("=");
    if (equalsIndex == -1) {
        std::cerr << "The token '" << token << "' doesn't have an '=' in it." << std::endl;
        return;
    }
    String key = token.substring(0, equalsIndex);
    String value = token.substring(equalsIndex+1).trimEnd();
    
    if(currentSection == GLOBAL) {
        valueTree.setProperty(key, value, nullptr);
    } else if (currentSection == GROUP) {
        currentGroup.setProperty(key, value, nullptr);
    } else if (currentSection == REGION) {
        currentRegion.setProperty(key, value, nullptr);
    }
}
