#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0A00  
#include <locale.h>  // Windows 10+
#include <windows.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <sddl.h>             // ConvertStringSidToSid (admin check)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "cfgmgr32.lib")
#pragma comment(lib, "advapi32.lib")
// MinGW : gcc main.c -o deviceManager.exe -lsetupapi -lcfgmgr32 -ladvapi32

/* ===================== Types ===================== */

typedef enum { ST_ACTIVE, ST_DISABLED, ST_ERROR, ST_UNKNOWN } DevStatus;
typedef enum { TYPE_KEYBOARD, TYPE_MOUSE, TYPE_HID, TYPE_USB,
               TYPE_MONITOR, TYPE_BLUETOOTH, TYPE_PORT, TYPE_OTHER } DevType;

typedef struct {
    wchar_t   name[256];       // FriendlyName ou DeviceDesc
    wchar_t   className[64];   // ex: "Keyboard", "Mouse", "HIDClass"
    wchar_t   instanceId[256]; // identifiant unique (clé)
    DevStatus status;
    DevType   type;
    int       disableable;     // DN_DISABLEABLE
} Peripheral;

static Peripheral *g_devices = NULL;
static size_t g_count = 0;

/* ===================== Helpers ===================== */

static BOOL GetDevString(HDEVINFO hDev, PSP_DEVINFO_DATA did,
                         DWORD prop, wchar_t *buf, DWORD bufBytes)
{
    return SetupDiGetDeviceRegistryPropertyW(hDev, did, prop,
                                             NULL, (PBYTE)buf, bufBytes, NULL);
}

static DevStatus GetStatus(DEVINST devInst)
{
    ULONG status = 0, problem = 0;
    if (CM_Get_DevNode_Status(&status, &problem, devInst, 0) != CR_SUCCESS)
        return ST_UNKNOWN;
    if (problem == CM_PROB_DISABLED) return ST_DISABLED;  // 22
    if (problem != 0)                  return ST_ERROR;
    if (status & DN_STARTED)           return ST_ACTIVE;
    return ST_UNKNOWN;
}

static DevType Classify(const wchar_t *cls)
{
    if (_wcsicmp(cls, L"Keyboard")    == 0) return TYPE_KEYBOARD;
    if (_wcsicmp(cls, L"Mouse")       == 0) return TYPE_MOUSE;
    if (_wcsicmp(cls, L"HIDClass")    == 0) return TYPE_HID;
    if (_wcsicmp(cls, L"USB")         == 0) return TYPE_USB;
    if (_wcsicmp(cls, L"Monitor")     == 0) return TYPE_MONITOR;
    if (_wcsicmp(cls, L"Bluetooth")   == 0) return TYPE_BLUETOOTH;
    if (_wcsicmp(cls, L"Ports")       == 0) return TYPE_PORT;
    return TYPE_OTHER;
}

static const wchar_t *StatusStr(DevStatus s)
{
    switch (s) {
        case ST_ACTIVE:   return L"ACTIF";
        case ST_DISABLED: return L"DESACTIVE";
        case ST_ERROR:    return L"ERREUR";
        default:          return L"INCONNU";
    }
}

static const wchar_t *TypeStr(DevType t)
{
    switch (t) {
        case TYPE_KEYBOARD:   return L"Clavier";
        case TYPE_MOUSE:      return L"Souris";
        case TYPE_HID:        return L"HID (USB)";
        case TYPE_USB:        return L"USB";
        case TYPE_MONITOR:    return L"Moniteur";
        case TYPE_BLUETOOTH:  return L"Bluetooth";
        case TYPE_PORT:       return L"Port COM/LPT";
        default:              return L"Autre";
    }
}

/* ===================== Enumeration ===================== */

static size_t EnumerateDevices(Peripheral **out)
{
    HDEVINFO hDev = SetupDiGetClassDevsW(NULL, NULL, NULL,
                                         DIGCF_ALLCLASSES | DIGCF_PRESENT);
    if (hDev == INVALID_HANDLE_VALUE) return 0;

    // compter d'abord
    SP_DEVINFO_DATA did; did.cbSize = sizeof(did);
    size_t n = 0;
    while (SetupDiEnumDeviceInfo(hDev, (DWORD)n, &did)) n++;

    Peripheral *arr = calloc(n ? n : 1, sizeof(Peripheral));
    size_t k = 0;
    for (DWORD i = 0; SetupDiEnumDeviceInfo(hDev, i, &did); i++) {
        Peripheral *p = &arr[k];

        if (!GetDevString(hDev, &did, SPDRP_FRIENDLYNAME, p->name, sizeof(p->name)))
            GetDevString(hDev, &did, SPDRP_DEVICEDESC, p->name, sizeof(p->name));

        GetDevString(hDev, &did, SPDRP_CLASS, p->className, sizeof(p->className));
        SetupDiGetDeviceInstanceIdW(hDev, &did, p->instanceId, 256, NULL);

        ULONG status = 0, problem = 0;
        CM_Get_DevNode_Status(&status, &problem, did.DevInst, 0);
        p->disableable = (status & DN_DISABLEABLE) != 0;

        p->status = GetStatus(did.DevInst);
        p->type   = Classify(p->className);
        k++;
    }
    SetupDiDestroyDeviceInfoList(hDev);

    free(*out);
    *out = arr;
    return k;
}

/* ===================== Enable / Disable ===================== */

