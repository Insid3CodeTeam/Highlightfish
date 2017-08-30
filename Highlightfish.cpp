#include <windows.h>
#include <stdio.h>

//----------------- From original plugin PDK (begin) -----------------//
#define PLUG_EXP __declspec(dllexport)
#define PLUG_IMP __declspec(dllimport)

//default structure alignments forced
#ifdef _WIN64
#pragma pack(push, 16)
#else //x86
#pragma pack(push, 8)
#endif //_WIN64

//defines
#define PLUG_SDKVERSION 1

//structures
typedef struct {
    //provided by the debugger
    int pluginHandle;
    //provided by the pluginit function
    int sdkVersion;
    int pluginVersion;
    char pluginName[256];
} PLUG_INITSTRUCT;

typedef struct {
    //provided by the debugger
    HWND hwndDlg; //gui window handle
    int hMenu; //plugin menu handle
    int hMenuDisasm; //plugin disasm menu handle
    int hMenuDump; //plugin dump menu handle
    int hMenuStack; //plugin stack menu handle
} PLUG_SETUPSTRUCT;


//callback structures
typedef struct {
    const char *szFileName;
} PLUG_CB_INITDEBUG;

typedef struct {
    int hEntry;
} PLUG_CB_MENUENTRY;


//enums
typedef enum {
    CB_INITDEBUG, //PLUG_CB_INITDEBUG
    CB_STOPDEBUG, //PLUG_CB_STOPDEBUG
    CB_CREATEPROCESS, //PLUG_CB_CREATEPROCESS
    CB_EXITPROCESS, //PLUG_CB_EXITPROCESS
    CB_CREATETHREAD, //PLUG_CB_CREATETHREAD
    CB_EXITTHREAD, //PLUG_CB_EXITTHREAD
    CB_SYSTEMBREAKPOINT, //PLUG_CB_SYSTEMBREAKPOINT
    CB_LOADDLL, //PLUG_CB_LOADDLL
    CB_UNLOADDLL, //PLUG_CB_UNLOADDLL
    CB_OUTPUTDEBUGSTRING, //PLUG_CB_OUTPUTDEBUGSTRING
    CB_EXCEPTION, //PLUG_CB_EXCEPTION
    CB_BREAKPOINT, //PLUG_CB_BREAKPOINT
    CB_PAUSEDEBUG, //PLUG_CB_PAUSEDEBUG
    CB_RESUMEDEBUG, //PLUG_CB_RESUMEDEBUG
    CB_STEPPED, //PLUG_CB_STEPPED
    CB_ATTACH, //PLUG_CB_ATTACHED (before attaching, after CB_INITDEBUG)
    CB_DETACH, //PLUG_CB_DETACH (before detaching, before CB_STOPDEBUG)
    CB_DEBUGEVENT, //PLUG_CB_DEBUGEVENT (called on any debug event)
    CB_MENUENTRY, //PLUG_CB_MENUENTRY
    CB_WINEVENT, //PLUG_CB_WINEVENT
    CB_WINEVENTGLOBAL, //PLUG_CB_WINEVENTGLOBAL
    CB_LOADDB, //PLUG_CB_LOADSAVEDB
    CB_SAVEDB, //PLUG_CB_LOADSAVEDB
    CB_FILTERSYMBOL, //PLUG_CB_FILTERSYMBOL
    CB_TRACEEXECUTE, //PLUG_CB_TRACEEXECUTE
    CB_SELCHANGED, //PLUG_CB_SELCHANGED
    CB_ANALYZE, //PLUG_CB_ANALYZE
    CB_ADDRINFO, //PLUG_CB_ADDRINFO
    CB_VALFROMSTRING, //PLUG_CB_VALFROMSTRING
    CB_VALTOSTRING, //PLUG_CB_VALTOSTRING
    CB_LAST
} CBTYPE;

