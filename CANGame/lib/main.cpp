#include <Arduino.h>
#include <CAN.h>
#include <virtualTimer.h>

// CAN Setup
const uint32_t CONNECTION_REQUEST_MESSAGE_ID = 0x000;
const uint32_t CONNECTION_RESPONSE_MESSAGE_ID = 0x100;
const uint32_t PLAYER_INPUT_MESSAGE_ID = 0x200;

CAN g_can ();
VirtualTimer g_timergroup;
MakeSignedCANSignal(int8_t, 0 8 1.0 0.0) g_dviceIDSignal;
CANTXMessage<1> g_connectionRequestMessage {
    g_can,
    CONNECTION_REQUEST_MESSAGE_ID
    1, 
    100, 
    g_deviceIDSignal
};

void handleConnectionResponse();

MakeUnsignedCANSignal(uint64_t, 0, 64, 1.0, 0.0) g_responseDataSignal;
CANRXMessage<1> g_responseMessage {
    g_can,
    CONNECTION_RESPONSE_MESSAGE_ID,
    handleConnectionResponse,
    g_responseDataSignal

};

MakeSignedCANSignal(int8_t, 0, 8, 1.0, 0.0) g_playerIDSignal;
MakeSignedCANSignal(float, 8, 16, 10, 0.0) g_horisontalAxisSignal;
MakeSignedCANSignal(float, 24, 16, 10, 0.0) g_vertialAxisSignal;
MakeSignedCANSignal(float, 40, 16, 10, 0.0) g_rotationAxisSignal;
MakeSignedCANSignal(uint8_t, 56, 8, 1.0, 0.0) g_bitmaskSignal;

CANTX

//  End CAN_Setup
void setup(){

}
void update(){

}