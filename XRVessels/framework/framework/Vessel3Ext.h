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
// vessel3ext.h
// Class defining an extended VESSEL2 class for use with the new XR cockpit framework.
// ==============================================================

#pragma once

#include "XRVesselCtrl.h"
#include "XRGrappleTargetVessel.h"
#include "stringhasher.h"
#include "PropType.h"
#include "VesselConfigFileParser.h"
#include "RegKeyManager.h"

#include <unordered_map>
#include <vector>

// redefine oapiGetSimTime as a compile-time error to prevent XR code from accidentally invoking it
// Note: ideally I would like to have this invoke '#error foo' to generate an elegant message, but there's no way to declare that
#define oapiGetSimTime ERROR! "Do not invoke oapiGetSimTime: see comment block in VESSEL3_EXT::clbkPreStep for details"

#define XR_GLOBAL_SETTINGS_REG_KEY "SOFTWARE\\AlteaAerospace\\XR"   // under HKEY_CURRENT_USER

#define MAX_VELOCITY_FOR_WHEEL_STOP 0.04		  // max meters-per-second the ship can be moving and still be considered wheel-stop (used by IsLanded(); determines when parking brakes engage, for example).

class Area;
class PrePostStep;
class PrePostStep;

// must use forward reference to avoid circular dependency
class InstrumentPanel;  

// Constant for all MeshTextureIDToTextureIndex methods.  All normal MeshTextureIDToTextureIndex #defines are >= 0.
// VCPANEL_TEXTURE_NONE = -1 = "no texture" (i.e., "not applicable"), so these texture IDs here must start at zero or higher.
// This must be defined up here since it is used by both Component and Area classes.
#define VCPANEL_TEXTURE_NONE -1

// define useful macros
#define MPS_TO_KNOTS(mps)   (mps / 0.5148)    /* meters-per-second to knots */
#define KNOTS_TO_MPS(knots) (knots * 0.5148)  /* knots to meters-per-second */

// a / b, but return zero if b == 0
inline double SAFE_FRACTION(double a, double b)   
{
    if (b == 0) 
        return 0; 
    
    return (a / b);
}

// VESSEL3_EXT base class common to all XR vessels
class VESSEL3_EXT : public XRVesselCtrl
{
public:
    // Constructor / Destructor
    VESSEL3_EXT(OBJHANDLE vessel, int fmodel = 1);
    virtual ~VESSEL3_EXT();

	//RegKeyManager m_regKeyManager;		// for global Orbiter-independent XR settings
	
	// Note: returns 0 if Orbiter is full-screen
    /*
	static HWND GetOrbiterRenderWindowHandle()
	{
		return ::FindWindow("Orbiter Render Window", nullptr);
	}
*/
	void *GetModuleHandle() { return m_hModule; }

    void SetModuleHandle(void *hModule) { m_hModule = hModule; }
    void AddInstrumentPanel(InstrumentPanel *pPanel, const int panelWidth);
    void AddPreStep(PrePostStep *pPostStep);
    void AddPostStep(PrePostStep *pPreStep);
    InstrumentPanel *GetInstrumentPanel(const int panelNumber);
    vector<PrePostStep *> &GetPostStepVector() { return m_postStepVector; }
    vector<PrePostStep *>  &GetPreStepVector()  { return m_preStepVector; }
    void DeactivateAllPanels();
    Area *GetArea(const int panelID, const int areaID);
    bool HasFocus() const { return m_hasFocus; }   // returns true if we have the focus, false if not

    // returns the number of '1' bits in dwBitmask
    static int CountOneBits(int dwBitmask)
    {
        int oneBitsCount = 0;
        for (int i = 0; i < 32; i++) {
            oneBitsCount += (dwBitmask & 1); // add 1 if the LSB is set
            dwBitmask >>= 1;
        }
        return oneBitsCount;
    }

    void SetMeshGroupsVisibility(const bool isVisible, const DEVMESHHANDLE hMesh, const int groupCount, const unsigned int *meshGroups) const
    {
        if (hMesh == nullptr)
            return;   // mesh not loaded yet
            
        for (int i=0; i < groupCount; i++)
        {
            SetMeshGroupVisible(hMesh, meshGroups[i], isVisible);
        }
    }