typedef enum {
    FORMAT_ERROR, //generic failure (no message)
    FORMAT_SUCCESS, //success
    FORMAT_ERROR_MESSAGE, //formatting failed but an error was put in the buffer (there are always at least 511 characters available).
    FORMAT_BUFFER_TOO_SMALL //buffer too small (x64dbg will retry until the buffer is big enough)
} FORMATRESULT;

//typedefs
typedef void (*CBPLUGIN)(CBTYPE cbType, void *callbackInfo);

//exports
#ifdef __cplusplus
extern "C" {
#endif

PLUG_IMP void _plugin_registercallback(int pluginHandle, CBTYPE cbType, CBPLUGIN cbPlugin);
PLUG_IMP bool _plugin_unregistercallback(int pluginHandle, CBTYPE cbType);
PLUG_IMP bool _plugin_menuaddentry(int hMenu, int hEntry, const char *title);
PLUG_IMP bool _plugin_menuaddseparator(int hMenu);
PLUG_IMP bool _plugin_menuclear(int hMenu);

#ifdef __cplusplus
}
#endif

#pragma pack(pop)
//----------------- From original plugin PDK (end) -----------------//
//----------------- Highlightfish plugin core (begin) -----//
#include "ColorSchemeName.h"
#include "GrayAngelfishScheme.h"
#include "WhiteAngelfishScheme.h"
#include "CypherScheme.h"
#include "DarkScheme.h"

#define PLUGIN_NAME "Highlightfish"
#define PLUGIN_VERSION 140

#define CYPHER_SCHEME_MENU 0
#define DARK_SCHEME_MENU 1
#define GRAY_ANGELFISH_SCHEME_MENU 2
#define WHITE_ANGELFISH_SCHEME_MENU 3
#define RESET_SCHEME_MENU 4
#define ABOUT_MENU 5

#ifdef _WIN64
#define INI_FILENAME ".\\x64dbg.ini"
#define DBG_FILENAME "x64dbg.exe"
#pragma comment(lib, "x64dbg.lib")
#else
#define INI_FILENAME ".\\x32dbg.ini"
#define DBG_FILENAME "x32dbg.exe"
#pragma comment(lib, "x32dbg.lib")
#endif

#define INI_SECTION_NAME "Colors"
#define COLOR_SCHEME_NAME_COUNT 199

int pluginHandle;
HWND hwndDlg;
int hMenu;

static void cbInitDebug(CBTYPE cbType, void *callbackInfo) {
    PLUG_CB_INITDEBUG *info = (PLUG_CB_INITDEBUG *)callbackInfo;
}

bool ConfirmationMsg() {
    if (MessageBoxA(hwndDlg,
                    "Set new Scheme and Restart the Debugger?\n\n"
                    "[YES] to Restart or [NO] to Cancel.\n",
                    "CONFIRMATION",
                    MB_YESNO | MB_ICONQUESTION) == IDYES)
        return true;
    else
        return false;
}


void PluginStop() {
    _plugin_unregistercallback(pluginHandle, CB_INITDEBUG);
    _plugin_unregistercallback(pluginHandle, CB_MENUENTRY);
    _plugin_menuclear(hMenu);
}

static void ExitDbg() {
    WinExec(DBG_FILENAME, SW_SHOWNORMAL);
    PluginStop();
    ExitProcess(0);
}

