#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "Dahlia/GUI/Display.h"
#include "Dahlia/Platform/OpenGL/OpenGLImGuiRenderer.h"

namespace DH
{
    OpenGLImGuiRenderer::OpenGLImGuiRenderer(const Display& display)
    {
        ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(display.GetAPIDisplay()), true);
        ImGui_ImplOpenGL3_Init("#version 410");
    }

    OpenGLImGuiRenderer::~OpenGLImGuiRenderer()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
    }

    void OpenGLImGuiRenderer::NewFrame(CommandList&)
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
    }

    void OpenGLImGuiRenderer::Render(ImDrawData* drawData, CommandList&)
    {
        ImGui_ImplOpenGL3_RenderDrawData(drawData);
    }

    void OpenGLImGuiRenderer::RenderPlatformWindows()
    {
        // Secondary windows have their own GL contexts; restore the main one afterwards
        GLFWwindow* backupContext = glfwGetCurrentContext();
        ImGuiRenderer::RenderPlatformWindows();
        glfwMakeContextCurrent(backupContext);
    }
}
