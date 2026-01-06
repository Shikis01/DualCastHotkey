#include <SKSE/SKSE.h>
#include <RE/Skyrim.h>
#include "SKSE/Logger.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <string>

using namespace SKSE;
using namespace RE;

// DualCastHotkey - a lightweight SKSE plugin that allows triggering a dual-cast
// via a configurable hotkey. Configuration is stored in an INI at
// Data\SKSE\Plugins\DualCastHotkey.ini. No MCM or Papyrus is required.

// FormID of Ordinator's Dual Cast Destruction Perk (not used by default, kept for compatibility and testing)
constexpr RE::FormID kDualCastPerkID = 0x000153CF;

// Configurable hotkey (H by default)
static std::uint32_t g_hotkey = static_cast<std::uint32_t>(BSKeyboardDevice::Key::kH);

// Charge status (simple global)
static bool g_isCharging = false;
static RE::FormID g_chargingSpellID = 0;
static std::chrono::steady_clock::time_point g_chargeStart;

// Config file helpers
// SaveConfig() writes a human-friendly INI with instructions for the player.
// LoadConfig() reads the configured hotkey (supports letters, common names like
// 'Space' or 'F1', or numeric key codes). If the INI is missing we create a
// default one with a brief header so players can change it manually.
#include <filesystem>
#include <algorithm>

static std::string KeyToString(std::uint32_t a_key)
{
    using K = BSKeyboardDevice::Key;
    switch (static_cast<K>(a_key)) {
        case K::kA: return "A";
        case K::kB: return "B";
        case K::kC: return "C";
        case K::kD: return "D";
        case K::kE: return "E";
        case K::kF: return "F";
        case K::kG: return "G";
        case K::kH: return "H";
        case K::kI: return "I";
        case K::kJ: return "J";
        case K::kK: return "K";
        case K::kL: return "L";
        case K::kM: return "M";
        case K::kN: return "N";
        case K::kO: return "O";
        case K::kP: return "P";
        case K::kQ: return "Q";
        case K::kR: return "R";
        case K::kS: return "S";
        case K::kT: return "T";
        case K::kU: return "U";
        case K::kV: return "V";
        case K::kW: return "W";
        case K::kX: return "X";
        case K::kY: return "Y";
        case K::kZ: return "Z";
        case K::kNum0: return "0";
        case K::kNum1: return "1";
        case K::kNum2: return "2";
        case K::kNum3: return "3";
        case K::kNum4: return "4";
        case K::kNum5: return "5";
        case K::kNum6: return "6";
        case K::kNum7: return "7";
        case K::kNum8: return "8";
        case K::kNum9: return "9";
        case K::kSpacebar: return "Space";
        case K::kEnter: return "Enter";
        default: {
            std::ostringstream ss; ss << static_cast<uint32_t>(a_key); return ss.str();
        }
    }
}

