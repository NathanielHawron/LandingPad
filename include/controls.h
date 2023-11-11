#pragma once

#include <cstdint>

#include <GLFW/glfw3.h>

namespace controls{
    uint8_t controls = 0;
    double mouseX = 0, mouseY = 0;
    double sensitivity = 0.001;
    double speed = 0.01;
    constexpr uint8_t RIGHT =       0b0000001;
    constexpr uint8_t LEFT =        0b0000010;
    constexpr uint8_t FORWARDS =    0b0000100;
    constexpr uint8_t BACKWARDS =   0b0001000;
    constexpr uint8_t UP =          0b0010000;
    constexpr uint8_t DOWN =        0b0100000;
    constexpr uint8_t SS =          0b1000000;

    void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods){
        if(action == GLFW_PRESS){
            switch(key){
                case GLFW_KEY_D:controls |= RIGHT;      break;
                case GLFW_KEY_A:controls |= LEFT;       break;
                case GLFW_KEY_W:controls |= FORWARDS;   break;
                case GLFW_KEY_S:controls |= BACKWARDS;  break;
                case GLFW_KEY_SPACE:controls |= DOWN;   break;
                case GLFW_KEY_LEFT_SHIFT:controls |= UP;break;
                case GLFW_KEY_1:controls |= SS;         break;
            }
        }else if(action == GLFW_RELEASE){
            switch(key){
                case GLFW_KEY_D:controls &= ~RIGHT;      break;
                case GLFW_KEY_A:controls &= ~LEFT;       break;
                case GLFW_KEY_W:controls &= ~FORWARDS;   break;
                case GLFW_KEY_S:controls &= ~BACKWARDS;  break;
                case GLFW_KEY_SPACE:controls &= ~DOWN;   break;
                case GLFW_KEY_LEFT_SHIFT:controls &= ~UP;break;
                case GLFW_KEY_1:controls &= ~SS;         break;

            }
        }
    }
    void cursor_position_callback(GLFWwindow* window, double xpos, double ypos){
        mouseX = xpos * sensitivity;
        mouseY = std::max(std::min(3.0,ypos*sensitivity),-3.0);
    }
}