    int Get2DPanelWidth();  // Returns the optimal (or configured) panel width to use for m_videoWindowWidth; this is public by design.
    bool Is2DPanel(const int panelNumber) const { return (panelNumber <  GetVCPanelIDBase()); }
    bool IsVCPanel(const int panelNumber) const { return (panelNumber >= GetVCPanelIDBase()); }
    bool GetLandingTargetInfo(double &distanceOut, char * const pBaseNameOut, const int nameBufferLength);  // get distance and name of landing target (closest surface base)
	int GetVideoWindowWidth() { return m_videoWindowWidth;  }
	int GetVideoWindowHeight() { return m_videoWindowHeight; }

    // Returns the linear simulation time since simulation start, ignoring any MJD changes (edits).
    // This is the same principle as oapiGetSimTime except that it always returns a value >= the previous frame's value.
    double GetAbsoluteSimTime() const { return m_absoluteSimTime; }  

    // Returns the number of seconds since the system booted (realtime); typically has 10-16 millisecond accuracy (16 ms = 1/60th second),
    // which should suffice for normal realtime deltas.
    // Note: it is OK for this method to be static without a mutex because Orbiter is single-threaded
    static double GetSystemUptime()
    {
        return glfwGetTime();
    }
    
    //----------------------------------------------------------------------------
    // Implemented VESSEL3 callback methods; you should not normally need to override these
    //----------------------------------------------------------------------------
    virtual bool clbkLoadPanel2D(int panelID, PANELHANDLE hPanel, int viewW, int viewH) override;

	//----------------------------------------------------------------------------
    // Implemented VESSEL2 methods; you should not normally need to override these
    //----------------------------------------------------------------------------
    virtual bool clbkLoadPanel(int panelID) override;   
    virtual bool clbkPanelMouseEvent(int areaID, int event, int mx, int my) override;
    virtual bool clbkPanelRedrawEvent(int areaID, int event, SURFHANDLE surf) override;
    virtual bool clbkVCMouseEvent (int id, int event, VECTOR3 &p) override;
    virtual bool clbkVCRedrawEvent(int areaID, int event, SURFHANDLE surf)  override { return clbkPanelRedrawEvent(areaID, event, surf); }
    //----------------------------------------------------------------------------

    // you should not normally need to override these methods; however, they are virtual in case you need to sometime
    virtual bool TriggerRedrawArea(const int areaID);

    // if you hook any of these methods in your subclass, be sure to invoke the VESSEL3_EXT method as well
    virtual void clbkPreStep(double simt, double simdt, double mjd);
    virtual void clbkPostStep(double simt, double simdt, double mjd);
    virtual void clbkFocusChanged(bool getfocus, OBJHANDLE hNewVessel, OBJHANDLE hOldVessel) { m_hasFocus = getfocus; }

	// earth pressure is ~101 kpa
	bool InEarthAtm() const      { return (GetAtmPressure() >= 50e3); }
	bool IsLanded() const        { return (GroundContact() && (GetGroundspeed() < MAX_VELOCITY_FOR_WHEEL_STOP)); }  // NOTE: used to compare speed to 0, but Orbiter 2016 causes a very slight airspeed bump on startup when landed because of gear compression physics in the core
	bool IsLandedOnEarth() const { return ((GetAtmPressure() >= 95e3) && IsLanded()); }

    // pure virtual methods
    virtual int GetVCPanelIDBase() const = 0;  // subclasses should simply return VC_PANEL_ID_BASE here
    virtual int MeshTextureIDToTextureIndex(const int meshTextureID, MESHHANDLE &hMesh) = 0;  // see DeltaGliderXR1.cpp for sample implementation

    // non-virtual methods
    double GetDistanceToVessel(const VESSEL &targetVessel) const;
    const XRGrappleTargetVessel *GetGrappleTargetVessel(const char *pTargetVesselName);
    // NOTE: this should be the only place in the code that invokes SetCameraDefaultDirection: as of Orbiter 2010 P1,
    // the core SetCameraDefaultDirection call no longer actually changes the camera view -- it simply sets the *default*
    // direction.  So we must also invoke oapiCameraSetCockpitDir to change the *current* camera direction as well.
    void SetXRCameraDirection(const VECTOR3 &dir) { SetCameraDefaultDirection(dir); oapiCameraSetCockpitDir(0, 0); } 

    // WARNING: you must invoke this to work around Orbiter core bug:
    // Orbiter uses data in flag[0] in DefSetState, but GetState() does not set those flags to zero!  
    // They are unitialized!
    void GetStatusSafe(VESSELSTATUS2 &status) const;
    static void GetStatusSafe(const VESSEL &vessel, VESSELSTATUS2 &status, const bool resetToDefault);

