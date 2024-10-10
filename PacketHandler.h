#ifndef PacketHandler
#define PacketHandler

#include <iostream>
#include <windows.h>
#include <fwpmu.h>
#include <fwpmtypes.h>
#include <winsock2.h>

#pragma comment(lib, "fwpuclnt.lib")
#pragma comment(lib, "fwcommon.lib")
#pragma comment(lib, "fwpmu.lib")

void initWFP(FWPM_SESSION** session) {
    FWPM_SESSION0 sessionTemplate = {};
    sessionTemplate.displayData.name = (PWSTR)L"Sample Session";
    sessionTemplate.flags = FWPM_SESSION_FLAG_DYNAMIC;
    DWORD result = FwpmEngineOpen0(
        nullptr, 
        RPC_C_AUTHN_WINNT,
        nullptr, 
        &sessionTemplate, (HANDLE*)session
    );
    if (result != ERROR_SUCCESS) {
        std::cerr << "Failed to open WFP session. Error code: " << result << std::endl;
        exit(1);
    }
}

void addFilter(FWPM_SESSION* session, UINT16 port) {
    FWPM_FILTER0 filter = {};
    filter.displayData.name = (PWSTR)L"Sample Filter";
    filter.layerKey = FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4;
    filter.action.type = FWP_ACTION_BLOCK;

    FWPM_FILTER_CONDITION0 condition = {};
    condition.fieldKey = FWPM_CONDITION_IP_REMOTE_PORT;
    condition.matchType = FWP_MATCH_EQUAL;
    condition.conditionValue.type = FWP_UINT16;
    condition.conditionValue.uint16 = port;

    filter.filterCondition = &condition;
    filter.numFilterConditions = 1;

    UINT64 filterId;
    DWORD result = FwpmFilterAdd0(session, &filter, nullptr, &filterId);
    if (result != ERROR_SUCCESS) {
        std::cerr << "Failed to add filter. Error code: " << result << std::endl;
        exit(1);
    }
}


void decryptAndInspectPacket(const char* packet) {
    const char* targetString = "example";
    if (strstr(packet, targetString)) {
        std::cout << "Packet contains the target string: " << targetString << std::endl;
    }
}

int ExampleUse() {

    FWPM_SESSION* session;

    initWFP(&session);

    // Add filter for a specific port (example: port 80)
    addFilter(session, 80);

    // Example packet for inspection
    const char* packet = "This is an example packet containing the string example.";
    decryptAndInspectPacket(packet);

    // Clean up
    FwpmEngineClose0(session);

    return 0;
}

#endif


