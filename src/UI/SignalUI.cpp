#include "SignalUI.h"

#include "imgui.h"
#include "implot.h"

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

void PlotRenderer(const char* title, const char* x_lable, const char* y_lable,
                  const double* x_data, const double* y_data, int count)
{
    if (ImGui::Begin(title, nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
    {
        auto plotFlags = ImPlotFlags_NoMenus | ImPlotFlags_NoLegend;
        auto axisFlags = ImPlotAxisFlags_AutoFit;

        if (ImPlot::BeginPlot(title, x_lable, y_lable, ImGui::GetContentRegionAvail(),
            plotFlags, axisFlags, axisFlags))
        {
            ImPlot::PlotLine(title, x_data, y_data, count);
            ImPlot::EndPlot();
        }
    }
    ImGui::End();
}