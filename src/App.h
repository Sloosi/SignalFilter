#pragma once

#include "imgui.h"
#include "implot.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include "SignalProcessor.h"
#include "UI/SignalUI.h"

class App
{
public:
    App();
    ~App();
    void Run();

private:
    void RenderUI();
    void RenderPlots();

    GLFWwindow* m_Window;
    SignalProcessor m_Processor;
    SignalProcessor::Config m_Config;
    bool m_NeedsUpdate = true;
};