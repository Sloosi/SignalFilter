#pragma once

#include <functional>
#include <vector>
#include "imgui.h"

struct SinParam
{
    float amplitude = 0;
    float frequency = 0;
    float phase = 0;
};

bool SinController(const char* title, SinParam& paramRef);