static std::uint32_t StringToKey(std::string s)
{
    using K = BSKeyboardDevice::Key;
    // Trim
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
    // Uppercase copy
    std::string up = s; std::transform(up.begin(), up.end(), up.begin(), ::toupper);

    if (up.size() == 1 && std::isalpha(up[0])) {
        char c = up[0];
        switch (c) {
            case 'A': return static_cast<std::uint32_t>(K::kA);
            case 'B': return static_cast<std::uint32_t>(K::kB);
            case 'C': return static_cast<std::uint32_t>(K::kC);
            case 'D': return static_cast<std::uint32_t>(K::kD);
            case 'E': return static_cast<std::uint32_t>(K::kE);
            case 'F': return static_cast<std::uint32_t>(K::kF);
            case 'G': return static_cast<std::uint32_t>(K::kG);
            case 'H': return static_cast<std::uint32_t>(K::kH);
            case 'I': return static_cast<std::uint32_t>(K::kI);
            case 'J': return static_cast<std::uint32_t>(K::kJ);
            case 'K': return static_cast<std::uint32_t>(K::kK);
            case 'L': return static_cast<std::uint32_t>(K::kL);
            case 'M': return static_cast<std::uint32_t>(K::kM);
            case 'N': return static_cast<std::uint32_t>(K::kN);
            case 'O': return static_cast<std::uint32_t>(K::kO);
            case 'P': return static_cast<std::uint32_t>(K::kP);
            case 'Q': return static_cast<std::uint32_t>(K::kQ);
            case 'R': return static_cast<std::uint32_t>(K::kR);
            case 'S': return static_cast<std::uint32_t>(K::kS);
            case 'T': return static_cast<std::uint32_t>(K::kT);
            case 'U': return static_cast<std::uint32_t>(K::kU);
            case 'V': return static_cast<std::uint32_t>(K::kV);
            case 'W': return static_cast<std::uint32_t>(K::kW);
            case 'X': return static_cast<std::uint32_t>(K::kX);
            case 'Y': return static_cast<std::uint32_t>(K::kY);
            case 'Z': return static_cast<std::uint32_t>(K::kZ);
        }
    }

    if (up == "SPACE") return static_cast<std::uint32_t>(K::kSpacebar);
    if (up == "ENTER") return static_cast<std::uint32_t>(K::kEnter);

    // F1..F12
    if (up.size() >= 2 && up[0] == 'F') {
        try {
            int f = std::stoi(up.substr(1));
            switch (f) {
                case 1: return static_cast<std::uint32_t>(K::kF1);
                case 2: return static_cast<std::uint32_t>(K::kF2);
                case 3: return static_cast<std::uint32_t>(K::kF3);
                case 4: return static_cast<std::uint32_t>(K::kF4);
                case 5: return static_cast<std::uint32_t>(K::kF5);
                case 6: return static_cast<std::uint32_t>(K::kF6);
                case 7: return static_cast<std::uint32_t>(K::kF7);
                case 8: return static_cast<std::uint32_t>(K::kF8);
                case 9: return static_cast<std::uint32_t>(K::kF9);
                case 10: return static_cast<std::uint32_t>(K::kF10);
                case 11: return static_cast<std::uint32_t>(K::kF11);
                case 12: return static_cast<std::uint32_t>(K::kF12);
                default: break;
            }
        } catch(...) {}
    }

    // Numeric codes
    try {
        return static_cast<std::uint32_t>(std::stoul(s));
    } catch(...) {}

    // Fallback: default to H
    return static_cast<std::uint32_t>(K::kH);
}

static void SaveConfig()
{
    std::error_code ec;
    std::filesystem::create_directories("Data\\SKSE\\Plugins", ec);
    std::ofstream out("Data\\SKSE\\Plugins\\DualCastHotkey.ini");
    if (!out) return;

    out << "; DualCastHotkey configuration file\n";
    out << "; Set `Hotkey` to a single character (A-Z, 0-9), a common name (Space, Enter, F1..F12), or a numeric key code.\n";
    out << "; Examples: Hotkey=H   Hotkey=Space   Hotkey=59\n";
    out << "; Default: H\n\n";

    // Attempt to write a human-friendly name if possible
    out << "Hotkey=" << KeyToString(g_hotkey) << "\n";
}

static void LoadConfig()
{
    std::ifstream in("Data\\SKSE\\Plugins\\DualCastHotkey.ini");
    if (!in) {
        // Create default config with instructions
        SaveConfig();
        return;
    }

    std::string line;
    while (std::getline(in, line)) {
        if (line.rfind("Hotkey=", 0) == 0) {
            std::string tok = line.substr(7);
            // Strip potential whitespace
            tok.erase(tok.begin(), std::find_if(tok.begin(), tok.end(), [](unsigned char ch) { return !std::isspace(ch); }));
            tok.erase(std::find_if(tok.rbegin(), tok.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), tok.end());
            if (!tok.empty()) {
                g_hotkey = StringToKey(tok);
            }
        }
    }
}

