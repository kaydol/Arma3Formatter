//this file is part of notepad++
//Copyright (C)2003 Don HO <donho@altern.org>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "PluginDefinition.h"
#include "menuCmdID.h"
#include <string>
#include <list>

//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;

//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE /*hModule*/)
{
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{

    //--------------------------------------------//
    //-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
    //--------------------------------------------//
    // with function :
    // setCommand(int index,                      // zero based number to indicate the order of command
    //            TCHAR *commandName,             // the command name that you want to see in plugin menu
    //            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
    //            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
    //            bool check0nInit                // optional. Make this menu item be checked visually
    //            );
    setCommand(0, TEXT("Format setParticleParams argument"), setParticlesParamsFormatter, NULL, false);
    //setCommand(1, TEXT("Hello (with dialog)"), helloDlg, NULL, false);
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
}


//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit) 
{
    if (index >= nbFunc)
        return false;

    if (!pFunc)
        return false;

    lstrcpy(funcItem[index]._itemName, cmdName);
    funcItem[index]._pFunc = pFunc;
    funcItem[index]._init2Check = check0nInit;
    funcItem[index]._pShKey = sk;

    return true;
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//
void setParticlesParamsFormatter()
{
    // Get the current scintilla
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return;
    HWND curScintilla = (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;

    // Get selected text
    size_t start = ::SendMessage(curScintilla, SCI_GETSELECTIONSTART, 0, 0);
    size_t end = ::SendMessage(curScintilla, SCI_GETSELECTIONEND, 0, 0);
    if (end < start)
    {
        size_t tmp = start;
        start = end;
        end = tmp;
    }

    size_t textLen = end - start;
    if (textLen == 0) return;

    char* pText = new char[textLen + 1];
    ::SendMessage(curScintilla, SCI_GETSELTEXT, 0, (LPARAM)pText);

    std::list<std::string> annotations = { 
        "p3dPath, Ntieth, Index, FrameCount, Loop", 
        "animationName (obsolete parameter that was meant to play .rtm anims, should be empty)",
        "particleType (\"Billboard\" or \"SpaceObject\")",
        "timerPeriod",
        "lifeTime",
        "position",
        "moveVelocity",
        "rotationVelocity (rotations per second)",
        "weight (weight of the particle, kg)",
        "volume (volume of the particle in m3)",
        "rubbing (determines how much particles blown by winds)",
        "size (array of particle size along its lifetime)",
        "color (array of [RGBA] arrays)",
        "animationSpeed",
        "randomDirectionPeriod",
        "randomDirectionIntensity",
        "onTimerScript",
        "beforeDestroyScript",
        "this (if this parameter isn't objNull, the particle source will be attached to the object, the generation of particles stops when beyond Object View Distance)",
        "angle (optional)",
        "onSurface (optional)",
        "bounceOnSurface (optional, default -1. Coef of bounce in collision with ground, 0..1 for collisions, -1 to disable collision. Should be used soberly as it has a significant impact on performance)",
        "emissiveColor (optional, array of [RGBA] arrays, needs to match 'color' values and be 100x times the RGB color values. Alpha is not used)",
        "vectorDir (optional, sets the default direction for SpaceObject particles)"
    };

    // Adding annotations to each parameter followed by ','
    std::string newText = "";
    int squareBrackets = 0;
    for (size_t i = 0; i < textLen; ++i) {
        newText += pText[i];
        if (pText[i] == '[') ++squareBrackets;
        if (pText[i] == ']') --squareBrackets;
        if (pText[i] == ',' && squareBrackets == 1) { 
            std::string annotation = "";
            if (!annotations.empty()) {
                annotation = annotations.front();
                annotations.pop_front();
                annotation = " // " + annotation + "\n";
            }
            newText += annotation;
        }
    }

    // Edge case. The last parameter does not have an ending ',' 
    if (!annotations.empty()) { // No sense inserting something if there are no more annotations left
        bool insert = false;
        for (size_t i = newText.length()-1; i > 0; --i) {
            if (insert) {
                std::string annotation = "";
                if (!annotations.empty()) {
                    annotation = annotations.front();
                    annotations.pop_front();
                    annotation = " // " + annotation + "\n";
                }
                newText = newText.insert(i+1, annotation.c_str());
                break;
            }
            if (newText[i] == ']') insert = true;
        }
    }

    ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)newText.c_str());
    
    delete[] pText;
}
