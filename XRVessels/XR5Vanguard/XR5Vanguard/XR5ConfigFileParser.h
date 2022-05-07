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
// ConfigFileParser.h
// Parse the XR5 configuration file.
// ==============================================================

#pragma once

#include "XR1ConfigFileParser.h"  // our base class
#include "SecondaryHUDData.h"
#include "XR5Globals.h"

class XR5ConfigFileParser : public XR1ConfigFileParser
{
public:
    XR5ConfigFileParser();

    virtual bool ParseLine(const char *pSection, const char *pPropertyName, const char *pValue, const bool bParsingOverrideFile);

    // NOTE: common payload items moved up to XR1 class; nothing custom for now.
};