static BOOL SetDeviceState(const wchar_t *instanceId, BOOL enable)
{
    // retouver le device dans une liste fraiche (par instanceId, pas par index:
    // l'ordre peut changer apres une action)
    HDEVINFO hDev = SetupDiGetClassDevsW(NULL, NULL, NULL,
                                         DIGCF_ALLCLASSES | DIGCF_PRESENT);
    if (hDev == INVALID_HANDLE_VALUE) return FALSE;

    BOOL ok = FALSE;
    SP_DEVINFO_DATA did; did.cbSize = sizeof(did);
    wchar_t id[256];

    for (DWORD i = 0; SetupDiEnumDeviceInfo(hDev, i, &did); i++) {
        if (!SetupDiGetDeviceInstanceIdW(hDev, &did, id, 256, NULL)) continue;
        if (_wcsicmp(id, instanceId) != 0) continue;

        // configurer la demande de changement d'etat
        SP_PROPCHANGE_PARAMS pcp;
        ZeroMemory(&pcp, sizeof(pcp));
        pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
        pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
        pcp.StateChange = enable ? DICS_ENABLE : DICS_DISABLE;
        pcp.Scope       = enable ? DICS_FLAG_GLOBAL : DICS_FLAG_CONFIGSPECIFIC;
        pcp.HwProfile   = 0;

        if (!SetupDiSetClassInstallParamsW(hDev, &did,
                                           &pcp.ClassInstallHeader, sizeof(pcp)))
            break;

        ok = SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDev, &did);

        // pour ENABLE: fall-back CONFIGSPECIFIC (methode de devcon)
        if (!ok && enable) {
            pcp.Scope = DICS_FLAG_CONFIGSPECIFIC;
            if (SetupDiSetClassInstallParamsW(hDev, &did,
                                              &pcp.ClassInstallHeader, sizeof(pcp)))
                ok = SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDev, &did);
        }

        // redemarrage necessaire ?
        if (ok) {
            SP_DEVINSTALL_PARAMS_W ip; ip.cbSize = sizeof(ip);
            if (SetupDiGetDeviceInstallParamsW(hDev, &did, &ip) &&
                (ip.Flags & DI_NEEDREBOOT))
                wprintf(L"[!] Un redemarrage est necessaire pour cet appareil.\n");
        } else {
            wprintf(L"[X] Echec (%lu). Executez en Administrateur.\n",
                    GetLastError());
        }
        break;
    }

    SetupDiDestroyDeviceInfoList(hDev);
    return ok;
}

/* ===================== Affichage / Menu ===================== */

static void PrintDevices(int typeFilter)   // -1 = tout
{
    wprintf(L"\n%-4s %-18s %-10s %-3s %s\n",
            L"N", L"TYPE", L"STATUT", L"DIS", L"NOM");
    wprintf(L"--------------------------------------------------------------------------\n");
    for (size_t i = 0; i < g_count; i++) {
        Peripheral *p = &g_devices[i];
        if (typeFilter >= 0 && p->type != (DevType)typeFilter) continue;
        wprintf(L"%-4zu %-18s %-10s %-3s %s\n", i, TypeStr(p->type),
                StatusStr(p->status), p->disableable ? L"oui" : L"non", p->name);
    }
}

static void Refresh(void)
{
    g_count = EnumerateDevices(&g_devices);
    wprintf(L"[OK] %zu peripherique(s) detecte(s).\n", g_count);
}

static BOOL IsAdmin(void)
{
    BOOL admin = FALSE;
    PSID sid = NULL;
    if (ConvertStringSidToSidW(L"S-1-5-32-544", &sid)) {
        CheckTokenMembership(NULL, sid, &admin);
        LocalFree(sid);
    }
    return admin;
}

int main(void)
{
    _wsetlocale(LC_ALL, L"");   // pour afficher les accents

    if (!IsAdmin())
        wprintf(L"[!] Pas execute en Administrateur: activer/desactiver echouera.\n");

    Refresh();

    for (;;) {
        wprintf(L"\n===== DETECTEUR DE PERIPHERIQUES =====\n"
                L"1. Lister tout\n"
                L"2. Lister claviers + souris + HID\n"
                L"3. Desactiver un peripherique\n"
                L"4. Activer un peripherique\n"
                L"5. Rafraichir la liste\n"
                L"0. Quitter\n> ");

        int c; 
        if (scanf("%d", &c) != 1) { while (getchar() != '\n'); continue; }

        if (c == 0) break;

        if (c == 1) PrintDevices(-1);

        else if (c == 2)
            for (int t = TYPE_KEYBOARD; t <= TYPE_HID; t++) PrintDevices(t);

        else if (c == 3 || c == 4) {
            PrintDevices(-1);
            wprintf(L"Numero du peripherique: ");
            size_t idx;
            if (scanf("%zu", &idx) != 1 || idx >= g_count) {
                wprintf(L"Numero invalide.\n"); continue;
            }
            Peripheral *p = &g_devices[idx];
            if (c == 3 && !p->disableable) {
                wprintf(L"[X] Ce peripherique ne peut pas etre desactive.\n");
                continue;
            }
            wprintf(L"ATTENTION: desactiver clavier/souris peut couper votre saisie. Continuer ? (o/n): ");
            char r; 
            while (scanf(" %c", &r) != 1);
            if (r != 'o' && r != 'O') continue;

            if (SetDeviceState(p->instanceId, c == 4))
                wprintf(L"[OK] Operation reussie.\n");
            Refresh();
        }
        else if (c == 5) Refresh();
    }

    free(g_devices);
    return 0;
}