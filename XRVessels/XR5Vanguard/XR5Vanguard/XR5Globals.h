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
// XR5Vanguard implementation class
//
// XR5Globals.h
// Contains externs for global variables and constants for the XR5.
// ==============================================================

#pragma once

// extends XR1Globals
#include "XR1Globals.h"   

// size of a mesh group array
#define SizeOfGrp(grp) (sizeof(grp) / sizeof(unsigned int))

// unique panel IDs; standard panels are 0,1,2 and are defined in xr1globals.h
#define PANEL_OVERHEAD  3   // Orbiter 2D panel ID
#define PANEL_PAYLOAD   4   // Orbiter 2D panel ID

#ifdef UNUSED
extern const double HIGHSPEED_CENTER_OF_LIFT;
extern const double FLAPS_FULLY_RETRACTED_SPEED;
extern const double FLAPS_FULLY_DEPLOYED_SPEED;
#endif

extern const double GEAR_COMPRESSION_DISTANCE;
extern const double NOSE_GEAR_ZCOORD;
extern const double REAR_GEAR_ZCOORD;
extern const double GEAR_UNCOMPRESSED_YCOORD;

// addtional warning light values
#define XR5_WARNING_LIGHT_COUNT  2
enum class XR5WarningLight 
{ 
    wl5NONE = -1,    // no light
    wl5Elev,
    wl5Bay
};

// new globals for the XR5
extern const double BAY_OPERATING_SPEED;
extern const double ELEVATOR_OPERATING_SPEED;

extern const double BAY_LIMIT;
extern const double ELEVATOR_LIMIT;
extern const VECTOR3 DOCKING_PORT_COORD;

// new damage enum values we need from the XR1's DamageItem enum
// WARNING: if you add or remove values here, update the D_END global as well!
#define BayDoors DISubclass1
#define Elevator DISubclass2
extern const DamageItem D_END;

// ==============================================================
// Global callback prototypes 

//INT_PTR CALLBACK XR5Ctrl_DlgProc(HWND, UINT, WPARAM, LPARAM);

