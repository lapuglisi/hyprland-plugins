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
#include <hyprland/src/managers/LayoutManager.hpp>
#undef private

#include <hyprutils/string/VarList.hpp>
using namespace Hyprutils::String;

#include "globals.hpp"

// STL
#include <iostream>
#include <filesystem> // For std::filesystem

// Do NOT change this function.
APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

//
static std::iostream& log(void) {
    static std::fstream logstream("/tmp/hyprland-tabbed.log", std::ios::out | std::ios::app);
    return logstream;
}

#ifdef _DEBUG
#define logStep(__m) \
    log() << "[" << std::filesystem::path(__FILE__).filename() << ":" << __LINE__ << "] " << __m << "\n";
#else
#define logStep(__m) (void)(0);
#endif

static inline PHLWORKSPACE getActiveWorkspace()
{
    WORKSPACEID wId = g_pCompositor->m_lastMonitor->activeSpecialWorkspaceID();
    PHLWORKSPACE workspace = g_pCompositor->getWorkspaceByID(wId);

    if (!valid(workspace))
    {
        wId = g_pCompositor->m_lastMonitor->activeWorkspaceID();
        workspace = g_pCompositor->getWorkspaceByID(wId);
    }

    if (!valid(workspace))
    {
        throw std::exception();
    }
    
    return workspace;
}

static SDispatchResult moveFocus(std::string args)
{
    PHLWORKSPACE curWorkspace = getActiveWorkspace();
    PHLWINDOW targetWindow;
    const char direction = args[0];

    PHLWINDOW curWindow = g_pCompositor->m_lastWindow.lock();
    if (!valid(curWindow)) {
        log() << "No current window" << std::endl;
        return {};
    }

    if (curWindow->m_isFloating) {
        log() << "Current window is floating, ignoring" << std::endl;
        return {};
    }

    bool loneWindow = (curWindow->getGroupSize() == 1);
    bool notGrouped = (curWindow->getGroupSize() == 0);

    log() <<
        "[hyprland-tabbed:::moveFocus]\n"
        << "args is .......... " << args << "\n"
        << "loneWindow is .... " << loneWindow << "\n"
        << "curWorkspace ..... " << curWorkspace->m_name << "\n"
        << "curWindow ........ " << curWindow->m_title << "\n";

    switch (direction)
    {
        case 'l':
        case 'd':
        {
            if (loneWindow || notGrouped)
            {
                logStep("case 'l', loneWindow || notGrouped, getWindowCicle");

                targetWindow = g_pCompositor->getWindowCycle(curWindow, true, std::nullopt, true, true);
                if (valid(targetWindow))
                {
                    logStep("targetWindow is " << targetWindow->m_title);
                    g_pCompositor->focusWindow(targetWindow, nullptr, true);
                }
                else
                {
                    logStep("targetWindow is not valid");
                }
            }
            else
            {
                if (curWindow == curWindow->getGroupHead())
                {
                    // Window is the first in a group
                    logStep("curWindow == curWindow->getGroupHead()");
                    logStep("targetWindow = g_pCompositor->getWindowCycle(curWindow, true, std::nullopt, true, true)");

                    targetWindow = g_pCompositor->getWindowCycle(curWindow, true, std::nullopt, true, true);
                    if (valid(targetWindow))
                    {
                        logStep("targetWindow is " << targetWindow->m_title);
                        g_pCompositor->focusWindow(targetWindow);
                    }
                    else
                    {
                        logStep("targetWindow is not valid");
                    }
                }
                else
                {
                    logStep("curWindow != curWindow->getGroupHead()");
                    targetWindow = curWindow->getGroupPrevious();

                    if (valid(targetWindow))
                    {
                        logStep("targetWindow is " << targetWindow->m_title);
                        g_pCompositor->focusWindow(targetWindow, nullptr, true);
                    }
                    else
                    {
                        logStep("targetWindow is not valid");
                    }
                }
            }
            break;
        }
    
        // case 'u'
        // case 'r'
        default:
        {            
            if (loneWindow || notGrouped)
            {
                logStep("case 'r', loneWindow || notGrouped, getWindowCycle");
                targetWindow = g_pCompositor->getWindowCycle(curWindow, true, std::nullopt, true, false);
                
                if (valid(targetWindow))
                {
                    logStep("targetWindow is " << targetWindow->m_title);
                    
                    g_pCompositor->focusWindow(targetWindow);

                    logStep("g_pCompositor->focusWindow(targetWindow)");
                }
                else
                {
                    logStep("targetWindow is not valid");
                }
            }
            else
            {
                if (curWindow == curWindow->getGroupTail())
                {
                    logStep("curWindow == curWindow->getGroupTail()");
                    logStep("targetWindow = g_pCompositor->getWindowCycle(curWindow, true, std::nullopt, true, false)");

                    targetWindow = g_pCompositor->getWindowCycle(curWindow, true, std::nullopt, true, false);
                    if (valid(targetWindow))
                    {
                        logStep("targetWindow is " << targetWindow->m_title);

                        g_pCompositor->focusWindow(targetWindow);
                    }
                    else
                    {
                        logStep("targetWindow is not valid");
                    }
                }
                else
                {
                    logStep("curWindow != curWindow->getGroupTail()");
                    logStep("targetWindow = curWindow->m_groupData.pNextWindow.lock()");

                    targetWindow = curWindow->m_groupData.pNextWindow.lock();
                    if (valid(targetWindow))
                    {
                        logStep("targetWindow is " << targetWindow->m_title);
                        g_pCompositor->focusWindow(targetWindow);
                    }
                    else
                    {
                        logStep("targetWindow is not valid");
                    }
                }
            }

            break;
        }
    } // switch

    logStep("return from moveFocus");
    log().flush();
    
    return SDispatchResult{};
}

