#include <Arduino.h>
#include <CAN.h>
#include <virtualTimer.h>
#include <Encoder.h>

// * CAN_SETUP
const uint32_t CONNECTION_REQUEST_MESSAGE_ID = 0x000;
const uint32_t CONNECTION_RESPONSE_MESSAGE_ID = 0x100;
const uint32_t PLAYER_INPUT_MESSAGE_ID = 0x200;


CAN g_can {};
VirtualTimerGroup g_timerGroup;

MakeSignedCANSignal(int8_t, 0, 8, 1.0, 0.0) g_deviceIDSignal;
CANTXMessage<1> g_connectionRequestMessage {
    g_can,
    CONNECTION_REQUEST_MESSAGE_ID,
    1, // in bytes
    100, // transmit every 100ms
    g_deviceIDSignal
};

// Forward Declaration
void handleConnectionResponse();

MakeUnsignedCANSignal(uint64_t, 0, 64, 1.0, 0.0) g_responseDataSignal;
CANRXMessage<1> g_responseMessage {
    g_can, 
    CONNECTION_RESPONSE_MESSAGE_ID,
    handleConnectionResponse,
    g_responseDataSignal
};

// Input message
MakeSignedCANSignal(int8_t, 0, 8, 1.0, 0.0) g_playerIDSignal;
MakeSignedCANSignal(float, 8, 16, 10, 0.0) g_horizontalAxisSignal;
MakeSignedCANSignal(float, 24, 16, 10, 0) g_verticalAxisSignal;
MakeSignedCANSignal(float, 40, 16, 10, 0) g_rotationAxisSignal;
MakeUnsignedCANSignal(uint8_t, 56, 8, 1, 0) g_bitmaskSignal;

CANTXMessage<5> g_playerInputMessage {
    g_can,
    PLAYER_INPUT_MESSAGE_ID,
    1, 100, g_timerGroup,
    g_playerIDSignal,
    g_verticalAxisSignal,
    g_horizontalAxisSignal,
    g_rotationAxisSignal,
    g_bitmaskSignal
};

// * END CAN_SETUP


// * State machine setup
enum ControllerState
{
    DISCONNECTED, AWAITING_CONNECTION, CONNECTED
};

ControllerState g_state = DISCONNECTED;

// * END Statemachine setup

void handleConnectionResponse()
{
    uint64_t responseData = g_responseDataSignal;
    std::array<int8_t, 8> byteArray;

    for (int i = 0; i < 8; i++)
    {
        byteArray[i] = (int8_t)(responseData >> (i * 8));
    }

    int8_t deviceID = byteArray[0];
    int8_t playerNmber = byteArray[1];

    // is this message for us?
    if (g_deviceIDSignal == deviceID)
    {
        // we gonna do something about it
        g_playerIDSignal = playerNmber;
        g_state = ControllerState::CONNECTED;
    }
}

void handleState()
{
    switch (g_state)
    {
        case DISCONNECTED:
            Serial.println("STATE: DISCONNECTED");
            g_state = AWAITING_CONNECTION;
            break;
        case AWAITING_CONNECTION:
            Serial.println("STATE: AWAITING CONNECTION");
            break;
        case CONNECTED:
            Serial.println("STATE: CONNECTED");
            break;
    }

    g_can.Tick();
}


// CAN SIGNAL SETUP, For reference
// MakeSignedCANSignal(int8_t, 0, 8, 1.0, 0.0) g_playerIDSignal;
// MakeSignedCANSignal(float, 8, 16, 10, 0.0) g_horizontalAxisSignal;
// MakeSignedCANSignal(float, 24, 16, 10, 0) g_verticalAxisSignal;
// MakeSignedCANSignal(float, 40, 16, 10, 0) g_rotationAxisSignal;
// MakeUnsignedCANSignal(uint8_t, 56, 8, 1, 0) g_bitmaskSignal;

bool g_shouldShoot;
bool g_shouldMine;
bool g_shouldShield;

#define SET_BIT(var, n, value) ((value) ? ((var) |= (1 << (n))) : ((var) &= ~(1 << (n))))

void readSensors()
{
    // you guys would set the signals here...
    // g_horizontalAxisSignal = readJoystickHoriztonal();
    uint8_t bitMask = g_bitmaskSignal;
    SET_BIT(bitMask, 0, g_shouldShoot);
    SET_BIT(bitMask, 1, g_shouldMine);
    SET_BIT(bitMask, 2, g_shouldShield);
    g_bitmaskSignal = bitMask;

    // Read and update encoder-based rotation
    int32_t newPosition = g_encoder.read();
    if (newPosition != g_lastEncoderPosition) {
        g_rotationAxisValue += (newPosition - g_lastEncoderPosition) * 0.1f; // Scale as needed
        g_lastEncoderPosition = newPosition;

        // Update CAN signal
        g_rotationAxisSignal = g_rotationAxisValue;
    }

    // Handle button press
    if (g_buttonPressed) {
        Serial.println("Encoder button pressed!");
        g_buttonPressed = false; // Reset flag
    }
}


void setup()
{
    Serial.begin(9600);
    g_can.Initialize(ICAN::BaudRate::kBaud1M);
    g_timerGroup.AddTimer(10, handleState);
    g_timerGroup.AddTimer(10, readSensors);

    g_playerIDSignal = -1;
    g_deviceIDSignal = (int8_t)random(0, 255);

    // Initialize encoder button with interrupt
    pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ENCODER_BUTTON_PIN), handleEncoderButton, FALLING);

    // Initialize encoder position
    g_lastEncoderPosition = g_encoder.read();
}

void loop()
{
    g_timerGroup.Tick(millis());
}

const int ENCODER_PIN_A = 2; // Pin for encoder A
const int ENCODER_PIN_B = 3; // Pin for encoder B
const int ENCODER_BUTTON_PIN = 4; // Optional: Pin for encoder button

Encoder g_encoder(ENCODER_PIN_A, ENCODER_PIN_B);
volatile int32_t g_lastEncoderPosition = 0;
float g_rotationAxisValue = 0.0; // To update g_rotationAxisSignal


// Interupt for the button
volatile bool g_buttonPressed = false;

void IRAM_ATTR handleEncoderButton() {
    g_buttonPressed = true;
}
