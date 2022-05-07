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

#include "DeltaGliderXR1.h"
#include "XR1MultiDisplayArea.h"

//-------------------------------------------------------------------------

AttitudeHoldMultiDisplayMode::AttitudeHoldMultiDisplayMode(int modeNumber) :
    MultiDisplayMode(modeNumber),
    m_backgroundSurface(0), m_mouseHoldTargetSimt(-1), m_lastAction(AXIS_ACTION::ACT_NONE), m_repeatCount(0)
{
    m_engageButtonCoord.x = 6;
    m_engageButtonCoord.y = 42;

    m_toggleAOAPitchCoord.x = 169;
    m_toggleAOAPitchCoord.y = 28;

    m_pitchUpArrowSmallCoord.x = 166;
    m_pitchUpArrowSmallCoord.y = 41;

    m_pitchUpArrowLargeCoord.x = 149;
    m_pitchUpArrowLargeCoord.y = 41;

    m_pitchDownArrowSmallCoord.x = 166;
    m_pitchDownArrowSmallCoord.y = 50;

    m_pitchDownArrowLargeCoord.x = 149;
    m_pitchDownArrowLargeCoord.y = 50;

    m_bankLeftArrowCoord.x = 124;
    m_bankLeftArrowCoord.y = 86;

    m_bankRightArrowCoord.x = 169;
    m_bankRightArrowCoord.y = 86;

    m_resetBankButtonCoord.x = 78;
    m_resetBankButtonCoord.y = 99;

    m_resetPitchButtonCoord.x = 6;
    m_resetPitchButtonCoord.y = 88;

    m_resetBothButtonCoord.x = 6;
    m_resetBothButtonCoord.y = 99;

    m_syncButtonCoord.x = 78;
    m_syncButtonCoord.y = 88;

    m_repeatSpeed = 0.125;  // seconds between clicks if mouse held down
}

void AttitudeHoldMultiDisplayMode::Activate()
{
    m_backgroundSurface = CreateSurface("Bitmaps/DeltaGliderXR1/AttitudeHoldMultiDisplay.bmp");

    m_statusFont = oapiCreateFont(12, true, "Microsoft Sans Serif", FONT_BOLD);  // ENGAGED or DISENGAGED
    m_numberFont = oapiCreateFont(12, true, "Microsoft Sans Serif", FONT_BOLD);  // bank/pitch number text
    m_buttonFont = oapiCreateFont(12, true, "Microsoft Sans Serif", FONT_BOLD);  // engage/disengage button text
    m_aoaPitchFont = oapiCreateFont(10, true, "Arial", FONT_BOLD);  // "Hold Pitch", "Hold AOA" text
}

void AttitudeHoldMultiDisplayMode::Deactivate()
{
    DestroySurface(&m_backgroundSurface);
    oapiReleaseFont(m_statusFont);
    oapiReleaseFont(m_numberFont);
    oapiReleaseFont(m_buttonFont);
    oapiReleaseFont(m_aoaPitchFont);
}

