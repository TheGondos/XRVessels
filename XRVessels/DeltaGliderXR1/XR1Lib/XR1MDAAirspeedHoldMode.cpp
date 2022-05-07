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

#define ROLLING_AVG_SIZE ((sizeof(m_maxMainAccRollingAvg) / sizeof(double)))

// Constructor
AirspeedHoldMultiDisplayMode::AirspeedHoldMultiDisplayMode(int modeNumber) :
    MultiDisplayMode(modeNumber), m_statusFont(0), m_numberFont(0), m_buttonFont(0),
    m_backgroundSurface(0), m_mouseHoldTargetSimt(-1), m_lastAction(RATE_ACTION::ACT_NONE), m_repeatCount(0)
{
    m_engageButtonCoord.x = 6;
    m_engageButtonCoord.y = 42;

    m_rateUpP1ArrowCoord.x = 166;
    m_rateUpP1ArrowCoord.y = 47;

    m_rateDownP1ArrowCoord.x = 166;
    m_rateDownP1ArrowCoord.y = 56;

    m_rateUp1ArrowCoord.x = 153;
    m_rateUp1ArrowCoord.y = 47;

    m_rateDown1ArrowCoord.x = 153;
    m_rateDown1ArrowCoord.y = 56;

    m_rateUp5ArrowCoord.x = 140;
    m_rateUp5ArrowCoord.y = 47;

    m_rateDown5ArrowCoord.x = 140;
    m_rateDown5ArrowCoord.y = 56;

    m_rateUp25ArrowCoord.x = 127;
    m_rateUp25ArrowCoord.y = 47;

    m_rateDown25ArrowCoord.x = 127;
    m_rateDown25ArrowCoord.y = 56;

    m_holdCurrentButtonCoord.x = 113;
    m_holdCurrentButtonCoord.y = 77;

    m_resetButtonCoord.x = 113;
    m_resetButtonCoord.y = 88;

    m_repeatSpeed = 0.0625;  // seconds between clicks if mouse held down: 16 clicks per second

    // Note: 10 frames is not enough here: it still jumps in the thousanth's place
    m_pMaxMainAccRollingArray = new RollingArray(20);  // average last 20 frame values
}

// Destructor
AirspeedHoldMultiDisplayMode::~AirspeedHoldMultiDisplayMode()
{
    delete m_pMaxMainAccRollingArray;
}

void AirspeedHoldMultiDisplayMode::Activate()
{
    m_backgroundSurface = CreateSurface("Bitmaps/DeltaGliderXR1/AirspeedHoldMultiDisplay.bmp");

    m_statusFont = oapiCreateFont(12, true, "Microsoft Sans Serif");  // ENGAGED or DISENGAGED
    m_numberFont = oapiCreateFont(12, true, "Microsoft Sans Serif");  // set airspeed number text
    m_buttonFont = oapiCreateFont(12, true, "Microsoft Sans Serif");  // engage/disengage button text
}

void AirspeedHoldMultiDisplayMode::Deactivate()
{
    DestroySurface(&m_backgroundSurface);
    oapiReleaseFont(m_statusFont);
    oapiReleaseFont(m_numberFont);
    oapiReleaseFont(m_buttonFont);
}

