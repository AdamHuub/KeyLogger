
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0A00

#include <windows.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

typedef enum {
    STATUS_UNKNOWN,
    STATUS_STARTED,
    STATUS_NOT_STARTED,
    STATUS_HAS_PROBLEM
} PeripheralStatus;

typedef enum {
    TYPE_UNKNOWN,
    TYPE_USB,
    TYPE_NETWORK,
    TYPE_AUDIO,
    TYPE_DISPLAY,
    TYPE_BATTERY,
    TYPE_BLUETOOTH,
    TYPE_CAMERA,
    TYPE_LECTEURDVD,
    TYPE_STORAGE,
    TYPE_GRAPHICS_CARDS,
    TYPE_GPS,
    TYPE_IMAGE,
    TYPE_MEMORY,
    TYPE_MONITOR,
    TYPE_PRINTER,
    TYPE_MOUSE,
    TYPE_KEYBOARD,
    TYPE_HIDClass,
    TYPE_OTHER,
} PeripheralType;

typedef struct {
    wchar_t name[256];
    wchar_t className[64];
    wchar_t manufacturer[256];
    wchar_t instanceId[256];
    wchar_t enumerator[64];
    PeripheralStatus status;
    PeripheralType type;
    int disableable;
} Peripheral;


PeripheralType GetPeripheralType(const wchar_t *className)
{
    if (wcscmp(className, L"USB") == 0)
        return TYPE_USB;

    if (wcscmp(className, L"Net") == 0)
        return TYPE_NETWORK;

    if (wcscmp(className, L"MEDIA") == 0)
        return TYPE_AUDIO;

    if (wcscmp(className, L"Display") == 0)
        return TYPE_DISPLAY;

    if (wcscmp(className, L"Battery") == 0)
        return TYPE_BATTERY;
        
     if (wcscmp(className, L"Bluetooth") == 0)
        return TYPE_BLUETOOTH;

    if (wcscmp(className, L"Camera") == 0)
        return TYPE_CAMERA;
    
    if (wcscmp(className, L"CDROM") == 0)
        return TYPE_LECTEURDVD;

    if (wcscmp(className, L"DiskDrive") == 0)
        return TYPE_STORAGE;

    if (wcscmp(className, L"Display") == 0)
        return TYPE_GRAPHICS_CARDS;

    if (wcscmp(className, L"GPS") == 0)
        return TYPE_GPS;

    if (wcscmp(className, L"Image") == 0)
        return TYPE_IMAGE;

    if (wcscmp(className, L"Memory") == 0)
        return TYPE_MEMORY;

    if (wcscmp(className, L"Monitor") == 0)
        return TYPE_MONITOR;

    if (wcscmp(className, L"Printer") == 0)
        return TYPE_PRINTER;
    
    if(wcscmp(className, L"Mouse") == 0)
        return TYPE_MOUSE;
    
    if(wcscmp(className, L"Keyboard") == 0)
        return TYPE_KEYBOARD;

    if(wcscmp(className, L"HIDClass") == 0)
        return TYPE_HIDClass;
        
    if (className[0] != L'\0')
        return TYPE_OTHER;

    return TYPE_UNKNOWN;
}



