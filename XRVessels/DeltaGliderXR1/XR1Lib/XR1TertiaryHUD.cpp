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

#include "DeltaGliderXR1.h"
#include "XR1HUD.h"
#include "Bitmaps.h"

// ==============================================================

TertiaryHUDButtonArea::TertiaryHUDButtonArea(InstrumentPanel& parentPanel, const COORD2 panelCoordinates, const int areaID) :
    XR1Area(parentPanel, panelCoordinates, areaID)
{
}

void TertiaryHUDButtonArea::Activate()
{
    Area::Activate();  // invoke superclass method
    if (IsVC()) // 3D panel?
    {
        // 3D support N/A
    }
    else    // 2D panel
    {
        oapiRegisterPanelArea(GetAreaID(), GetRectForSize(12, 12), PANEL_REDRAW_MOUSE | PANEL_REDRAW_USER, PANEL_MOUSE_LBDOWN);
        m_mainSurface = CreateSurface(IDB_LIGHT2);
    }
}

bool TertiaryHUDButtonArea::Redraw2D(const int event, const SURFHANDLE surf)
{
    int srcX = (GetXR1().m_tertiaryHUDOn ? 12 : 0);
    DeltaGliderXR1::SafeBlt(surf, m_mainSurface, 0, 0, srcX, 0, 12, 12);

    return true;
}

bool TertiaryHUDButtonArea::ProcessMouseEvent(const int event, const int mx, const int my)
{
    // if crew is incapacitated or systems offline, nothing to do here
    if (GetXR1().IsCrewIncapacitatedOrNoPilotOnBoard() || GetXR1().m_internalSystemsFailure)
        return false;

    GetXR1().m_tertiaryHUDOn = !GetXR1().m_tertiaryHUDOn;   // toggle
    GetXR1().PlaySound(GetXR1().SwitchOn, DeltaGliderXR1::ST_Other, QUIET_CLICK);

    return true;
}

//----------------------------------------------------------------------------------

//
// Constructor
// vessel = our vessel handle
// panelCoordinates = absolute coordinates of this area on the parent instrument panel
// areaID = unique Orbiter area ID
TertiaryHUDArea::TertiaryHUDArea(InstrumentPanel& parentPanel, const COORD2 panelCoordinates, const int areaID) :
    PopupHUDArea(parentPanel, panelCoordinates, areaID, 209, 82)
{
    const XR1ConfigFileParser& config = *GetXR1().GetXR1Config();

    const int width = GetWidth(), height = GetHeight();
    SetTextBox(new TextBox(width - 2, height, config.TertiaryHUDNormalColor, config.TertiaryHUDWarningColor, config.TertiaryHUDBackgroundColor,
        7, GetXR1().m_infoWarningTextLineGroup));

    // create our font
    // NOTE: we want an ALIASED font for a non-transparent background, or UNALIASED font for transparent background
//FIXME
//    const int antialiasFlag = ((config.TertiaryHUDBackgroundColor == 0xFFFFFF) ? NONANTIALIASED_QUALITY : 0);
    m_mainFont = oapiCreateFont(14, true, "Arial");
    m_lineSpacing = 11;     // pixels between lines
}

TertiaryHUDArea::~TertiaryHUDArea()
{
    // clean up the text box we allocated
    delete GetTextBox();

    // clean up the font we allocated
    oapiReleaseFont(m_mainFont);
}

// returns TRUE if HUD is on
bool TertiaryHUDArea::isOn()
{
    return GetXR1().m_tertiaryHUDOn;
}

// Set HUD colors; invoked by the superclass before HUD rendering begins
void TertiaryHUDArea::SetHUDColors()
{
    const XR1ConfigFileParser& config = *GetXR1().GetXR1Config();

    SetColor(config.TertiaryHUDNormalColor);    // normal color
    SetHighlightColor(config.TertiaryHUDWarningColor);
    SetBackgroundColor(config.TertiaryHUDBackgroundColor);

}

// Render the contents of the HUD
// NOTE: the subclass MUST draw text from the supplied topY coordinate (plus some border gap space)
// The X coordinate is zero @ the border
// Returns: true if text re-rendered, false if not
bool TertiaryHUDArea::DrawHUD(const int event, const int topY, oapi::Sketchpad *skp, uint32_t colorRef, bool forceRender)
{
    // NOTE: area was registered with PANEL_MAP_BACKGROUND, so we don't need to always repaint it
    // fill the background area if not transparent; this is to make the background solid between letters
    if (GetXR1().GetXR1Config()->TertiaryHUDBackgroundColor != CWHITE)
    {
        auto old = skp->SetBrush(m_hBackgroundBrush);
        skp->Rectangle(0, m_topYCoordinate, m_width, m_height);
        skp->SetBrush(old);
    }

    // invoke new TextBox handler to draw text using a TRANSPARENT background; this same TextBox handler
    // can also be used on the lower panel to render on a normal screen.
    // Note that our text box will never be null here.
    return m_pTextBox->Render(skp, topY, m_mainFont, m_lineSpacing, forceRender);  // CWHITE = use transparent background
}