    // static utility methods    
    static bool IsToday(int wMonth, int wDay);  // month=1-12, day=1-31
    static void SetMeshGroupVisible(DEVMESHHANDLE hMesh, int dwMeshGroup, bool isVisible);
    static int ResetAllFuelLevels(VESSEL *pVessel, const double levelFrac);
    static float ComputeVariableVolume(const double minVolume, const double maxVolume, double level);

    // data
    MESHHANDLE exmesh_tpl;        // Note: this is the *template*, so this is a *MESHHANDLE*, not a *DEVMESHHANDLE*
    VesselConfigFileParser *m_pConfig;  // our configuration file parser

    vector<MESHGROUP *> m_meshGroupVector;       // all mesh groups; initialized in clbkVisualCreated

protected:
    void WriteForced2DResolutionLogMessage(const int panelWidth) const;

    // construct panel ID key: (panelWidth * 1000) + panel ID
    // Note: panelWidth MUST be zero for VC (non-2D) panels!
    int GetPanelKey(const int panelID, const int panelWidth) { return (panelWidth * 1000) + panelID; }

    unordered_map<int, InstrumentPanel *> &GetPanelMap() { return m_panelMap; }  // returns map of all panels in this ship

    // map of our XRGrappleTargetVessels: key=vessel name, value=XRGrappleTargetVessel itself
    typedef unordered_map<const string *, XRGrappleTargetVessel *, stringhasher, stringhasher> HASHMAP_STR_XRGRAPPLETARGETVESSEL;
	typedef pair<const string *, XRGrappleTargetVessel *> str_XRGrappleTargetVessel_Pair;

    HASHMAP_STR_XRGRAPPLETARGETVESSEL m_grappleTargetMap;

private:
    // data
    int m_videoWindowWidth;                      // in pixels; 0 = UNKNOWN (NOT PARSED YET)
	int m_videoWindowHeight;					 // in pixels; 0 = UNKNOWN (NOT PARSED YET)
    int m_lastVideoWindowWidth;                  // previous m_videoWindowWidth value; -1 = not set 
    int m_last2DPanelWidth;                      // 2D panel width for m_lastVideoWindowWidth; e.g., 1280, 1600, 1920
    void *m_hModule;
    bool m_hasFocus;                             // true if we are in focus (i.e., we are the active ship), false if not
    unordered_map<int, InstrumentPanel *> m_panelMap; // map of all instrument panels: key = (panelWidth * 1000) + panel ID, value = InstrumentPanel *
    vector<PrePostStep *> m_postStepVector;      // list of PrePostStep objects; may be empty
    vector<PrePostStep *> m_preStepVector;       // list of PrePostStep objects; may be empty
    double m_absoluteSimTime;                    // linear simulation time since simulation start, ignoring any MJD changes (edits)
};

//---------------------------------------------------------------------------

// 2D coordinates on an instrument panel (2D or 3D)
struct COORD2
{    
    int x, y;

    // check whether these coordinates are in specified bounds (inclusive)
    bool InBounds(const COORD2 &topLeft, const int width, const int height) const
    {
        return ((x >= topLeft.x) && 
                (x <= (topLeft.x + width)) &&
                (y >= topLeft.y) &&
                (y <= topLeft.y + height));
    }
};

inline COORD2 _COORD2(int x, int y)
{
    COORD2 c = { x, y }; 
    return c;
};

inline COORD2 operator+ (const COORD2 &a, const COORD2 &b)
{
	COORD2 c;
	c.x = a.x + b.x;
	c.y = a.y + b.y;
    return c;
}

inline COORD2 operator- (const COORD2 &a, const COORD2 &b)
{
	COORD2 c;
	c.x = a.x - b.x;
	c.y = a.y - b.y;
    return c;
}

inline COORD2 &operator+= (COORD2 &a, const COORD2 &b)
{
	a.x += b.x;
	a.y += b.y;
	return a;
}

inline COORD2 &operator-= (COORD2 &a, const COORD2 &b)
{
	a.x -= b.x;
	a.y -= b.y;
	return a;
}

typedef unordered_map<int, InstrumentPanel *>::iterator InstrumentPanelIterator;
typedef vector<PrePostStep *>::iterator PostStepIterator;
typedef vector<PrePostStep *>::iterator PreStepIterator;
typedef vector<MESHGROUP *>::iterator MESHGROUPIterator;