int EnumerateDevices(Peripheral *devices, int maxDevices)
{
    HDEVINFO hDev = SetupDiGetClassDevsW(
        NULL, NULL, NULL,
        DIGCF_ALLCLASSES | DIGCF_PRESENT
    );

    if (hDev == INVALID_HANDLE_VALUE) {
        printf("SetupDiGetClassDevsW error: %lu\n",
               (unsigned long)GetLastError());
        return -1;
    }

    int count = 0;

    for (DWORD i = 0; count < maxDevices; i++) {
        SP_DEVINFO_DATA did = {0};
        did.cbSize = sizeof(did);

        if (!SetupDiEnumDeviceInfo(hDev, i, &did))
            break;

        Peripheral *p = &devices[count];
        memset(p, 0, sizeof(*p));

        // 1. Device description
        SetupDiGetDeviceRegistryPropertyW(
            hDev, &did, SPDRP_DEVICEDESC,
            NULL, (PBYTE)p->name,
            sizeof(p->name), NULL
        );

        // 2. Device class
        SetupDiGetDeviceRegistryPropertyW(
            hDev, &did, SPDRP_CLASS,
            NULL, (PBYTE)p->className,
            sizeof(p->className), NULL
        );

        // 3. Manufacturer
        SetupDiGetDeviceRegistryPropertyW(
            hDev, &did, SPDRP_MFG,
            NULL, (PBYTE)p->manufacturer,
            sizeof(p->manufacturer), NULL
        );

        // 4. Enumerator name (USB, PCI, etc.)
        SetupDiGetDeviceRegistryPropertyW(
            hDev, &did, SPDRP_ENUMERATOR_NAME,
            NULL, (PBYTE)p->enumerator,
            sizeof(p->enumerator), NULL
        );

        // 5. Device instance ID
        SetupDiGetDeviceInstanceIdW(
            hDev, &did,
            p->instanceId,
            (DWORD)(sizeof(p->instanceId) /
                    sizeof(p->instanceId[0])),
            NULL
        );

        // 6. Device status
        ULONG devStatus = 0;
        ULONG problem = 0;

        CONFIGRET cr = CM_Get_DevNode_Status(
            &devStatus,
            &problem,
            did.DevInst,
            0
        );

        if (cr != CR_SUCCESS) {
            p->status = STATUS_UNKNOWN;
        }
        else if (devStatus & DN_HAS_PROBLEM) {
            p->status = STATUS_HAS_PROBLEM;
        }
        else if (devStatus & DN_STARTED) {
            p->status = STATUS_STARTED;
        }
        else {
            p->status = STATUS_NOT_STARTED;
        }

        // 7. Classify device
        p->type = GetPeripheralType(p->className);

        // 8. Disable capability not checked in this version
        p->disableable = 0;

        count++;
    }

    SetupDiDestroyDeviceInfoList(hDev);
    return count;
}