static SDispatchResult moveWindow(std::string args) {
    log() << "moveWindow called with args: " << args << std::endl;

    const char direction = args[0];

    PHLWORKSPACE curWorkspace = getActiveWorkspace();
    PHLWINDOW targetWindow;
    IHyprLayout* curLayout = g_pLayoutManager->getCurrentLayout();
    
    SDispatchResult result;

    if (!curLayout)
    {
        log() << "current layout is not valid\n";
        return {};
    }

    PHLWINDOW curWindow = g_pCompositor->m_lastWindow.lock();
    if (!valid(curWindow)) {
        log() << "No current window" << std::endl;
        return {};
    }

    if (curWindow->m_isFloating) {
        log() << "Current window is floating, ignoring" << std::endl;
        return {};
    }
    
    bool loneWindow = (curWindow->getGroupSize() == 1);
    bool notGrouped = (curWindow->getGroupSize() == 0);

    switch (direction)
    {
        case 'l':
        case 'd':        
        {
            if (notGrouped || loneWindow)
            {
                logStep("curWindow is not grouped || is loneWindow.");
                
                targetWindow = g_pCompositor->getWindowInDirection(curWindow, 'l');
                if (valid(targetWindow))
                {
                    logStep("targetWindow is " << targetWindow->m_title);
                    // Check if targetWindow is in a group
                    if (targetWindow->getGroupSize() > 0)
                    {
                        logStep("targetWindow is grouped");

                        g_pKeybindManager->moveWindowIntoGroup(curWindow, targetWindow);

                        logStep("g_pKeybindManager->moveWindowIntoGroup(curWindow, targetWindow)");
                    }
                    else
                    {
                        logStep("targetWindow is not grouped");
                        
                        result = g_pKeybindManager->moveWindowOrGroup("l");

                        logStep("g_pKeybindManager->moveGroupWindow('prev')");
                        logStep("  result is: " << (result.success ? "ok" : result.error));
                    }
                }
            }
            else
            {
                if (curWindow->getGroupHead() == curWindow)
                {
                    logStep("curWindow->getGroupHead() == curWindow");

                    g_pKeybindManager->moveWindowOutOfGroup(curWindow, "l");

                    logStep("g_pKeybindManager->moveWindowOutOfGroup(curWindow, 'l')");
                }
                else
                {
                    logStep("curWindow->getGroupHead() != curWindow");
                    
                    result = g_pKeybindManager->moveGroupWindow("prev");

                    logStep("g_pKeybindManager->moveGroupWindow('prev')");
                    logStep("  result is " << (result.success ? "ok" : result.error));
                }
            }
            break;
        }

        //case 'u':
        //case 'r':
        default:
        {
            if (notGrouped || loneWindow)
            {
                logStep("curWindow is not grouped || is loneWindow.");
                
                targetWindow = g_pCompositor->getWindowInDirection(curWindow, 'r');
                if (valid(targetWindow))
                {
                    logStep("targetWindow is " << targetWindow->m_title);
                    // Check if targetWindow is in a group
                    if (targetWindow->getGroupSize() > 0)
                    {
                        logStep("targetWindow is grouped");

                        g_pKeybindManager->moveWindowIntoGroup(curWindow, targetWindow);

                        logStep("g_pKeybindManager->moveWindowIntoGroup(curWindow, targetWindow)");
                    }
                    else
                    {
                        logStep("targetWindow is not grouped");
                        
                        result = g_pKeybindManager->moveWindowOrGroup("r");

                        logStep("result = g_pKeybindManager->moveWindowOrGroup('r')");
                        logStep("  result is " << (result.success ? "ok" : result.error))
                    }
                }
                else
                {
                    logStep("targetWindow is not valid");
                }
            }
            else
            {
                if (curWindow->getGroupTail() == curWindow)
                {
                    logStep("curWindow->getGroupTail() == curWindow");
                    
                    g_pKeybindManager->moveWindowOutOfGroup(curWindow, "r");

                    logStep("g_pKeybindManager->moveWindowOutOfGroup(curWindow, 'r')");
                }
                else
                {
                    logStep("curWindow->getGroupTail() != curWindow");
                    
                    result = g_pKeybindManager->moveGroupWindow("next");

                    logStep("g_pKeybindManager->moveGroupWindow('next')");
                    logStep("  result is " << (result.success ? "ok" : result.error))
                }
            }
            break;
        }
    }

    return result;
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
    success      = success && HyprlandAPI::addDispatcherV2(PHANDLE, "plugin:hyprland-tabbed:movefocus", moveFocus);
    success      = success && HyprlandAPI::addDispatcherV2(PHANDLE, "plugin:hyprland-tabbed:movewindow", moveWindow);

    if (success)
    {
      HyprlandAPI::addNotification(PHANDLE, "[hyprland-tabbed] Initialized successfully!", CHyprColor{0.2, 1.0, 0.2, 1.0}, 5000);
    }
    else
    {
      HyprlandAPI::addNotification(PHANDLE, "[hyprland-tabbed] init error: failed to register dispatchers", CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
      throw std::runtime_error("[hyprland-tabbed] Dispatchers failed");
    }

    return {"hyprland-tabbed", "A plugin to add a tabbed layout to hyprland, sway-like", "lapuglisi", "1.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    try
    {
        log().flush();
    }
    catch (...)
    {
    }
    ;
}