static void cbMenuEntry(CBTYPE cbType, void *callbackInfo) {
    PLUG_CB_MENUENTRY *info = (PLUG_CB_MENUENTRY *)callbackInfo;
    switch(info->hEntry) {
    case CYPHER_SCHEME_MENU: {
        if (ConfirmationMsg() == true) {
            for(unsigned long i = 0; i < COLOR_SCHEME_NAME_COUNT; ++i) {
                WritePrivateProfileString(INI_SECTION_NAME,
                                          ColorSchemeName[i],
                                          CypherSchemeValue[i],
                                          INI_FILENAME);
            }
            ExitDbg();
        }
        break;
    }
    case DARK_SCHEME_MENU: {
        if (ConfirmationMsg() == true) {
            for(unsigned long i = 0; i < COLOR_SCHEME_NAME_COUNT; ++i) {
                WritePrivateProfileString(INI_SECTION_NAME,
                                          ColorSchemeName[i],
                                          DarkSchemeValue[i],
                                          INI_FILENAME);
            }
            ExitDbg();
        }
        break;
    }
    case GRAY_ANGELFISH_SCHEME_MENU:  {
        if (ConfirmationMsg() == true) {
            for(unsigned long i = 0; i < COLOR_SCHEME_NAME_COUNT; ++i) {
                WritePrivateProfileString(INI_SECTION_NAME,
                                          ColorSchemeName[i],
                                          GrayAngelfishSchemeValue[i],
                                          INI_FILENAME);
            }
            ExitDbg();
        }
        break;
    }

    case WHITE_ANGELFISH_SCHEME_MENU:  {
        if (ConfirmationMsg() == true) {
            for(unsigned long i = 0; i < COLOR_SCHEME_NAME_COUNT; ++i) {
                WritePrivateProfileString(INI_SECTION_NAME,
                                          ColorSchemeName[i],
                                          WhiteAngelfishSchemeValue[i],
                                          INI_FILENAME);
            }
            ExitDbg();
        }
        break;
    }

    case RESET_SCHEME_MENU:  {
        if (ConfirmationMsg() == true) {
            WritePrivateProfileString(INI_SECTION_NAME,
                                      NULL,
                                      NULL,
                                      INI_FILENAME);

            ExitDbg();
        }
        break;
    }

    case ABOUT_MENU: {
        MessageBoxA(hwndDlg,
                    "Highlightfish v1.40 Build Date 29/08/2017 \n"
                    "Released by Insid3Code Team\n\n"
                    "C)2014-2017 I3CT",
                    "About",
                    MB_ICONINFORMATION);
        break;
    }
    break;
    }
}

void PluginInit(PLUG_INITSTRUCT *initStruct) {
    _plugin_registercallback(pluginHandle, CB_INITDEBUG, cbInitDebug);
    _plugin_registercallback(pluginHandle, CB_MENUENTRY, cbMenuEntry);
}

void PluginSetup() {
    _plugin_menuaddentry(hMenu, CYPHER_SCHEME_MENU, "&Cypher Scheme...");
    _plugin_menuaddentry(hMenu, DARK_SCHEME_MENU, "&Dark Scheme...");
    _plugin_menuaddseparator(hMenu);
    _plugin_menuaddentry(hMenu, GRAY_ANGELFISH_SCHEME_MENU, "&GrayAngelFish Scheme...");
    _plugin_menuaddentry(hMenu, WHITE_ANGELFISH_SCHEME_MENU, "&WhiteAngelFish Scheme...");
    _plugin_menuaddseparator(hMenu);
    _plugin_menuaddentry(hMenu, RESET_SCHEME_MENU, "&Reset Default Scheme...");
    _plugin_menuaddseparator(hMenu);
    _plugin_menuaddentry(hMenu, ABOUT_MENU, "&About...");
}

extern "C" {
    PLUG_EXP bool pluginit(PLUG_INITSTRUCT *initStruct) {
        initStruct->pluginVersion = PLUGIN_VERSION;
        initStruct->sdkVersion = PLUG_SDKVERSION;
        strcpy_s(initStruct->pluginName, PLUGIN_NAME);
        pluginHandle = initStruct->pluginHandle;
        PluginInit(initStruct);
        return true;
    }

    PLUG_EXP bool plugstop() {
        PluginStop();
        return true;
    }

    PLUG_EXP void plugsetup(PLUG_SETUPSTRUCT *setupStruct) {
        hwndDlg = setupStruct->hwndDlg;
        hMenu = setupStruct->hMenu;
        PluginSetup();
    }
}
//----------------- Highlightfish plugin core (end) -----//
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {

    switch(fdwReason) {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}