int main(void)
{
    while (1) {
        wprintf(
            L"\n===== DETECTEUR DE PERIPHERIQUES =====\n"
            L"1. Lister tout\n"
            L"2. Lister claviers + souris + HID\n"
           /*\ L"3. Desactiver un peripherique\n"
            L"4. Activer un peripherique\n"*/
            L"0. Quitter\n> "
        );

        int chosedValue;

        if (scanf("%d", &chosedValue) != 1)
            break;

        switch (chosedValue) {

            case 1: {
                wprintf(
                    L"\n===== LISTE DE TOUS LES PERIPHERIQUES =====\n"
                );

                Peripheral devices[512] = {0};
                int count = EnumerateDevices(devices, 512);

                if (count < 0)
                    return 1;

                
                   
wprintf(L"\n");
wprintf(L"============================================================\n");
wprintf(L"                 PERIPHERAL DEVICE MANAGER\n");
wprintf(L"============================================================\n");
wprintf(L"Total devices detected: %d\n", count);
wprintf(L"============================================================\n");

for (int i = 0; i < count; i++) {

    wprintf(L"\n");
    wprintf(L"+----------------------------------------------------------+\n");
    wprintf(L"| DEVICE %-3d                                              |\n", i + 1);
    wprintf(L"+----------------------------------------------------------+\n");

    wprintf(L"  Name         : %ls\n",
            devices[i].name[0] ? devices[i].name : L"Unknown");

    wprintf(L"  Class        : %ls\n",
            devices[i].className[0] ? devices[i].className : L"Unknown");

    wprintf(L"  Manufacturer : %ls\n",
            devices[i].manufacturer[0] ? devices[i].manufacturer : L"Unknown");

    wprintf(L"  Enumerator   : %ls\n",
            devices[i].enumerator[0] ? devices[i].enumerator : L"Unknown");

    wprintf(L"  Instance ID  : %ls\n",
            devices[i].instanceId[0] ? devices[i].instanceId : L"Unknown");

    wprintf(L"  Status       : ");

    switch (devices[i].status) {
        case STATUS_STARTED:
            wprintf(L"Started\n");
            break;

        case STATUS_NOT_STARTED:
            wprintf(L"Not started\n");
            break;

        case STATUS_HAS_PROBLEM:
            wprintf(L"Has a problem\n");
            break;

        default:
            wprintf(L"Unknown\n");
            break;
    }

    wprintf(L"  Type         : ");

    switch (devices[i].type) {
        case TYPE_USB:
            wprintf(L"USB\n");
            break;

        case TYPE_NETWORK:
            wprintf(L"Network\n");
            break;

        case TYPE_AUDIO:
            wprintf(L"Audio\n");
            break;

        case TYPE_DISPLAY:
            wprintf(L"Display\n");
            break;

        case TYPE_STORAGE:
            wprintf(L"storage\n");
            break;

        case TYPE_LECTEURDVD:
            wprintf(L"kecteur dvd\n");
            break;

        case TYPE_CAMERA:
            wprintf(L"camera\n");
            break;

        case TYPE_BLUETOOTH:
            wprintf(L"bluetooth\n");
            break;

        case TYPE_BATTERY:
            wprintf(L"battery\n");
            break;

        case TYPE_GRAPHICS_CARDS:
            wprintf(L"graphics\n");
            break;

        case TYPE_GPS:
            wprintf(L"GPS\n");
            break;

        case TYPE_IMAGE:
            wprintf(L"image\n");
            break;

        case TYPE_MEMORY:
            wprintf(L"memory\n");
            break;

         case TYPE_MONITOR:
            wprintf(L"monitor\n");
            break;

         case TYPE_PRINTER:
            wprintf(L"printer\n");
            break;

         case TYPE_MOUSE:
            wprintf(L"mouse\n");
            break;

         case TYPE_KEYBOARD:
            wprintf(L"keyboard\n");
            break;

        case TYPE_HIDClass:
            wprintf(L"hid\n");
            break;

         case TYPE_OTHER:
            wprintf(L"Other\n");
            break;
        

        default:
            wprintf(L"Unknown\n");
            break;
    }

    wprintf(L"  Disableable  : %ls\n",
            devices[i].disableable ? L"Yes" : L"Not checked");

    wprintf(L"+----------------------------------------------------------+\n");
}

wprintf(L"\n============================================================\n");
wprintf(L"                 END OF DEVICE LIST\n");
wprintf(L"============================================================\n");

                

                wprintf(L"\nTotal des peripheriques: %d\n", count);
                break;
            }

            case 2: {

                wprintf(L"Liste des claviers, souris et HID\n");

                

                    Peripheral devices[512] = {0};
                    int count = EnumerateDevices(devices, 512);

                    if (count < 0)
                        return 1;

                    int displayed = 0;

                    for (int i = 0; i < count; i++) {

                        if (wcscmp(devices[i].className, L"Printer") == 0 ||
                            wcscmp(devices[i].className, L"Monitor") == 0 ||
                            wcscmp(devices[i].className, L"Keyboard") == 0 ||
                            wcscmp(devices[i].className, L"Mouse") == 0 ||
                            wcscmp(devices[i].className, L"HIDClass") == 0) {

                            displayed++;

                            wprintf(L"\n");
                            wprintf(L"============================================================\n");
                            wprintf(L"                 PERIPHERAL DEVICE MANAGER\n");
                            wprintf(L"============================================================\n");

                            wprintf(L"\n+----------------------------------------------------------+\n");
                            wprintf(L"| DEVICE %-3d                                              |\n", displayed);
                            wprintf(L"+----------------------------------------------------------+\n");

                            wprintf(L"  Name         : %ls\n",
                                    devices[i].name[0] ? devices[i].name : L"Unknown");

                            wprintf(L"  Class        : %ls\n",
                                    devices[i].className[0] ? devices[i].className : L"Unknown");

                            wprintf(L"  Manufacturer : %ls\n",
                                    devices[i].manufacturer[0] ? devices[i].manufacturer : L"Unknown");

                            wprintf(L"  Enumerator   : %ls\n",
                                    devices[i].enumerator[0] ? devices[i].enumerator : L"Unknown");

                            wprintf(L"  Instance ID  : %ls\n",
                                    devices[i].instanceId[0] ? devices[i].instanceId : L"Unknown");

                            wprintf(L"  Status       : ");

                            switch (devices[i].status) {
                                case STATUS_STARTED:
                                    wprintf(L"Started\n");
                                    break;

                                case STATUS_NOT_STARTED:
                                    wprintf(L"Not started\n");
                                    break;

                                case STATUS_HAS_PROBLEM:
                                    wprintf(L"Has a problem\n");
                                    break;

                                default:
                                    wprintf(L"Unknown\n");
                                    break;
                            }

                            wprintf(L"  Type         : ");

                          switch (devices[i].type) {
                                    case TYPE_USB:
                                        wprintf(L"USB\n");
                                        break;

                                    case TYPE_NETWORK:
                                        wprintf(L"Network\n");
                                        break;

                                    case TYPE_AUDIO:
                                        wprintf(L"Audio\n");
                                        break;

                                    case TYPE_DISPLAY:
                                        wprintf(L"Display\n");
                                        break;

                                    case TYPE_STORAGE:
                                        wprintf(L"storage\n");
                                        break;

                                    case TYPE_LECTEURDVD:
                                        wprintf(L"kecteur dvd\n");
                                        break;

                                    case TYPE_CAMERA:
                                        wprintf(L"camera\n");
                                        break;

                                    case TYPE_BLUETOOTH:
                                        wprintf(L"bluetooth\n");
                                        break;

                                    case TYPE_BATTERY:
                                        wprintf(L"battery\n");
                                        break;

                                    case TYPE_GRAPHICS_CARDS:
                                        wprintf(L"graphics\n");
                                        break;

                                    case TYPE_GPS:
                                        wprintf(L"GPS\n");
                                        break;

                                    case TYPE_IMAGE:
                                        wprintf(L"image\n");
                                        break;

                                    case TYPE_MEMORY:
                                        wprintf(L"memory\n");
                                        break;

                                    case TYPE_MONITOR:
                                        wprintf(L"monitor\n");
                                        break;

                                    case TYPE_PRINTER:
                                        wprintf(L"printer\n");
                                        break;

                                    case TYPE_MOUSE:
                                        wprintf(L"mouse\n");
                                        break;

                                    case TYPE_KEYBOARD:
                                        wprintf(L"keyboard\n");
                                        break;

                                    case TYPE_HIDClass:
                                        wprintf(L"hid\n");
                                        break;

                                    case TYPE_OTHER:
                                        wprintf(L"Other\n");
                                        break;
                                    

                                    default:
                                        wprintf(L"Unknown\n");
                                        break;
                            }

                            wprintf(L"  Disableable  : %ls\n",
                                    devices[i].disableable ? L"Yes" : L"Not checked");

                            wprintf(L"+----------------------------------------------------------+\n");
                        }
                    }

                    if (displayed == 0)
                        wprintf(L"No matching devices found.\n");

                    break;
                }
                /*case 3:
                wprintf(L"Desactiver un peripherique\n");
                break;

                case 4:
                wprintf(L"Activer un peripherique\n");
                break;*/
            

            case 0:
                wprintf(L"Quitter\n");
                return 0;

            default:
                wprintf(L"Choix invalide\n");
                break;
        }
    }

    return 0;
}
