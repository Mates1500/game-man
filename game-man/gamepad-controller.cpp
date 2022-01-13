#include "gamepad-controller.h"

#include <stdexcept>

GamepadController::GamepadController():output_state(OutputState::None)
{
    this->pressed_buttons = {
        {Button::Right, false},
        {Button::Left, false},
        {Button::Up, false},
        {Button::Down, false},
        {Button::A, false},
        {Button::B, false},
        {Button::Select, false},
        {Button::Start, false}
    };
}

void GamepadController::SetOutputState(uint8_t mode)
{
    if ((mode & 0x20) == 0x20)
        this->output_state = OutputState::P14;

    if ((mode & 0x10) == 0x10)
        this->output_state = OutputState::P15;

    if ((mode & 0x30) == 0x30)
    {
        this->output_state = OutputState::None;
    }
}

uint8_t GamepadController::GetOutput()
{
    if (this->output_state == OutputState::P15)
        return static_cast<uint8_t>(~(static_cast<uint8_t>(Button::Right) | static_cast<uint8_t>(Button::Left) |
            static_cast<uint8_t>(Button::Up) | static_cast<uint8_t>(Button::Down))
            & 0x0F) | HiBit();

    if (this->output_state == OutputState::P14)
        return static_cast<uint8_t>(~((static_cast<uint8_t>(Button::A) | static_cast<uint8_t>(Button::B) |
                static_cast<uint8_t>(Button::Select) | static_cast<uint8_t>(Button::Start)) >> 4) // shift right 4, as we only want to output low 4 bits
            & 0x0F) | HiBit();

    return 0x0F | HiBit();
}

void GamepadController::SetButtonValue(const Button b, const bool val)
{
    this->pressed_buttons[b] = val;
}

uint8_t GamepadController::HiBit()
{
    switch(this->output_state)
    {
    case OutputState::None: 
        return 0xF0;
    case OutputState::P15:
        return 0b11010000;
    case OutputState::P14:
        return 0b11100000;
    }

    throw std::runtime_error("Unexpected OutputState");
}


