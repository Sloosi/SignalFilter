#include "imgui.h"
#include "implot.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include <print>
#include <vector>

#include "UI/SinController.h"

SinParam sin1 = {10, 10, 0};
SinParam sin2 = {15, 20, 0};
SinParam sin3 = {0, 0, 0};

float freq = 1'00;

static void glfw_error_callback(int error, const char* description)
{
    std::println(stderr, "GLFW Error %d: %s\n", error, description);
}

void FillSinData(std::vector<double>& x_data, std::vector<double>& y_data)
{
    float time_step = 1 / freq;
    x_data.clear();
    y_data.clear();
    x_data.reserve(1000);
    y_data.reserve(1000);
    for (int i = 0; i < (int)freq; i++)
    {
        double sin1_data = sin1.amplitude * sin(sin1.frequency * time_step * i + sin1.phase);
        double sin2_data = sin2.amplitude * sin(sin2.frequency * time_step * i + sin2.phase);
        double sin3_data = sin3.amplitude * sin(sin3.frequency * time_step * i + sin3.phase);

        x_data.push_back(time_step * i);
        y_data.push_back(sin1_data + sin2_data + sin3_data);
    }
}

void RenderPlot(const char* title, const char* x_lable, const char* y_lable, const double* x_data, const double* y_data)
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
            ImPlot::PlotLine(title, x_data, y_data, freq);
            ImPlot::EndPlot();
        }
    }
    ImGui::End();
}

int main(int, char**)
{
    /*================= INIT GLFW ====================*/
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char* glsl_version = "#version 460";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Signal Filter", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    /*================================================*/

    /*================= INIT IMGUI ===================*/
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ImGui::StyleColorsClassic();
    ImPlot::StyleColorsClassic();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    /*================================================*/
    ImVec4 clear_color = ImVec4(0.02f, 0.02f, 0.03f, 1.0f);

    std::vector<double> x_data, y_data;
    FillSinData(x_data, y_data);
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiViewport* viewport = ImGui::GetMainViewport();

        
        //Left Panel
        {
            bool is_changed = false;
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(ImVec2(400, viewport->WorkSize.y));
            if (ImGui::Begin("Left Panel", nullptr,
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
            {

                ImGui::Separator();
                is_changed |= SinController("First harmonic", sin1);
                ImGui::Separator();
                is_changed |= SinController("Second harmonic", sin2);
                ImGui::Separator();
                is_changed |= SinController("Third harmonic", sin3);
                ImGui::Separator();

                ImGui::Text("Other params");
                is_changed |= ImGui::SliderFloat("Sampling rate", &freq, 1, 1000, "%.0f");
            }
            ImGui::End();
            
            if (is_changed) FillSinData(x_data, y_data);

            ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + 400, viewport->WorkPos.y));
            ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x - 400, viewport->WorkSize.y / 3));
            RenderPlot("Input signal", "t, sec", "x", x_data.data(), y_data.data());
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, 
                     clear_color.y * clear_color.w, 
                     clear_color.z * clear_color.w, 
                     clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    ImPlot::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}