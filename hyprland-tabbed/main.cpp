#define WLR_USE_UNSTABLE

#include <unistd.h>

#include <hyprland/src/includes.hpp>
#include <sstream>

#define private public
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/Window.hpp>
#include <hyprland/src/config/ConfigManager.hpp>
#include <hyprland/src/render/Renderer.hpp>
#include <hyprland/src/managers/KeybindManager.hpp>
#undef private

#include <hyprutils/string/VarList.hpp>
using namespace Hyprutils::String;

#include "globals.hpp"

// Do NOT change this function.
APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

//
static SDispatchResult moveFocus(std::string in) {
    return SDispatchResult{};
}

static SDispatchResult moveWindow(std::string in) {
    return SDispatchResult{};
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    const std::string HASH = __hyprland_api_get_hash();

    if (HASH != GIT_COMMIT_HASH) {
        HyprlandAPI::addNotification(PHANDLE, "[hyprland-tabbed] Error: Version mismatch (headers ver != hyprland ver)",
                                     CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[hyprland-tabbed] Version mismatch");
    }

    bool success = true;
    success      = success && HyprlandAPI::addDispatcherV2(PHANDLE, "plugin:hyprland-tabbed:movefocus", ::moveFocus);
    success      = success && HyprlandAPI::addDispatcherV2(PHANDLE, "plugin:hyprland-tabbed:movewindow", ::moveWindow);

    if (success)
    {
      HyprlandAPI::addNotification(PHANDLE, "[hyprland-tabbed] Initialized successfully!", CHyprColor{0.2, 1.0, 0.2, 1.0}, 5000);
    }
    else
    {
      HyprlandAPI::addNotification(PHANDLE, "[hyprland-tabbed] init error: failed to register dispatchers", CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
      throw std::runtime_error("[hyprland-tabbed] Dispatchers failed");
    }

    return {"hyprland-tabbed", "A plugin to add a tabbed layout, sway-like, to hyprland", "lapuglisi", "1.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    ;
}