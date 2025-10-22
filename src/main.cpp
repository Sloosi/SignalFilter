#include "imgui.h"
#include "implot.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include <print>
#include <vector>

#include "UI/SinController.h"
#include "fft.h"

SinParam sin1 = {10, 10, 0};
SinParam sin2 = {0, 0, 0};
SinParam sin3 = {0, 0, 0};

int FS = 1'024;

int data_count = 1024;
float alpha = 1.0f;

double CalculateEnergy(const std::vector<double>& data)
{
    double energy = 0;
    for (double s : data)
    {
        energy += (s * s);
    }
    return energy;
}

void CreateWhiteNoise(std::vector<double>& data)
{
    for (size_t i = 0; i < data.size(); i++)
    {
        int noise = 0;
        for (size_t s = 0; s < 12; s++)
        {
            noise += rand();
        }
        data[i] = (noise - static_cast<double>(RAND_MAX / 2)) / RAND_MAX;
    }
}

static void glfw_error_callback(int error, const char* description)
{
    std::println(stderr, "GLFW Error %d: %s\n", error, description);
}

void FillSinData(std::vector<double>& x_data, std::vector<double>& y_data, std::vector<double>& noise)
{
    float time_step = 1.0f / FS;
    x_data.clear();
    y_data.clear();
    x_data.reserve(data_count);
    y_data.reserve(data_count);

    for (int i = 0; i < data_count; i++)
    {
        double sin1_data = sin1.amplitude * sin(2 * PI * sin1.frequency * time_step * i + sin1.phase);
        double sin2_data = sin2.amplitude * sin(2 * PI * sin2.frequency * time_step * i + sin2.phase);
        double sin3_data = sin3.amplitude * sin(2 * PI * sin3.frequency * time_step * i + sin3.phase);

        x_data.push_back(time_step * i);
        y_data.push_back(sin1_data + sin2_data + sin3_data);
    }

    double sign_energy = CalculateEnergy(y_data);
    double noise_energy = CalculateEnergy(noise);

    double beta = sign_energy * alpha /noise_energy;

    for (int i = 0; i < data_count; i++)
    {
        noise[i] *= beta;
        y_data[i] += noise[i];
    }
}

void CalculateSpectrum(const std::vector<double>& signal, std::vector<double>& ampls, std::vector<double>& freqs, std::vector<Complex>& spec)
{
    auto N = signal.size();

    auto fft_res = fft_real(signal);
    spec = fft_res;
    ampls = amplitude_spectrum(fft_res);
    
    freqs.resize(N);
    ampls.resize(N);
    for (int k = 0; k < N; k++)
    {
        freqs[k] = static_cast<float>(k * FS) / N;
    }

}

std::vector<Complex> ClearSpectrum(std::vector<Complex> spec, const std::vector<double>& ampl_data)
{
    double sn_energy = CalculateEnergy(ampl_data);
    double s_energy = sn_energy / (1 + alpha);

    double energy_counter = 0;

    size_t k = 0;
    for (; k < data_count; k++)
    {
        energy_counter += (ampl_data[k] * ampl_data[k]);
        energy_counter += (ampl_data[data_count - 1 - k] * ampl_data[data_count - 1 - k]);
        if (energy_counter > s_energy) break;
    }

    for (size_t i = k + 1; i < data_count - k - 2; i++)
        spec[i] = 0;

    return spec;
}

void RenderPlot(const char* title, const char* x_lable, const char* y_lable, const double* x_data, const double* y_data, int count = data_count)
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

    std::vector<double> x_data, y_data, ampls, freqs, noise_data, clear_ampls;
    std::vector<Complex>  spec_data, clear_spec;
    noise_data.resize(data_count);
    CreateWhiteNoise(noise_data);
    FillSinData(x_data, y_data, noise_data);
    CalculateSpectrum(y_data, ampls, freqs, spec_data);
    clear_spec = ClearSpectrum(spec_data, ampls);
    clear_ampls = amplitude_spectrum(clear_spec);
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
                is_changed |= ImGui::InputInt("Sample rate", &FS, 100);
                is_changed |= ImGui::InputInt("Count of points", &data_count, 100);
                is_changed |= ImGui::InputFloat("Noise multiplier", &alpha, 0.1, 0.1, "%.1f");
            }
            ImGui::End();
            
            if (is_changed)
            {
                CreateWhiteNoise(noise_data);
                FillSinData(x_data, y_data, noise_data);
                CalculateSpectrum(y_data, ampls, freqs, spec_data);
                clear_spec = ClearSpectrum(spec_data, ampls);
                clear_ampls = amplitude_spectrum(clear_spec);
            }

            ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + 400, viewport->WorkPos.y));
            ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x - 400, viewport->WorkSize.y / 3));
            RenderPlot("Input signal", "t, sec", "x", x_data.data(), y_data.data());

            ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + 400, viewport->WorkPos.y + 0.33 * viewport->WorkSize.y));
            ImGui::SetNextWindowSize(ImVec2((viewport->WorkSize.x - 400) / 2, viewport->WorkSize.y / 3));
            RenderPlot("Spectrum signal (noise)", "f, Hz", "Ampl", freqs.data(), ampls.data(), freqs.size());

            ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + 400 + (viewport->WorkSize.x - 400) / 2, viewport->WorkPos.y + 0.33 * viewport->WorkSize.y));
            ImGui::SetNextWindowSize(ImVec2((viewport->WorkSize.x - 400) / 2, viewport->WorkSize.y / 3));
            RenderPlot("Spectrum signal (clear)", "f, Hz", "Ampl", freqs.data(), clear_ampls.data(), freqs.size());
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