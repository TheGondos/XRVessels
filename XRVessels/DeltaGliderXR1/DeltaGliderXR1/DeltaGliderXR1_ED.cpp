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
#include "ScnEditorAPI.h"
#include "font_awesome_5.h"
#include <imgui-knobs.h>

/*
static HELPCONTEXT g_hc = {
    "html/vessels/deltaglider.chm",
        0,
        "html/vessels/deltaglider.chm::/deltaglider.hhc",
        "html/vessels/deltaglider.chm::/deltaglider.hhk"
};
*/

#if 0  // this was not compatible with UMMU to begin with
// --------------------------------------------------------------
// Message procedure for editor page 2 (passengers)
// --------------------------------------------------------------
INT_PTR CALLBACK EdPg2Proc (HWND hTab, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DeltaGliderXR1 *dg;
    
    switch (uMsg) {
    case WM_INITDIALOG: 
        {
            char cbuf[256];
            dg = static_cast<DeltaGliderXR1 *>(oapiGetVesselInterface((OBJHANDLE)lParam));
            for (int i=0; i < MAX_PASSENGERS; i++)  
            {
                const char *pName = dg->GetCrewNameBySlotNumber(i);
                bool passengerOnBoard = dg->IsCrewMemberOnBoard(i);
                SendDlgItemMessage (hTab, IDC_CHECK1+i, BM_SETCHECK, passengerOnBoard ? BST_CHECKED : BST_UNCHECKED, 0);
            }
            sprintf (cbuf, "%0.2f kg", dg->GetMass());
            SetWindowText (GetDlgItem (hTab, IDC_MASS), cbuf);
            break;
        } 
    case WM_COMMAND:
        switch (LOWORD (wParam))
        {
        case IDC_CHECK1:
        case IDC_CHECK2:
        case IDC_CHECK3:
        case IDC_CHECK4: 
        case IDC_CHECK5: 
            {
                // WARNING: this is not compatible with UMmu crew management because UMmu reorders the slots (0-4) each time a crew member is added or removed.
                char cbuf[256];
                LRESULT lResult = SendDlgItemMessage (hTab, LOWORD(wParam), BM_GETCHECK, 0, 0);
                dg = GetDG(hTab);

                // get current crew member for this slot
                const int index = LOWORD(wParam)-IDC_CHECK1; // 0...n
                const char *pName = dg->GetCrewNameBySlotNumber(index);
                if (lResult)    // adding crew member?
                {
                    if (strlen(pName) == 0)     // not already on board?
                    {
                        // add default crew member for this slot
                        CrewMember *pCM = dg->GetXR1Config()->CrewMembers + index;
                        dg->UMmu.AddCrewMember(pCM->name, pCM->age, pCM->pulse, pCM->mass, pCM->rank);
                    }
                }
                else    // removing crew member
                {
                    if (strlen(pName) > 0)  // is crew member on board?
                    {
                        // UMmu bug: must cast away constness here
                        dg->UMmu.RemoveCrewMember(const_cast<char *>(pName));   // remove him
                    }
                }

                dg->SetPassengerVisuals();
                dg->SetEmptyMass();
                sprintf (cbuf, "%0.2f kg", dg->GetMass());
                SetWindowText (GetDlgItem (hTab, IDC_MASS), cbuf);
                break;
            } 
        }
        break;
    }
    return FALSE;
}
#endif