bool AirspeedHoldMultiDisplayMode::Redraw2D(const int event, const SURFHANDLE surf)
{
    // Always re-render everything; it is too error-prone to try to track all values and clear any 
    // old data underneath from the previous render.

    // render the background
    const COORD2& screenSize = GetScreenSize();
    DeltaGliderXR1::SafeBlt(surf, m_backgroundSurface, 0, 0, 0, 0, screenSize.x, screenSize.y);

    // obtain device context and save existing font
    oapi::Sketchpad *skp = oapiGetSketchpad(surf);
    skp->SetFont(m_statusFont);
    skp->SetBackgroundMode(oapi::Sketchpad::BkgMode::BK_TRANSPARENT);
    skp->SetTextAlign(oapi::Sketchpad::TAlign_horizontal::LEFT);

    // render autopilot status
    const char* pStatus;        // set below
    uint32_t statusColor;
    const bool engaged = GetXR1().m_airspeedHoldEngaged;
    if (engaged && (GetXR1().m_airspeedHoldSuspended))
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

    // render button text
    skp->SetFont(m_buttonFont);
    const char* pEngageDisengage = (engaged ? "Disengage" : "Engage");
    skp->SetTextColor(CREF(LIGHT_BLUE));
    skp->Text(27, 43, pEngageDisengage, static_cast<int>(strlen(pEngageDisengage)));

    skp->SetFont(m_numberFont);
    skp->SetTextColor(CREF(OFF_WHITE217));
    char temp[15];

    // airspeed 
    double airspeed = GetXR1().GetAirspeed();  // in m/s; we are holding KIAS here, NOT groundspeed!

    // keep in range
    if (airspeed > 99999.9)
        airspeed = 99999.9;
    else if (airspeed < 0)
        airspeed = 0;     // sanity-check
    sprintf(temp, "%-.1f m/s", airspeed);
    skp->Text(48, 62, temp, static_cast<int>(strlen(temp)));

    // imperial airspeed 
    double airspeedImp = XR1Area::MpsToMph(airspeed);
    if (airspeedImp > 99999.9)
        airspeedImp = 99999.9;
    else if (airspeedImp < 0)
        airspeedImp = 0;     // sanity-check
    sprintf(temp, "%-.1f mph", airspeedImp);
    skp->Text(48, 73, temp, static_cast<int>(strlen(temp)));

    // max main engine acc based on ship mass + atm drag
    // NOTE: this is a ROLLING AVERAGE over the last n frames to help the jumping around the Orbiter does with the acc values
    m_pMaxMainAccRollingArray->AddSample(GetXR1().m_maxMainAcc);
    const double maxMainAcc = m_pMaxMainAccRollingArray->GetAverage();   // overall average for all samples

    if (fabs(maxMainAcc) > 99.999)        // keep in range
        sprintf(temp, "------ m/s²");
    else
        sprintf(temp, "%.3f m/s²", maxMainAcc);
    uint32_t cref;  // reused below as well
    if (maxMainAcc <= 0)
        cref = CREF(MEDB_RED);
    else if (maxMainAcc < 1.0)
        cref = CREF(BRIGHT_YELLOW);
    else
        cref = CREF(BRIGHT_GREEN);
    skp->SetTextColor(cref);
    skp->Text(62, 95, temp, static_cast<int>(strlen(temp)));

    // main thrust pct 
    double mainThrustFrac = GetVessel().GetThrusterGroupLevel(THGROUP_MAIN);  // do not round this; sprintf will do it
    double mainThrustPct = (mainThrustFrac * 100.0);
    sprintf(temp, "%.3f%%", mainThrustPct);
    if (mainThrustPct >= 100)
        cref = CREF(MEDB_RED);
    else if (mainThrustPct >= 90)
        cref = CREF(BRIGHT_YELLOW);
    else
        cref = CREF(BRIGHT_GREEN);

    skp->SetTextColor(cref);
    skp->Text(62, 84, temp, static_cast<int>(strlen(temp)));

    // render the set airspeed
    sprintf(temp, "%.1lf", GetXR1().m_setAirspeed);
    skp->SetTextAlign(oapi::Sketchpad::TAlign_horizontal::RIGHT);
    skp->SetTextColor(CREF(LIGHT_BLUE));
    skp->Text(121, 48, temp, static_cast<int>(strlen(temp)));

    oapiReleaseSketchpad(skp);

    return true;
}

