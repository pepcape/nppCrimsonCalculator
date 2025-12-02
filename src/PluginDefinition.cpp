//this file is part of notepad++
//Copyright (C)2022 Don HO <don.h@free.fr>
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
    // index 0: our Ctrl+Enter command
    ShortcutKey* sk = new ShortcutKey;
    sk->_isCtrl = false;
    sk->_isAlt = true;
    sk->_isShift = false;
    sk->_key = VK_RETURN;   // Enter key

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
    setCommand(0, TEXT("Crimson Calculator"), calculate, sk, false);
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
void calculate()
{
    // 1) Get current Scintilla view (main or secondary)
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return;

    HWND hSci = (which == 0) ? nppData._scintillaMainHandle
        : nppData._scintillaSecondHandle;

    // 2) Get current caret position and line start
    const Sci_Position curPos =
        (Sci_Position)::SendMessage(hSci, SCI_GETCURRENTPOS, 0, 0);
    const Sci_Position line =
        (Sci_Position)::SendMessage(hSci, SCI_LINEFROMPOSITION, curPos, 0);
    const Sci_Position lineStart =
        (Sci_Position)::SendMessage(hSci, SCI_POSITIONFROMLINE, line, 0);

    if (curPos < lineStart)
        return; // should not happen, but just in case

    const Sci_Position prefixLen = curPos - lineStart;

    // 3) Read text from lineStart .. curPos (prefix of the line)
    // prefixLen is (curPos - lineStart)

    std::string prefix;
    // allocate buffer: prefix + terminating 0
    prefix.resize(static_cast<size_t>(prefixLen) + 1);

    Sci_TextRangeFull tr;
    tr.chrg.cpMin = static_cast<Sci_PositionCR>(lineStart);
    tr.chrg.cpMax = static_cast<Sci_PositionCR>(curPos);

    // non-const buffer for Scintilla to write into
    tr.lpstrText = const_cast<char*>(prefix.data());

    ::SendMessage(hSci, SCI_GETTEXTRANGEFULL, 0, reinterpret_cast<LPARAM>(&tr));

    // make sure it's terminated exactly where we expect
    prefix[static_cast<size_t>(prefixLen)] = '\0';

    // 4) Insert newline at caret and copy the prefix onto the new line

    ::SendMessage(hSci, SCI_BEGINUNDOACTION, 0, 0);

    // insert newline at current position
    ::SendMessage(hSci, SCI_REPLACESEL, 0, (LPARAM)"\n");

    // after SCI_REPLACESEL, caret is just after the newline – get new position
    Sci_Position newPos =
        (Sci_Position)::SendMessage(hSci, SCI_GETCURRENTPOS, 0, 0);

    // insert the prefix on the new line
    ::SendMessage(hSci, SCI_INSERTTEXT, (WPARAM)newPos, (LPARAM)prefix.c_str());
    newPos += prefixLen;

    // insert "= "
    ::SendMessage(hSci, SCI_INSERTTEXT, (WPARAM)newPos, (LPARAM)" = ");
    newPos += 3;

    // move caret to the end of inserted prefix (optional, but nice)
    ::SendMessage(hSci, SCI_GOTOPOS, (WPARAM)newPos, 0);

    ::SendMessage(hSci, SCI_ENDUNDOACTION, 0, 0);
}