bool AttitudeHoldMultiDisplayMode::Redraw2D(const int event, const SURFHANDLE surf)
{
    // Always re-render everything; it is too error-prone to try to track all values and clear any 
    // old data underneath from the previous render.

    const bool holdAOA = GetXR1().m_holdAOA;        // for convenience

    // render the background
    const COORD2& screenSize = GetScreenSize();
    DeltaGliderXR1::SafeBlt(surf, m_backgroundSurface, 0, 0, 0, 0, screenSize.x, screenSize.y);

    oapi::Sketchpad *skp = oapiGetSketchpad(surf);
    // obtain device context and save existing font
    skp->SetFont(m_statusFont);
    skp->SetBackgroundMode(oapi::Sketchpad::BkgMode::BK_TRANSPARENT);
    skp->SetTextAlign(oapi::Sketchpad::TAlign_horizontal::LEFT);

    // render autopilot status
    const char* pStatus;        // set below
    uint32_t statusColor;
    const bool engaged = (GetXR1().m_customAutopilotMode == AUTOPILOT::AP_ATTITUDEHOLD);
    if (engaged && (GetXR1().m_customAutopilotSuspended))
    {
        pStatus = "SUSPENDED";
        statusColor = CREF(BRIGHT_WHITE);
    }
    else  // normal operation
    {
        pStatus = (engaged ? "ENGAGED" : "DISENGAGED");
        statusColor = (engaged ? CREF(BRIGHT_GREEN) : CREF(BRIGHT_RED));  // use CREF macro to convert to Windows' Blue, Green, Red COLORREF
    }
    skp->SetTextColor(statusColor);
    skp->Text(46, 24, pStatus, static_cast<int>(strlen(pStatus)));

    // render "Set Pitch" or "Set AOA" text
    skp->SetFont(m_aoaPitchFont);
    skp->SetTextAlign(oapi::Sketchpad::TAlign_horizontal::RIGHT);
    const char* pSetText = (holdAOA ? "SET AOA" : "SET PITCH");
    skp->SetTextColor(CREF((holdAOA ? BRIGHT_YELLOW : BRIGHT_GREEN)));
    skp->Text(165, 26, pSetText, static_cast<int>(strlen(pSetText)));
    skp->SetTextAlign(oapi::Sketchpad::TAlign_horizontal::LEFT);     // restore to default to LEFT alignment

    // render button text
    skp->SetFont(m_buttonFont);
    const char* pEngageDisengage = (engaged ? "Disengage" : "Engage");
    skp->SetTextColor(CREF(LIGHT_BLUE));
    skp->Text(27, 43, pEngageDisengage, static_cast<int>(strlen(pEngageDisengage)));

    // render ship's current pitch, bank, and AOA
    skp->SetFont(m_numberFont);
    skp->SetTextColor(CREF(OFF_WHITE217));
    char temp[15];
    sprintf(temp, "%+7.2f°", GetVessel().GetPitch() * DEG);
    skp->Text(31, 61, temp, static_cast<int>(strlen(temp)));

    sprintf(temp, "%+7.2f°", GetVessel().GetBank() * DEG);
    skp->Text(31, 72, temp, static_cast<int>(strlen(temp)));

    sprintf(temp, "%+7.2f°", GetVessel().GetAOA() * DEG);
    skp->Text(98, 61, temp, static_cast<int>(strlen(temp)));

    // render "ZERO PITCH" or "ZERO AOA"
    skp->SetFont(m_aoaPitchFont);
    const char* pZeroText = (holdAOA ? "ZERO AOA" : "ZERO PITCH");
    skp->SetTextColor(CREF((holdAOA ? BRIGHT_YELLOW : BRIGHT_GREEN)));
    skp->Text(18, 86, pZeroText, static_cast<int>(strlen(pZeroText)));

    // render SET pitch/aoa and bank values; these values will be limited to +-90 degrees at the most
    skp->SetFont(m_numberFont);

    skp->SetTextAlign(oapi::Sketchpad::TAlign_horizontal::RIGHT);
    skp->SetTextColor(engaged ? CREF((holdAOA ? BRIGHT_YELLOW : BRIGHT_GREEN)) : CREF(LIGHT_BLUE));
    sprintf(temp, "%+5.1f°", GetXR1().m_setPitchOrAOA);  // already in degrees
    skp->Text(143, 41, temp, static_cast<int>(strlen(temp)));

    skp->SetTextAlign(oapi::Sketchpad::TAlign_horizontal::CENTER);
    skp->SetTextColor(engaged ? CREF(BRIGHT_GREEN) : CREF(LIGHT_BLUE));
    sprintf(temp, "%+5.1f°", GetXR1().m_setBank);  // already in degrees
    skp->Text(151, 83, temp, static_cast<int>(strlen(temp)));

    oapiReleaseSketchpad(skp);

    return true;
}

