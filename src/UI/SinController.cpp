#include "SinController.h"

bool SinController(const char* title, SinParam& paramRef)
{
    bool flag = false;

    ImGui::PushID(title);
    ImGui::Text(title);
    
    flag |= ImGui::InputFloat("Amplitude", &paramRef.amplitude, 1, 10, "%.1f");
    flag |= ImGui::InputFloat("Frequency", &paramRef.frequency, 1, 10, "%.1f");
    flag |= ImGui::InputFloat("Phase", &paramRef.phase, 0.1f, 1, "%.2f");
    ImGui::PopID();

    return flag;
}
