#include "App.h"

#include <algorithm>

App::App()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    const char* glsl_version = "#version 460";

    m_Window = glfwCreateWindow(1280, 720, "Signal Filter", nullptr, nullptr);
    glfwMakeContextCurrent(m_Window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsClassic();
    ImPlot::StyleColorsClassic();

    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

App::~App()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    ImPlot::DestroyContext();

    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void App::Run()
{
    while (!glfwWindowShouldClose(m_Window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        RenderControlPanel();

        if (m_NeedsUpdate)
        {
            m_Processor.Update(m_Config);
            m_NeedsUpdate = false;
        }

        RenderPlots();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(m_Window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(m_Window);
    }
}

void App::RenderControlPanel()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    //Control Panel
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(ImVec2(400, viewport->WorkSize.y));
    if (ImGui::Begin("Control Panel", nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
    {
        int oldPointCount = m_Config.pointCount;

        ImGui::Separator();
        m_NeedsUpdate |= SinController("First harmonic", m_Config.sin1);
        ImGui::Separator();
        m_NeedsUpdate |= SinController("Second harmonic", m_Config.sin2);
        ImGui::Separator();
        m_NeedsUpdate |= SinController("Third harmonic", m_Config.sin3);
        ImGui::Separator();

        m_NeedsUpdate |= ImGui::InputInt("Sample Rate", &m_Config.sampleRate, 100);
        m_NeedsUpdate |= ImGui::InputInt("Points", &m_Config.pointCount, 100);
        ImGui::Separator();
        m_NeedsUpdate |= ImGui::SliderFloat("Noise alpha", &m_Config.noiseAlpha, 0.0f, 2.0f, "%.2f");
        m_NeedsUpdate |= ImGui::SliderFloat("Gamma", &m_Config.gamma, 0.0f, 1.0f, "%.2f");
        m_NeedsUpdate |= ImGui::Checkbox("Show Noise", &m_Config.showNoise);

        m_Config.sampleRate = std::max(1, m_Config.sampleRate);
        m_Config.pointCount = std::max(1024, m_Config.pointCount);
        m_Config.noiseAlpha = std::max(0.0f, m_Config.noiseAlpha);

        m_Config.pointCount = (oldPointCount < m_Config.pointCount) ? oldPointCount << 1
            : (oldPointCount > m_Config.pointCount) ? oldPointCount >> 1
            : oldPointCount;

        ImGui::Separator();

        ImGui::Text("Delta: %.2f", m_Processor.GetDelta());

        ImGui::End();
    }
}

void App::RenderPlots()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    int timeCount = m_Processor.GetTime().size();
    const double* timeData = m_Processor.GetTime().data();
    const double* cleanSignalData = m_Processor.GetCleanSignal().data();

    bool showNoise = m_Processor.IsShowNoise();
    const double* inputSignalData = showNoise? m_Processor.GetInputSignal().data() :
                                               m_Processor.GetIdealSignal().data();


    int frequenciesCount = m_Processor.GetTime().size();
    const double* frequenciesData = m_Processor.GetFrequencies().data();
    const double* inputSpectrumData = m_Processor.GetInputSpectrum().data();
    const double* cleanSpectrumData = m_Processor.GetCleanSpectrum().data();

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + 400, viewport->WorkPos.y));
    ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x - 400, viewport->WorkSize.y / 3));
    PlotRenderer("Input signal", "t, sec", "x", timeData, inputSignalData, timeCount);

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + 400, viewport->WorkPos.y + (1.0 / 3.0) * viewport->WorkSize.y));
    ImGui::SetNextWindowSize(ImVec2((viewport->WorkSize.x - 400) / 2, viewport->WorkSize.y / 3));
    PlotRenderer("Spectrum signal (noise)", "f, Hz", "Ampl", frequenciesData, inputSpectrumData, frequenciesCount);

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + 400 + (viewport->WorkSize.x - 400) / 2, viewport->WorkPos.y + (1.0 / 3.0) * viewport->WorkSize.y));
    ImGui::SetNextWindowSize(ImVec2((viewport->WorkSize.x - 400) / 2, viewport->WorkSize.y / 3));
    PlotRenderer("Spectrum signal (clear)", "f, Hz", "Ampl", frequenciesData, cleanSpectrumData, frequenciesCount);

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + 400, viewport->WorkPos.y + (2.0 / 3.0) * viewport->WorkSize.y));
    ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x - 400, viewport->WorkSize.y / 3));
    PlotRenderer("Clear signal", "t, sec", "x", timeData, cleanSignalData, timeCount);
}
