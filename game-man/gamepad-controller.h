#pragma once
#include <unordered_map>

class GamepadController
{
public:
    enum class OutputState{None, P14, P15};
    enum class Button
    {
        // P15
        Right = 0x1,
        Left = 0x2,
        Up = 0x4,
        Down = 0x8,
        // P14
        A = 0x10,
        B = 0x20,
        Select = 0x40,
        Start = 0x80
    };
    GamepadController();
    void SetOutputState(uint8_t mode);
    uint8_t GetOutput();
    void SetButtonValue(Button b, bool val);
private:
    OutputState output_state;
    std::unordered_map<Button, bool> pressed_buttons;
    uint8_t HiBit();
};