// --------------------------------------------------------------
// Message procedure for editor page 3 (damage)
// --------------------------------------------------------------
#if 0
INT_PTR CALLBACK EdPg3Proc (HWND hTab, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DeltaGliderXR1 *dg;
    
    switch (uMsg) {
    case WM_INITDIALOG: {
        dg = static_cast<DeltaGliderXR1 *>(oapiGetVesselInterface(reinterpret_cast<OBJHANDLE>(lParam)));
        GAUGEPARAM gp = { 0, 100, GAUGEPARAM::LEFT, GAUGEPARAM::BLACK };
        oapiSetGaugeParams (GetDlgItem (hTab, IDC_LEFTWING_SLIDER), &gp);
        oapiSetGaugeParams (GetDlgItem (hTab, IDC_RIGHTWING_SLIDER), &gp);
        UpdateDamage (hTab, dg);
                        } break;
    case WM_COMMAND:
        /* NOT IMPLEMENTED
        switch (LOWORD (wParam)) {
        case IDC_REPAIR:
            dg = GetDG(hTab);
            dg->RepairDamage ();
            UpdateDamage (hTab, dg);
            return TRUE;
        }
        break;
        */
        case WM_HSCROLL:
            dg = GetDG(hTab);
            int id = GetDlgCtrlID ((HWND)lParam);
            switch (id) {
            case IDC_LEFTWING_SLIDER:
            case IDC_RIGHTWING_SLIDER:
                switch (LOWORD (wParam)) {
                case SB_THUMBTRACK:
                case SB_LINELEFT:
                case SB_LINERIGHT:
                    if (id == IDC_LEFTWING_SLIDER)
                        dg->lwingstatus = HIWORD(wParam)*0.01;
                    else
                        dg->rwingstatus = HIWORD(wParam)*0.01;
                    dg->ApplyDamage ();
                    UpdateDamage (hTab, dg);
                    return TRUE;
                }
                break;
            }
            break;
    }
    return FALSE;
}
#endif
// --------------------------------------------------------------
// Add vessel-specific pages into scenario editor
// --------------------------------------------------------------


static void DrawState(DoorStatus status, std::array<const char *, 4> &&desc) {
    const char *txt;
    switch(status) {
        case DoorStatus::DOOR_FAILED:   txt = "Failed"; break;
        case DoorStatus::DOOR_CLOSED:   txt = desc[0]; break;
        case DoorStatus::DOOR_CLOSING:  txt = desc[1]; break;
        case DoorStatus::DOOR_OPENING:  txt = desc[2]; break;
        case DoorStatus::DOOR_OPEN:     txt = desc[3]; break;
        default: // fallthrough
        case DoorStatus::NOT_SET:       txt = "Not set"; break;
    }

	ImGui::SetNextItemWidth(80.0f);
	ImGui::BeginDisabled(true);
	ImGui::SameLine();
	ImGui::TextUnformatted(txt);
	ImGui::SameLine();
	ImGui::EndDisabled();
}

static void DrawControl(const char *name, double ratio, DoorStatus &status, std::function<void(DoorStatus)> onChange,
                        std::array<const char *, 2> &&btn, std::array<const char *, 4> &&desc) {
    const ImVec2 button_sz(ImVec2(60, 20));
    ImGui::BeginGroupPanel(name, ImVec2(ImGui::GetContentRegionAvail().x * ratio, 0));
    ImGui::PushID(name);
    if(ImGui::Button(btn[0], button_sz)) { onChange(DoorStatus::DOOR_CLOSED); }
    DrawState(status, std::move(desc));
    if(ImGui::Button(btn[1], button_sz)) { onChange(DoorStatus::DOOR_OPEN); }
    ImGui::PopID();
    ImGui::EndGroupPanel();

}

