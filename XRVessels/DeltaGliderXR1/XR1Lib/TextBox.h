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
// TextBox.h
// Draw text in a Windows area; supports newlines via '&' characters.
// ==============================================================

#pragma once

#include "XR1Colors.h"

#include <string>
#include <vector>
#include "Orbitersdk.h"

using namespace std;

enum class TEXTCOLOR { Normal, Highlighted };

// line of text to be added to buffer
struct TextLine
{
    // Normal constructor
    TextLine(const char *pText, TEXTCOLOR color) 
    {
        this->text = pText;
        this->color = color;
    }

    // NOTE: A copy constructor is required by the 'vector' class.
    TextLine(const TextLine &that) 
    {
        text = that.text;
        color = that.color;
    }

    string text;                // text itself
    TEXTCOLOR color;      // color of line to be rendered

    bool operator=(const TextLine &that) 
    { 
        return ((text == that.text) && (color == that.color));
    }
};

// Manages a group of TextLine objects; this is the primary public object for populating a TextBox
class TextLineGroup
{
public:
    TextLineGroup(const int maxLines);
    virtual ~TextLineGroup();

    int GetLineCount() const { return static_cast<int>(m_lines.size()); }
    void Clear() { m_lines.clear(); }

    // retrives lines in buffer
    const vector<const TextLine *> &GetLines() const { return m_lines; } 

    // retrieves a single line from the buffer
    const TextLine &GetLine(const int index) const { return *m_lines[index]; }

    // Returns how many times AddLines has been invoked; useful to determine whether
    // text has changed since the last check.
    int GetAddLinesCount() const { return m_addLinesCount; }

    // main method that clients will use
    virtual void AddLines(const char *pStr, bool highlighted);

protected:
    void AddLine(const TextLine &textLine);
    const int m_maxLines;
    int m_addLinesCount;   // total # of times AddLines invoked
    vector<const TextLine *> m_lines;  // uses TextLine ptrs for 1) efficiency, and 2) so that vector.erase works
};

//-------------------------------------------------------------------------

class TextBox
{
public:
    TextBox(int width, int height, uint32_t normalTextColor, uint32_t highlightTextColor, uint32_t bgColor, int screenLineCount, const TextLineGroup &textLineGroup);
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    uint32_t GetBackgroundColor() const { return m_bgColor; }
    uint32_t GetNormalColor() const     { return m_normalTextColor; }
    uint32_t GetHighlightColor() const  { return m_highlightTextColor; }
    int GetScreenLineCount() const      { return m_screenLineCount; }
    const TextLineGroup &GetTextLineGroup() const { return m_textLineGroup; }
    
    virtual bool Render(oapi::Sketchpad *skp, int topY, oapi::Font *font, int lineSpacing, bool forceRender, int startingLineNumber = -1);
    
protected:
    int m_width, m_height;
    uint32_t m_normalTextColor, m_highlightTextColor, m_bgColor;
    int m_screenLineCount;  // # of text lines on screen
    int m_lastRenderedAddLinesCount;  

    // reference to text lines themselves
    const TextLineGroup &m_textLineGroup;
};

