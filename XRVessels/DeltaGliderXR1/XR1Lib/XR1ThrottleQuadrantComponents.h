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
// XR1ThrottleQuadrantComponents.cpp
// Handles main, hover, and scram throttle controls
// ==============================================================

#pragma once

#include "Orbitersdk.h"
#include "XR1Component.h"

class MainThrottleComponent : public XR1Component
{
public:
    MainThrottleComponent(InstrumentPanel &instrumentPanel, COORD2 topLeft);
};

class HoverThrottleComponent : public XR1Component
{
public:
    HoverThrottleComponent(InstrumentPanel &instrumentPanel, COORD2 topLeft);
};

class ScramThrottleComponent : public XR1Component
{
public:
    ScramThrottleComponent(InstrumentPanel &instrumentPanel, COORD2 topLeft);
};


//----------------------------------------------------------------------------------

class MainThrottleArea : public XR1Area
{
public:
    MainThrottleArea(InstrumentPanel &parentPanel, const COORD2 panelCoordinates, const int areaID);
    virtual ~MainThrottleArea();
    virtual void Activate();
    virtual bool Redraw2D(const int event, const SURFHANDLE surf);
    virtual bool Redraw3D(const int event, const SURFHANDLE surf);
    virtual bool ProcessMouseEvent(const int event, const int mx, const int my);
    virtual bool ProcessVCMouseEvent(const int event, const VECTOR3 &coords);

protected:
    // state data 
    int m_engsliderpos[2];
    // for VC only
    int m_vcCtrl;
    int m_vcMode; 
    double m_vcPY;
};

//----------------------------------------------------------------------------------

class LargeHoverThrottleArea : public XR1Area
{
public:
    LargeHoverThrottleArea(InstrumentPanel &parentPanel, const COORD2 panelCoordinates, const int areaID);
    virtual void Activate();
    virtual bool Redraw2D(const int event, const SURFHANDLE surf);
    virtual bool Redraw3D(const int event, const SURFHANDLE surf);
    virtual bool ProcessMouseEvent(const int event, const int mx, const int my);
    virtual bool ProcessVCMouseEvent(const int event, const VECTOR3 &coords);

protected:
    // state data 
    int m_engsliderpos;
    // for VC only
    double m_vcPY;
};

// not currently used
#if 0
//----------------------------------------------------------------------------------

class SmallHoverThrottleArea : public XR1Area
{
public:
    SmallHoverThrottleArea(InstrumentPanel &parentPanel, const COORD2 panelCoordinates, const int areaID);
    virtual void Activate();
    virtual bool Redraw2D(const int event, const SURFHANDLE surf);
    virtual bool ProcessMouseEvent(const int event, const int mx, const int my);

protected:
    // state data 
    int m_engsliderpos;
};
#endif

//----------------------------------------------------------------------------------

class ScramThrottleArea : public XR1Area
{
public:
    ScramThrottleArea(InstrumentPanel &parentPanel, const COORD2 panelCoordinates, const int areaID);
    virtual void Activate();
    virtual bool Redraw2D(const int event, const SURFHANDLE surf);
    virtual bool Redraw3D(const int event, const SURFHANDLE surf);
    virtual bool ProcessMouseEvent(const int event, const int mx, const int my);
    virtual bool ProcessVCMouseEvent(const int event, const VECTOR3 &coords);

protected:
    // state data 
    int m_engsliderpos[2];
    // for VC only
    int m_vcCtrl;
    int m_vcMode; 
    double m_vcPY;
};

//----------------------------------------------------------------------------------

class HoverBalanceSwitchArea : public VerticalCenteringRockerSwitchArea
{
public:
    HoverBalanceSwitchArea(InstrumentPanel &parentPanel, const COORD2 panelCoordinates, const int areaID, const int meshTextureID = VCPANEL_TEXTURE_NONE);

protected:
    virtual void ProcessSwitchEvent(SWITCHES switches, POSITION position);
};

//----------------------------------------------------------------------------------

class HoverBalanceVerticalGaugeArea : public VerticalGaugeArea
{
public:
    HoverBalanceVerticalGaugeArea(InstrumentPanel &parentPanel, const COORD2 panelCoordinates, const int areaID, const int meshTextureID = VCPANEL_TEXTURE_NONE);

protected:
    virtual RENDERDATA GetRenderData(const SIDE side);
};

//----------------------------------------------------------------------------------

class ScramPitchSwitchArea : public VerticalCenteringRockerSwitchArea
{
public:
    ScramPitchSwitchArea(InstrumentPanel &parentPanel, const COORD2 panelCoordinates, const int areaID, const int meshTextureID = VCPANEL_TEXTURE_NONE);

protected:
    virtual void ProcessSwitchEvent(SWITCHES switches, POSITION position);
};

//----------------------------------------------------------------------------------

class ScramPitchVerticalGaugeArea : public VerticalGaugeArea
{
public:
    ScramPitchVerticalGaugeArea(InstrumentPanel &parentPanel, const COORD2 panelCoordinates, const int areaID, const int meshTextureID = VCPANEL_TEXTURE_NONE);

protected:
    virtual RENDERDATA GetRenderData(const SIDE side);
};

//----------------------------------------------------------------------------------

class MainPitchSwitchArea : public VerticalCenteringRockerSwitchArea
{
public:
    MainPitchSwitchArea(InstrumentPanel &parentPanel, const COORD2 panelCoordinates, const int areaID, const int meshTextureID = VCPANEL_TEXTURE_NONE);

protected:
    virtual void ProcessSwitchEvent(SWITCHES switches, POSITION position);
};

//----------------------------------------------------------------------------------

class MainPitchVerticalGaugeArea : public VerticalGaugeArea
{
public:
    MainPitchVerticalGaugeArea(InstrumentPanel &parentPanel, const COORD2 panelCoordinates, const int areaID, const int meshTextureID = VCPANEL_TEXTURE_NONE);

protected:
    virtual RENDERDATA GetRenderData(const SIDE side);
};

//----------------------------------------------------------------------------------

class MainYawSwitchArea : public HorizontalCenteringRockerSwitchArea
{
public:
    MainYawSwitchArea(InstrumentPanel &parentPanel, const COORD2 panelCoordinates, const int areaID, const int meshTextureID = VCPANEL_TEXTURE_NONE);

protected:
    virtual void ProcessSwitchEvent(SWITCHES switches, POSITION position);
};

//----------------------------------------------------------------------------------

class MainYawHorizontalGaugeArea : public HorizontalGaugeArea
{
public:
    MainYawHorizontalGaugeArea(InstrumentPanel &parentPanel, const COORD2 panelCoordinates, const int areaID, const int meshTextureID = VCPANEL_TEXTURE_NONE);

protected:
    virtual RENDERDATA GetRenderData(const SIDE side);
};
