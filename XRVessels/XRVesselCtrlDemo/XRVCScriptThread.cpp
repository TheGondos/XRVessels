/**
  XR Vessel add-ons for OpenOrbiter Space Flight Simulator
  Copyright (C) 2006-2021 Douglas Beachy

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.

  Email: mailto:doug.beachy@outlook.com
  Web: https://www.alteaaerospace.com
**/

//-------------------------------------------------------------------------
// XRVCScriptThread.cpp : Implementation of XRVCScriptThread class.
//-------------------------------------------------------------------------

#include "XRVCScriptThread.h"
#include <cassert>

//
// This class handles script browsing and I/O in a thread so we don't block Orbiter's
// main thread while we are waiting for user input.
//

// Constructor
XRVCScriptThread::XRVCScriptThread(const HWND hwndMainDialog) : 
    m_hwndMainDialog(hwndMainDialog), m_terminateEST(false)
{ 
    InitializeCriticalSection(&m_criticalSectionST);
    m_hEventScriptFile =  CreateEvent(nullptr, TRUE, FALSE, nullptr);  // manual-reset event
    m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 8192, ScriptThread, this, 0, nullptr));
}

// Destructor
XRVCScriptThread::~XRVCScriptThread() 
{ 
    // signal thread to exit and wait until it does
    EnterCriticalSection(&m_criticalSectionST);  // lock for write
    m_terminateEST = true;   
    LeaveCriticalSection(&m_criticalSectionST);  // unlock    
    SetEvent(m_hEventScriptFile);           // wake up the thread if it's asleep
    WaitForSingleObject(m_hThread, 5000);   // wait up to 5 seconds (should always return virtually immediately, however)
    
    CloseHandle(m_hThread);
    CloseHandle(m_hEventScriptFile);  
    DeleteCriticalSection(&m_criticalSectionST);
}

//*************************************************************************
// Static methods that run as the thread
//*************************************************************************

//==============================================================
// Thread to handle file browse/load/run a script
// Returns 0.
//==============================================================
unsigned __stdcall XRVCScriptThread::ScriptThread(void *pParameter)
{
    XRVCScriptThread *pSingleton = static_cast<XRVCScriptThread *>(pParameter);

    // main loop while we wait for work
    for (;;)
    {
        // reset our event to non-signaled and go back to sleep while we wait for more work
        ResetEvent(pSingleton->m_hEventScriptFile);

        if (WaitForSingleObject(pSingleton->m_hEventScriptFile, INFINITE) != WAIT_OBJECT_0)  
        {
            // something bad happened, so exit
            break;
        }

        if (pSingleton->m_terminateEST)
            break;      // main thread signaled us to close

        bool fileOK = false; 
        char filename[MAX_PATH];
        
        // see if the user already sent in a script to execute 
        EnterCriticalSection(&pSingleton->m_criticalSectionST);  // lock for read & write
        if (!pSingleton->m_csScriptToExecute.IsEmpty())
        {
            // main thread passed in a filename: latch the value and reset it to signal that we got it
            strcpy_s(filename, pSingleton->m_csScriptToExecute);  
            pSingleton->m_csScriptToExecute.Empty();
            fileOK = true;
        }
        LeaveCriticalSection(&pSingleton->m_criticalSectionST);  // unlock    

        OPENFILENAME ofn = { 0 };
        ofn.lpstrFile = filename;   // point to our filename buffer
        
        if (!fileOK)
        {
            // User is requesting a file browse dialog to open a script file
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = pSingleton->m_hwndMainDialog;
            *ofn.lpstrFile = 0;   // do not pre-populate the filename
            ofn.nMaxFile = sizeof(filename);
            ofn.lpstrFilter = "XRVesselCtrl Script Files\0*.xrvc\0All\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = nullptr;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = ".";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            // Display the Open dialog box. 
            fileOK = (GetOpenFileName(&ofn) == TRUE);
        }

        if (fileOK) 
        {
            FILE *pFile = nullptr;

            // open the script file
            if (fopen_s(&pFile, ofn.lpstrFile, "rt") != 0)
            {
                CString msg;
                msg.Format("Could not open script file '%s'.", ofn.lpstrFile);
                ST_SendStatusMessage(*pSingleton, msg);   // tell the user about it
                continue;   // go back to sleep
            }

            // inform the user of successful open
            CString msg;
            msg.Format("Parsing script [%s]", ofn.lpstrFile);
            ST_SendStatusMessage(*pSingleton, msg);

            // read all lines from the script file
            vector<CString> commandList;
            int commandCount = ST_ParseScriptFile(pFile, commandList);
            fclose(pFile);

            if (commandCount < 0)
            {
                CString msg;
                msg.Format("Error reading script file '%s'.", ofn.lpstrFile);
                ST_SendStatusMessage(*pSingleton, msg);   // tell the user about it
                continue;   // go back to sleep
            }
            else if (commandCount == 0)
            {
                CString msg;
                msg.Format("Error: script file '%s' is empty (no commands).", ofn.lpstrFile);
                ST_SendStatusMessage(*pSingleton, msg);   // tell the user about it
                continue;  // go back to sleep
            }

            // send the command list back to the main thread
            if (!ST_SendCommands(*pSingleton, commandList))
            {
                // should never happen since 'Execute Script' button is disabled until we reset our m_hEventExecuteScriptFile event!
                // Since the main thread is stuck in a loop, we have to use a message box here.
                assert(false);
                MessageBox(pSingleton->m_hwndMainDialog, "Internal error: could not execute script - main thread is busy!", "XRVesselCtrl Script Thread Error", MB_OK | MB_SETFOREGROUND);
                // fall through and go back to sleep
            }
        }
        else
        {
            // 'cancel' selected or our parent window is terminating
            // fall through and return to the top of the loop
        }
    }

    return 0;
}