static void DrawXR1Controls(DeltaGliderXR1 *dg) {
	const ImVec2 button_sz(ImVec2(60, 20));

    DoorStatus orgAPUState = dg->apu_status;

    // hotwire the apu to ON so we can move the doors by "cheating" here
    dg->apu_status = DoorStatus::DOOR_OPEN;

    DrawControl("Landing gear", 0.5, dg->gear_status,
                [dg](DoorStatus s) {dg->ActivateLandingGear (s);},
                {"Up","Down"},{"Raised", "Raising", "Lowering", "Lowered"});
    ImGui::SameLine();
    DrawControl("Air brake", 1.0, dg->brake_status,
                [dg](DoorStatus s) {dg->ActivateAirbrake (s);},
                {"Stow","Deploy"},{"Stowed", "Stowing", "Extending", "Extended"});


    DrawControl("Outer airlock", 0.5, dg->olock_status,
                [dg](DoorStatus s) {dg->ActivateOuterAirlock (s);},
                {"Close","Open"},{"Closed", "Closing", "Opening", "Opened"});
    ImGui::SameLine();
    DrawControl("Inner airlock", 1.0, dg->ilock_status,
                [dg](DoorStatus s) {dg->ActivateInnerAirlock (s);},
                {"Close","Open"},{"Closed", "Closing", "Opening", "Opened"});


    DrawControl("Nose cone", 0.5, dg->nose_status,
                [dg](DoorStatus s) {
                    switch(s) {
                        case DoorStatus::DOOR_CLOSED:
                            dg->ActivateOuterAirlock (s);
                            dg->ActivateNoseCone (s);
                            break;
                        case DoorStatus::DOOR_OPEN:
                            dg->ActivateNoseCone (s);
                            break;
                        default:
                            break;
                    }
                },
                {"Close","Open"},{"Closed", "Closing", "Opening", "Opened"});
    ImGui::SameLine();
    DrawControl("Hatch", 1.0, dg->hatch_status,
                [dg](DoorStatus s) {dg->ActivateHatch (s);},
                {"Close","Open"},{"Closed", "Closing", "Opening", "Opened"});


    DrawControl("SCRAM doors", 0.5, dg->scramdoor_status,
                [dg](DoorStatus s) {dg->ActivateScramDoors (s);},
                {"Close","Open"},{"Closed", "Closing", "Opening", "Opened"});
    ImGui::SameLine();
    DrawControl("Hover doors", 1.0, dg->hoverdoor_status,
                [dg](DoorStatus s) {dg->ActivateHoverDoors (s);},
                {"Close","Open"},{"Closed", "Closing", "Opening", "Opened"});


    DrawControl("Radiator", 0.5, dg->radiator_status,
                [dg](DoorStatus s) {dg->ActivateRadiator (s);},
                {"Close","Open"},{"Closed", "Closing", "Opening", "Opened"});
    ImGui::SameLine();
    DrawControl("Ladder", 1.0, dg->ladder_status,
                [dg](DoorStatus s) {dg->ActivateLadder (s);},
                {"Close","Open"},{"Closed", "Closing", "Opening", "Opened"});


    DrawControl("Retro doors", 0.5, dg->rcover_status,
                [dg](DoorStatus s) {dg->ActivateRCover (s);},
                {"Close","Open"},{"Closed", "Closing", "Opening", "Opened"});
    ImGui::SameLine();
    DrawControl("Chamber", 1.0, dg->chamber_status,
                [dg](DoorStatus s) {dg->ActivateChamber (s, true);},
                {"Close","Open"},{"Closed", "Closing", "Opening", "Opened"});


    // restore original APU state
    dg->apu_status = orgAPUState;
}

void DrawXR1Damage(DeltaGliderXR1 *dg) {
    ImGui::BeginGroupPanel("Wings integrity");
    float tmp = dg->lwingstatus * 100.0;
    if (ImGuiKnobs::Knob("Left", &tmp, 0.0f, 100.0f, 1.0f, "%.2f%%", ImGuiKnobVariant_WiperOnly, 50.0, ImGuiKnobFlags_DragHorizontal)) {
        dg->lwingstatus = tmp / 100.0;
        dg->ApplyDamage ();
    }
    ImGui::SameLine();
    tmp = dg->rwingstatus * 100.0;
    if (ImGuiKnobs::Knob("Right", &tmp, 0.0f, 100.0f, 1.0f, "%.2f%%", ImGuiKnobVariant_WiperOnly, 50.0, ImGuiKnobFlags_DragHorizontal)) {
        dg->rwingstatus = tmp / 100.0;
        dg->ApplyDamage ();
    }
    ImGui::EndGroupPanel();
}

static void DrawScnEditorTabs(OBJHANDLE hVessel) {
	DeltaGliderXR1 *dg = (DeltaGliderXR1*)oapiGetVesselInterface (hVessel);
	if(ImGui::BeginTabItem(ICON_FA_DRAFTING_COMPASS" Animations")) {
		DrawXR1Controls(dg);
		ImGui::EndTabItem();
	}
	if(ImGui::BeginTabItem(ICON_FA_WRENCH " Damage")) {
        DrawXR1Damage(dg);
		ImGui::EndTabItem();
	}
}

// --------------------------------------------------------------
// Add vessel-specific pages into scenario editor
// --------------------------------------------------------------

DLLCLBK ScnDrawCustomTabs secInit ()
{
	return DrawScnEditorTabs;
}