// DualCastListener: receives raw input events from the game. On configured
// hotkey down, it verifies that the player has spells equipped in both hands
// (we only require 'is a Spell', not that both spells are identical), and then
// simulates the press of left+right+dual attack user events to start charging.
// On hotkey up, it computes the held duration and sends the corresponding
// release events with the measured heldDownSecs so the game handles charge
// release correctly.
class DualCastListener : public BSTEventSink<InputEvent*>
{
public:
    virtual BSEventNotifyControl ProcessEvent(InputEvent* const* evns, BSTEventSource<InputEvent*>*) override
    {
        if (!evns) return BSEventNotifyControl::kContinue;

        auto player = PlayerCharacter::GetSingleton();
        if (!player) return BSEventNotifyControl::kContinue;

        for (auto e = *evns; e; e = e->next)
        {
            if (e->GetEventType() != INPUT_EVENT_TYPE::kButton)
                continue;

            auto* button = e->AsButtonEvent();
            if (!button)
                continue;

            if (button->device != INPUT_DEVICE::kKeyboard)
                continue;

            if (button->GetIDCode() != g_hotkey)
                continue;

            // Distinct DOWN (start) y UP (end)
            if (button->IsDown())
            {
                // If already charging, ignore any other input from the same key
                if (g_isCharging) continue;

                // Obtain spells equipped in both hands
                auto* leftSpell = player->GetEquippedObject(true);   // left hand
                auto* rightSpell = player->GetEquippedObject(false);  // right hand

                if (!leftSpell || !rightSpell)
                {
                    continue;
                }

                if (leftSpell->GetSavedFormType() != FormType::Spell ||
                    rightSpell->GetSavedFormType() != FormType::Spell)
                {
                    continue;
                }

                // We only require that both hands have spells (doesn't matter if they're the same)
                // If one of the items are not a spell it was already checked up and will continue

                // No perk comprobation: Allow dualcasting per hotkey only if both hands have spells

                // Start charge, send press events
                auto* inputMgr = BSInputDeviceManager::GetSingleton();
                if (!inputMgr) {
                    continue;
                }

                auto* ue = UserEvents::GetSingleton();
                if (!ue) {
                    continue;
                }

                auto* leftPress = ButtonEvent::Create(INPUT_DEVICE::kMouse, ue->leftAttack, BSWin32MouseDevice::Key::kLeftButton, 1.0f, 0.0f);
                auto* rightPress = ButtonEvent::Create(INPUT_DEVICE::kMouse, ue->rightAttack, BSWin32MouseDevice::Key::kRightButton, 1.0f, 0.0f);
                auto* dualPress = ButtonEvent::Create(INPUT_DEVICE::kMouse, ue->dualAttack, 0, 1.0f, 0.0f);

                InputEvent* evtPtr = leftPress;
                inputMgr->SendEvent(&evtPtr);
                evtPtr = rightPress;
                inputMgr->SendEvent(&evtPtr);
                evtPtr = dualPress;
                inputMgr->SendEvent(&evtPtr);

                // Mark charge status and time
                g_isCharging = true;
                g_chargingSpellID = leftSpell->formID;
                g_chargeStart = std::chrono::steady_clock::now();
            }
            else if (button->IsUp())
            {
                // End of charge; only if we were charging
                if (!g_isCharging) continue;

                auto now = std::chrono::steady_clock::now();
                std::chrono::duration<float> elapsed = now - g_chargeStart;
                float heldSecs = elapsed.count();
                if (heldSecs < 0.01f) heldSecs = 0.01f; // small minimun

                auto* inputMgr = BSInputDeviceManager::GetSingleton();
                auto* ue = UserEvents::GetSingleton();
                if (inputMgr && ue)
                {
                    // Send release event
                    auto* leftRelease = ButtonEvent::Create(INPUT_DEVICE::kMouse, ue->leftAttack, BSWin32MouseDevice::Key::kLeftButton, 0.0f, heldSecs);
                    auto* rightRelease = ButtonEvent::Create(INPUT_DEVICE::kMouse, ue->rightAttack, BSWin32MouseDevice::Key::kRightButton, 0.0f, heldSecs);
                    auto* dualRelease = ButtonEvent::Create(INPUT_DEVICE::kMouse, ue->dualAttack, 0, 0.0f, heldSecs);

                    InputEvent* evtPtr = leftRelease;
                    inputMgr->SendEvent(&evtPtr);
                    evtPtr = rightRelease;
                    inputMgr->SendEvent(&evtPtr);
                    evtPtr = dualRelease;
                    inputMgr->SendEvent(&evtPtr);

                    {
                        // Silent: free charge
                    }
                }
                else
                {
                    // Silent: failed to send release
                }

                // Reset status
                g_isCharging = false;
                g_chargingSpellID = 0;
            }
        }

        return BSEventNotifyControl::kContinue;
    }
};

// Global instancy
DualCastListener g_dualCastListener;

// Callback SKSE Messaging
void OnSKSEMessage(MessagingInterface::Message* msg)
{
    if (msg->type == MessagingInterface::kInputLoaded)
    {
        auto* inputMgr = BSInputDeviceManager::GetSingleton();
        if (inputMgr)
        {
            inputMgr->AddEventSink(&g_dualCastListener);
        }

    }
}

// Papyrus/MCM support intentionally removed - configuration is via INI only.
// SKSEPluginLoad might be underlined with red (signaling error) but project can still be built
extern "C" __declspec(dllexport) bool SKSEPluginLoad(const LoadInterface* skse)
{
    Init(skse);

    // Load configuration (if present) - creates default INI if missing
    LoadConfig();

    // No Papyrus registration: MCM support was intentionally discarded.
    // Too complicated for my beginner's hands

    // Register input listener via SKSE messaging (InputLoaded)
    if (auto* messaging = GetMessagingInterface())
        messaging->RegisterListener(OnSKSEMessage);

    return true;
}