//===========================================================
// Send a status message back to the main thread
//===========================================================
void XRVCScriptThread::ST_SendStatusMessage(XRVCScriptThread &singleton, const char *pMsg)
{
    EnterCriticalSection(&singleton.m_criticalSectionST);  // lock for write
    // No need to wait until the main thread reads the existing status message, if any: we are overwriting any
    // previous status message anyway.
    singleton.m_csExecuteScriptStatus = *pMsg;   // copy by value
    LeaveCriticalSection(&singleton.m_criticalSectionST);  // unlock    
}

//===========================================================
// Send an XVRC command back to the main thread
//
// Returns: true on success, or false if the main thread is already running a script
//===========================================================
bool XRVCScriptThread::ST_SendCommands(XRVCScriptThread &singleton, const vector<CString> &commandList)
{
    bool sendSuccessful = true;
    EnterCriticalSection(&singleton.m_criticalSectionST);  // lock for read & write
    if (singleton.m_csExecuteScriptCommandList.size() > 0)
        sendSuccessful = false;   // main thread is busy (quite unlikely!)
    else 
        singleton.m_csExecuteScriptCommandList = commandList;   // copy by value
    LeaveCriticalSection(&singleton.m_criticalSectionST);  // unlock    

    return sendSuccessful;
}

//==================================================================
// Parse the supplied script file and write lines to commandListOut
//
// Returns: number of commands parsed, or -1 on error
//==================================================================
int XRVCScriptThread::ST_ParseScriptFile(FILE *pFile, vector<CString> &commandListOut)
{
    const int maxLineLength = 1024;
    char buffer[maxLineLength];
    for (;;)
    {
        if (fgets(buffer, maxLineLength, pFile) == nullptr)
        {
            if (!feof(pFile))    // not at EOF yet?
                return -1;       // read error
            // we reached EOF; stop parsing
            break;
        }

        CString csLine = buffer;
        csLine = csLine.Trim();
        if (csLine.IsEmpty() || (csLine[0] == '#'))
            continue;   // skip empty or comment line

        // we have a command
        commandListOut.push_back(buffer);  // copy by value
    }
    return static_cast<int>(commandListOut.size());
}

//*************************************************************************
// Member methods: these methods are called by our owning dialog and 
// send data to/retrieve data from the script thread.
//*************************************************************************

// Instructs our file selection thread to pop up a file selection box and read in a command script.
// Returns true if thread signaled successfully
bool XRVCScriptThread::OpenScriptFile()
{
    // signal our worker thread to wake up and read a script
    return (SetEvent(m_hEventScriptFile) == TRUE);
}

// Instructs our file selection thread to execute the supplied script.
// Returns true if thread signaled successfully, or false if thread is busy.
bool XRVCScriptThread::OpenScriptFile(const char *pFilename)
{
    bool threadOK = true;
    EnterCriticalSection(&m_criticalSectionST);  // lock for write
    if (!m_csScriptToExecute.IsEmpty())
        threadOK = false;                         // thread is still busy
    else
        m_csScriptToExecute = pFilename;          // copy by value
    LeaveCriticalSection(&m_criticalSectionST);  // unlock    
    
    // if thread not busy, signal our worker thread to wake up and execute the script
    if (threadOK)
        threadOK = (SetEvent(m_hEventScriptFile) == TRUE);

    return threadOK;
}

//=========================================================================================
// Interfaces with our ScriptThread and executes a group of script commands if ready.
// Returns true if script executed, or false if no work was available.
//=========================================================================================

// Latches and returns any pending status message from the thread; may be empty.
// Returns: true if status message added to csOut, false if no status message available
bool XRVCScriptThread::GetStatusMessage(CString &csOut)
{
    EnterCriticalSection(&m_criticalSectionST);  // lock for read
    csOut = m_csExecuteScriptStatus;              // latch it (copy by value)
    m_csExecuteScriptStatus.Empty();    
    LeaveCriticalSection(&m_criticalSectionST);  // unlock

    return !csOut.IsEmpty();
}

// Latches any pending script command list from the thread into commandListOut; may be empty.
// Returns number of commands added to commandListOut
int XRVCScriptThread::GetScriptCommands(vector<CString> &commandListOut)
{
    EnterCriticalSection(&m_criticalSectionST);    // lock for read
    commandListOut = m_csExecuteScriptCommandList;  // latch entire vector (copy by value)
    m_csExecuteScriptCommandList.clear();
    LeaveCriticalSection(&m_criticalSectionST);  // unlock

    return static_cast<int>(commandListOut.size());
}