bool AttitudeHoldMultiDisplayMode::ProcessMouseEvent(const int event, const int mx, const int my)
{
    // if crew is incapacitated, nothing to do here
    if (GetXR1().IsCrewIncapacitatedOrNoPilotOnBoard())
        return false;

    bool processed = false;
    bool playSound = false;  // play sound in button processing
    bool changeAxis = true;  // change axis value in button processing

    COORD2 c = { mx, my };

    if (event & PANEL_MOUSE_LBDOWN)
    {
        m_repeatCount = 0;  // reset just in case

        // check engage/disengage button
        if (c.InBounds(m_engageButtonCoord, 14, 14))
        {
            // toggle autopilot status
            GetXR1().ToggleAttitudeHold();
            processed = true;
            playSound = true;
        }
        else if (c.InBounds(m_toggleAOAPitchCoord, 7, 7))   // toggle AOA / Pitch hold
        {
            GetXR1().ToggleAOAPitchAttitudeHold(true);
            processed = true;
        }
        else if (c.InBounds(m_resetBankButtonCoord, 7, 7))    // reset bank button
        {
            GetXR1().ResetAttitudeHoldToLevel(true, true, false);
            processed = true;
        }
        else if (c.InBounds(m_resetBankButtonCoord, 7, 7))    // reset bank button
        {
            GetXR1().ResetAttitudeHoldToLevel(true, true, false);
            processed = true;
        }
        else if (c.InBounds(m_resetPitchButtonCoord, 7, 7))    // reset pitch/aoa button
        {
            GetXR1().ResetAttitudeHoldToLevel(true, false, true);
            processed = true;
        }
        else if (c.InBounds(m_syncButtonCoord, 7, 7))    // sync to current attitude
        {
            GetXR1().SyncAttitudeHold(true, false);  // do not force PITCH mode here
            processed = true;
        }
        else if (c.InBounds(m_resetBothButtonCoord, 7, 7))    // reset BOTH button
        {
            GetXR1().ResetAttitudeHoldToLevel(true, true, true);
            processed = true;
        }
    }

    // check axis buttons
    AXIS_ACTION action = AXIS_ACTION::ACT_NONE;
    if (event & (PANEL_MOUSE_LBDOWN | PANEL_MOUSE_LBPRESSED))
    {
        const double simt = GetAbsoluteSimTime();

        // if TRUE, process a button click
        bool doButtonClick = false;

        if (event & PANEL_MOUSE_LBDOWN)
        {
            // mouse just clicked; always process it immediately
            doButtonClick = true;
            playSound = true;

            // next click if mouse held down is one second from now
            m_mouseHoldTargetSimt = simt + 1.0;
        }

        // check whether we reached our target hold time
        if ((m_mouseHoldTargetSimt > 0) && (simt >= m_mouseHoldTargetSimt))
        {
            doButtonClick = true;
            m_mouseHoldTargetSimt = simt + m_repeatSpeed;   // process another event if mouse held down long enough
            m_repeatCount++;        // remember this
        }

        // check pitch and bank arrows
        if (c.InBounds(m_pitchUpArrowSmallCoord, 6, 7))
        {
            if (doButtonClick)
            {
                // use PILOT controls (reverse up/down arrows)
                m_lastAction = action = AXIS_ACTION::DECPITCH_SMALL;
            }
        }
        else if (c.InBounds(m_pitchDownArrowSmallCoord, 6, 7))
        {
            if (doButtonClick)
            {
                // use PILOT controls (reverse up/down arrows)
                m_lastAction = action = AXIS_ACTION::INCPITCH_SMALL;
            }
        }
        else if (c.InBounds(m_pitchUpArrowLargeCoord, 6, 7))
        {
            if (doButtonClick)
            {
                // use PILOT controls (reverse up/down arrows)
                m_lastAction = action = AXIS_ACTION::DECPITCH_LARGE;
            }
        }
        else if (c.InBounds(m_pitchDownArrowLargeCoord, 6, 7))
        {
            if (doButtonClick)
            {
                // use PILOT controls (reverse up/down arrows)
                m_lastAction = action = AXIS_ACTION::INCPITCH_LARGE;
            }
        }
        else if (c.InBounds(m_bankLeftArrowCoord, 6, 7))
        {
            if (doButtonClick)
            {
                m_lastAction = action = AXIS_ACTION::INCBANK;
            }
        }
        else if (c.InBounds(m_bankRightArrowCoord, 6, 7))
        {
            if (doButtonClick)
            {
                m_lastAction = action = AXIS_ACTION::DECBANK;
            }
        }
        else
        {
            // mouse is outside of any buttons!
            // NOTE: technically we should check if the mouse moves off of the FIRST button clicked, but it's not worth the 
            // effort since there is blank space between the buttons anyway.
            m_mouseHoldTargetSimt = -1;
        }
    }
    else if (event & (PANEL_MOUSE_LBUP))
    {
        // mouse released; reset hold timer
        m_mouseHoldTargetSimt = -1;

        // re-issue the last action so a message is logged about the final state now IF we were repeating the button clicks
        if (m_repeatCount > 0)
        {
            action = m_lastAction;
            playSound = true;   // show final message and play button up sound
            changeAxis = false; // ...but don't actually change the value
            m_repeatCount = 0;  // reset
        }

        m_lastAction = AXIS_ACTION::ACT_NONE;  // reset
    }

    if (action != AXIS_ACTION::ACT_NONE)
    {
        const bool invertPitchArrows = GetXR1().GetXR1Config()->InvertAttitudeHoldPitchArrows;
        switch (action)
        {
        case AXIS_ACTION::INCPITCH_SMALL:
            if (invertPitchArrows) goto decpitch_small;
        incpitch_small:
            GetXR1().IncrementAttitudeHoldPitch(playSound, changeAxis, AP_PITCH_DELTA_SMALL);
            processed = true;
            break;

        case AXIS_ACTION::DECPITCH_SMALL:
            if (invertPitchArrows) goto incpitch_small;
        decpitch_small:
            GetXR1().DecrementAttitudeHoldPitch(playSound, changeAxis, AP_PITCH_DELTA_SMALL);
            processed = true;
            break;

        case AXIS_ACTION::INCPITCH_LARGE:
            if (invertPitchArrows) goto decpitch_large;
        incpitch_large:
            GetXR1().IncrementAttitudeHoldPitch(playSound, changeAxis, AP_PITCH_DELTA_LARGE);
            processed = true;
            break;

        case AXIS_ACTION::DECPITCH_LARGE:
            if (invertPitchArrows) goto incpitch_large;
        decpitch_large:
            GetXR1().DecrementAttitudeHoldPitch(playSound, changeAxis, AP_PITCH_DELTA_LARGE);
            processed = true;
            break;

        case AXIS_ACTION::INCBANK:
            GetXR1().IncrementAttitudeHoldBank(playSound, changeAxis);
            processed = true;
            break;

        case AXIS_ACTION::DECBANK:
            GetXR1().DecrementAttitudeHoldBank(playSound, changeAxis);
            processed = true;
            break;

        default:
            // no action
            break;
        }
    }

    return processed;
}
