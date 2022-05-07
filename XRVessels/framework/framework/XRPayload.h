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
// XRPayload.h
// Defines custom payload data for XR-class vessels
// ==============================================================

#pragma once

#include "OrbiterAPI.h"
#include "stringhasher.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <cstring>

// define externs explicitly so we aren't tied to a specific XR-class vessel
extern const VECTOR3 PAYLOAD_SLOT_DIMENSIONS;
extern const char *DEFAULT_PAYLOAD_THUMBNAIL_PATH;

class XRPayloadClassData;

// hashmap: string -> vector of integers 
typedef unordered_map<string , vector<int> *> HASHMAP_STR_VECINT;

// hashmap: string -> XRPayload object
typedef unordered_map<string , XRPayloadClassData *> HASHMAP_STR_XRPAYLOAD;

// vector of XRPayloadClassData objects
typedef vector<const XRPayloadClassData *> VECTOR_XRPAYLOAD;

//=========================================================================

// defines the dimensions of all XR Payload thumbnails
const int PAYLOAD_THUMBNAIL_DIMX = 154;
const int PAYLOAD_THUMBNAIL_DIMY = 77;

// immutable payload class data
class XRPayloadClassData
{
public:
    static const XRPayloadClassData &GetXRPayloadClassDataForClassname(const char *pClassname);
    static void Terminate();  // clients should invoke this from their ExitModule method
    static void InitializeXRPayloadClassData();  // clients must invoke this from a one-shot PostStep one second after the simulation starts so that all XR payload vessels are loaded
    static const XRPayloadClassData **GetAllAvailableXRPayloads();  // returns all XRPayloads available in the config\vessels directory
    static ATTACHMENTHANDLE GetAttachmentHandleForPayloadVessel(const VESSEL &childVessel);
    static double getLongestYTouchdownPoint(const VESSEL &vessel);

    //===========================================================
    
    void AddExplicitAttachmentSlot(const char *pParentVesselClassname, int slotNumber);
    bool AreAnyExplicitAttachmentSlotsDefined(const char *pParentVesselClassname) const;
    bool IsExplicitAttachmentSlotAllowed(const char *pParentVesselClassname, int slotNumber) const;

    const char *GetClassname() const         { return m_pClassname; }      // will never be null
    const char *GetConfigFilespec() const    { return m_pConfigFilespec; } // will never be null
    const char *GetDescription() const       { return m_pDescription; }    // will never be null
    const VECTOR3 &GetDimensions() const     { return m_dimensions; }
    const VECTOR3 &GetSlotsOccupied() const  { return m_slotsOccupied; }
    const VECTOR3 &GetPrimarySlotCenterOfMassOffset() const { return m_primarySlotCenterOfMassOffset; }
    bool IsXRPayloadEnabled() const          { return m_isXRPayloadEnabled; }
    bool IsXRConsumableTank() const          { return m_isXRConsumableTank; }
    double GetMass() const                   { return m_mass; }
    const VECTOR3 &GetGroundDeploymentAdjustment() const { return m_groundDeploymentAdjustment; }
    SURFHANDLE GetThumbnailBitmapHandle() const { return m_hThumbnailBitmap; }  // may be null
    
    // operator overloading
    bool operator==(const XRPayloadClassData &that) const { return (strcmp(m_pClassname, that.m_pClassname) == 0); }  // vessel classnames are unique

protected:
    const char *m_pClassname;       // vessel bare classname (no leading path)
    const char *m_pConfigFilespec;  // config-relative path of vessel's .cfg file; e.g., "Vessel\XRParts.cfg"
    char *m_pDescription;           // cosmetic description
    VECTOR3 m_dimensions;       // width (X), height (Y), length (Z)
    VECTOR3 m_slotsOccupied;    // width (X), height (Y), length (Z)
    VECTOR3 m_primarySlotCenterOfMassOffset;  // X,Y,Z
    SURFHANDLE m_hThumbnailBitmap; // will be nullptr if bitmap is no defined or is invalid
    HASHMAP_STR_VECINT m_explicitAttachmentSlotsMap;   // key=vessel classname, value=list of ship bay slots to which this object may attach (assuming sufficient room).    
    bool m_isXRPayloadEnabled;  // true if this vessel is enabled for docking in the bay, false otherwise
    bool m_isXRConsumableTank;  // true if this vessel contains XR fuel consumable by the parent ship.
    double m_mass;              // nominal mass
    VECTOR3 m_groundDeploymentAdjustment;

private:
    // NOTE: these are 'private' by design to prevent incorrect instantiation: all client code should go through
    // the static GetXRPayloadClassDataForClassname to retrieve XRPayloadClassData data.
    XRPayloadClassData(const char *pConfigFilespec, const char *pClassname);
    virtual ~XRPayloadClassData();
    
    static HASHMAP_STR_XRPAYLOAD s_classnameToXRPayloadClassDataMap;
    static const XRPayloadClassData **s_allXRPayloadEnabledClassData;  // cached list of all XRPayload-enabled vessels objects in the Orbiter config directory, null-terminated
};
