#include "SinController.h"

bool SinController(const char* title, SinParam& paramRef)
{
    bool flag = false;

    ImGui::PushID(title);
    ImGui::Text(title);
    
    flag |= ImGui::SliderFloat("Amplitude", &paramRef.amplitude, 0, 100, "%.1f");
    flag |= ImGui::SliderFloat("Frequency", &paramRef.frequency, 0, 100, "%.1f");
    flag |= ImGui::SliderFloat("Phase", &paramRef.phase, -6.30f, 6.30f, "%.2f");
    ImGui::PopID();

    return flag;
}