bool AirspeedHoldMultiDisplayMode::ProcessMouseEvent(const int event, const int mx, const int my)
{
    // if crew is incapacitated, nothing to do here
    if (GetXR1().IsCrewIncapacitatedOrNoPilotOnBoard())
        return false;

    bool processed = false;
    bool playSound = false;  // play sound in button processing

    COORD2 c = { mx, my };

    if (event & PANEL_MOUSE_LBDOWN)
    {
        m_repeatCount = 0;  // reset just in case

        // check engage/disengage button
        if (c.InBounds(m_engageButtonCoord, 14, 14))
        {
            // toggle autopilot status
            GetXR1().SetAirspeedHoldMode(!GetXR1().m_airspeedHoldEngaged, true);

            processed = true;
            playSound = true;
        }
        else if (c.InBounds(m_holdCurrentButtonCoord, 7, 7))   // HOLD CURRENT button
        {
            GetXR1().SetAirspeedHold(true, AIRSPEEDHOLD_ADJUST::AS_HOLDCURRENT, 0);
            processed = true;
        }
        else if (c.InBounds(m_resetButtonCoord, 7, 7))   // RESET button
        {
            GetXR1().SetAirspeedHold(true, AIRSPEEDHOLD_ADJUST::AS_RESET, 0);
            processed = true;
        }
    }

    // check rate buttons
    RATE_ACTION action = RATE_ACTION::ACT_NONE;
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

            // next click if mouse held down is 0.75 second from now
            m_mouseHoldTargetSimt = simt + 0.75;
        }

        // check whether we reached our target hold time
        if ((m_mouseHoldTargetSimt > 0) && (simt >= m_mouseHoldTargetSimt))
        {
            doButtonClick = true;
            m_mouseHoldTargetSimt = simt + m_repeatSpeed;   // process another event if mouse held down long enough
            m_repeatCount++;        // remember this
        }

        // check rate arrows
        if (c.InBounds(m_rateUpP1ArrowCoord, 6, 7))
        {
            if (doButtonClick)
            {
                m_lastAction = action = RATE_ACTION::INCRATEP1;
            }
        }
        else if (c.InBounds(m_rateDownP1ArrowCoord, 6, 7))
        {
            if (doButtonClick)
            {
                m_lastAction = action = RATE_ACTION::DECRATEP1;
            }
        }
        else if (c.InBounds(m_rateUp1ArrowCoord, 6, 7))
        {
            if (doButtonClick)
            {
                m_lastAction = action = RATE_ACTION::INCRATE1;
            }
        }
        else if (c.InBounds(m_rateDown1ArrowCoord, 6, 7))
        {
            if (doButtonClick)
            {
                m_lastAction = action = RATE_ACTION::DECRATE1;
            }
        }
        else if (c.InBounds(m_rateUp5ArrowCoord, 6, 7))
        {
            if (doButtonClick)
            {
                m_lastAction = action = RATE_ACTION::INCRATE5;
            }
        }
        else if (c.InBounds(m_rateDown5ArrowCoord, 6, 7))
        {
            if (doButtonClick)
            {
                m_lastAction = action = RATE_ACTION::DECRATE5;
            }
        }
        else if (c.InBounds(m_rateUp25ArrowCoord, 6, 7))
        {
            if (doButtonClick)
            {
                m_lastAction = action = RATE_ACTION::INCRATE25;
            }
        }
        else if (c.InBounds(m_rateDown25ArrowCoord, 6, 7))
        {
            if (doButtonClick)
            {
                m_lastAction = action = RATE_ACTION::DECRATE25;
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
            m_repeatCount = 0;  // reset
        }

        m_lastAction = RATE_ACTION::ACT_NONE;  // reset
    }

    if (action != RATE_ACTION::ACT_NONE)
    {
        switch (action)
        {
        case RATE_ACTION::INCRATEP1:
            GetXR1().SetAirspeedHold(playSound, AIRSPEEDHOLD_ADJUST::AS_ADJUST, ASRATE_TINY);
            processed = true;
            break;

        case RATE_ACTION::DECRATEP1:
            GetXR1().SetAirspeedHold(playSound, AIRSPEEDHOLD_ADJUST::AS_ADJUST, -ASRATE_TINY);
            processed = true;
            break;

        case RATE_ACTION::INCRATE1:
            GetXR1().SetAirspeedHold(playSound, AIRSPEEDHOLD_ADJUST::AS_ADJUST, ASRATE_SMALL);
            processed = true;
            break;

        case RATE_ACTION::DECRATE1:
            GetXR1().SetAirspeedHold(playSound, AIRSPEEDHOLD_ADJUST::AS_ADJUST, -ASRATE_SMALL);
            processed = true;
            break;

        case RATE_ACTION::INCRATE5:
            GetXR1().SetAirspeedHold(playSound, AIRSPEEDHOLD_ADJUST::AS_ADJUST, ASRATE_MED);
            processed = true;
            break;

        case RATE_ACTION::DECRATE5:
            GetXR1().SetAirspeedHold(playSound, AIRSPEEDHOLD_ADJUST::AS_ADJUST, -ASRATE_MED);
            processed = true;
            break;

        case RATE_ACTION::INCRATE25:
            GetXR1().SetAirspeedHold(playSound, AIRSPEEDHOLD_ADJUST::AS_ADJUST, ASRATE_LARGE);
            processed = true;
            break;

        case RATE_ACTION::DECRATE25:
            GetXR1().SetAirspeedHold(playSound, AIRSPEEDHOLD_ADJUST::AS_ADJUST, -ASRATE_LARGE);
            processed = true;
            break;

        default:
            // no action
            break;
        }
    }

    return processed;
}
