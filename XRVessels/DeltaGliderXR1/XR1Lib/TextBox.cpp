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

// ==============================================================
// XR1 Base Class Library
// These classes extend and use the XR Framework classes
//
// TextBox.cpp
// Draw text in a Windows area; supports newlines via & values.
// ==============================================================

#include "TextBox.h"
#include "DeltaGliderXR1.h"

// base class for all TextBox objects
// screenLineCount = # of text lines on the screen
// bgColor: CWHITE = use transparent background
TextBox::TextBox(int width, int height, uint32_t normalTextColor, uint32_t highlightTextColor, uint32_t bgColor, int screenLineCount, const TextLineGroup &textLineGroup) :
    m_width(width), m_height(height), m_normalTextColor(normalTextColor), m_highlightTextColor(highlightTextColor), 
    m_bgColor(bgColor), m_screenLineCount(screenLineCount), m_textLineGroup(textLineGroup),
    m_lastRenderedAddLinesCount(0)
{
}

// Render info and warning text current in the buffer
// NOTE: if not transparent, it is the caller's responsibility to paint the empty spaces with the background color before invoking this method.
// lineSpacing = # of pixels between screen lines
// startingLineNumber = starting line # in buffer, 1-based; if <= 0, renders full screen starting from bottom
// Returns: true if text re-rendered, false if text is unchanged since last render
bool TextBox::Render(oapi::Sketchpad *skp, int topY, oapi::Font *font, int lineSpacing, bool forceRender, int startingLineNumber)
{
    bool retVal = false;        // assume NOT re-rendered

    const int currentAddLinesCount = m_textLineGroup.GetAddLinesCount();
    if (forceRender || (currentAddLinesCount != m_lastRenderedAddLinesCount))
    {
        m_lastRenderedAddLinesCount = currentAddLinesCount;     // remember this

        // text has changed; must re-render this box
        retVal = true;        

        oapi::Font *prevFont = skp->SetFont(font);
        if (m_bgColor == CWHITE)
        {
            skp->SetBackgroundMode(oapi::Sketchpad::BK_TRANSPARENT);
        }
        else
        {
            skp->SetBackgroundMode(oapi::Sketchpad::BK_OPAQUE);
            skp->SetBackgroundColor(m_bgColor);
        }

        skp->SetTextAlign(oapi::Sketchpad::LEFT);

        int cy = topY + 1;  // top spacing
        int cx = 3;         // left side spacing

        // render each line in the buffer, starting at the top
        int lineCount = 0;

        // figure out which line at which to begin rendering
        const int bufferLineCount = m_textLineGroup.GetLineCount();   // current # of lines in the buffer
        if (startingLineNumber <= 0)
        {
            // user wants a full screen starting from the end
            if (m_screenLineCount >= bufferLineCount)
                startingLineNumber = 1; // entire buffer fits on screen
            else
                startingLineNumber = bufferLineCount - m_screenLineCount + 1;    // bottom n lines in buffer (index is 1-based)
        }
        else if (startingLineNumber > bufferLineCount)   // out-of-range?
            return false;     // can't render anything (don't want to throw exception here b/c we don't want to CTD for this)

        // loop thorough lines in vector, which has 0-based index
        const int startingLineIndex = startingLineNumber-1;     // 0-based
        const int endingLineIndex = min(startingLineIndex + m_screenLineCount, bufferLineCount);   // EXCLUSIVE
        for (int i = startingLineIndex; i < endingLineIndex; i++)
        {
            const TextLine &line = m_textLineGroup.GetLine(i);
            const char *pText = line.text.c_str();

            skp->SetTextColor((line.color == TEXTCOLOR::Normal ? m_normalTextColor : m_highlightTextColor));
            skp->Text(cx, cy, pText, static_cast<int>(line.text.length()));

            // drop to next line
            cy += lineSpacing;
        }

        // restore previous font
        skp->SetFont(prevFont);
    }

    return retVal;
}


//-------------------------------------------------------------------------

// Constructor
// maxLines = maximum # of lines to preserve in this line group; after full, the oldest line will be discarded
TextLineGroup::TextLineGroup(const int maxLines) :
    m_maxLines(maxLines), m_addLinesCount(0)
{
}

TextLineGroup::~TextLineGroup()
{
    // free all TextLine objects in the buffer
    for (unsigned int i=0; i < m_lines.size(); i++)
        delete m_lines[i];
}


// Add lines of text to the HUD; newlines are denoted by the "&" character
// highlighted = to render in highlighted color or normal color
void TextLineGroup::AddLines(const char *pStr, bool highlighted)
{
    m_addLinesCount++;      // text has changed now

    // mark a working copy and parse it into separate lines
    char temp[MAX_MESSAGE_LENGTH];
    strcpy(temp, pStr);

    char *pStart = temp;
    bool cont = true;
    while (cont)
    {
        char *pEnd = strchr(pStart, '&');
        if (pEnd)   // found a newline?
        {
            *pEnd = 0;     // terminate line
        }
        else
            cont = false;   // this is the last line

        TextLine textLine(pStart, (highlighted ? TEXTCOLOR::Highlighted : TEXTCOLOR::Normal));
        AddLine(textLine);

        if (pEnd)
            pStart = pEnd + 1;  // set to start of next line
    }
}

// Add a line to the buffer, removing the oldest line in the buffer if necessary
// textLine will be cloned internally by the vector's push_back method
void TextLineGroup::AddLine(const TextLine &textLine)
{
    // lines are stored oldest -> newest; i.e., index #0 has oldest line
    m_lines.push_back(new TextLine(textLine));  // clone incoming line object

    int lineCount = GetLineCount();
    if (lineCount > m_maxLines)
    {
        lineCount = m_maxLines;  // keep in range
        // New code
        // Delete the oldest item in the list (at the front)
        delete m_lines[0];               // free TextLine object
        m_lines.erase(m_lines.begin());  // ...and remove the line ptr from the vector
    }
}

