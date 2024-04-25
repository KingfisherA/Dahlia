
#include "Application.h"

#include <filesystem>
#include <ImGui/imgui.h>
#define GLM_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include "GraphicsEngine/Events/Events.h"
#include "GraphicsEngine/Events/KeyEvents.h"
#include "GraphicsEngine/Events/MouseEvents.h"
#include "GraphicsEngine/3D/VertexArray.h"
#include "GraphicsEngine/Platform/Windows/Console.h"
#include "GraphicsEngine/Platform/API Core.h"
#include "GraphicsEngine/Utilities/Memory.h"
#include "GraphicsEngine/GUI/Display.h"
#include "GraphicsEngine/3D/Texture.h"
#include "GraphicsEngine/GUI/ImGuiLayer.h"
#include "GraphicsEngine/3D/Framebuffer.h"

#include "GraphicsEngine/Utilities/IO.h"
#include "GraphicsEngine/Utilities/PathUtils.h"
#include "GraphicsEngine/3D/Framebuffer.h"

// #define IMSPINNER_DEMO
// #include "GUI/ImSpinner.h"

#pragma region Entry Point

int main(int argc, char **argv)
{
	Example::Application app;
	if (!app.Init())
	{
		std::cout << "Failed to initilise application \n";
		return EXIT_FAILURE;
	}
	app.Run();
	return EXIT_SUCCESS;
}

#pragma endregion

namespace Example
{
	using namespace GEng;
	// Constants
	constexpr float NEAR_PLANE = 0.1f;
	constexpr float FAR_PLANE = 100.0f;

	// // Static Variable Initilisation
	// glm::mat4 Application::projectionMatrix(1.0f);

#define BIND_EVENT_FNC(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application::Application() : GraphicsApp("Example Graphics Engine Application", 800, 600, FramebufferSpecification{800, 600, 1, {FramebufferAttachment(PixelFormat::RGBA8), FramebufferAttachment(PixelFormat::DEPTH24_STENCIL8)}})
	{
	}

	Application::~Application()
	{
	}

	void Application::UpdateProjectionMatrix()
	{
		projectionMatrix = glm::perspective<float>(glm::radians(45.0f),
												   (float)mViewportFramebuffer->GetSpecification().Width / (float)mViewportFramebuffer->GetSpecification().Height, NEAR_PLANE, FAR_PLANE);
	}

	void Application::OnImGuiRender()
	{
		RenderCameraPanel();
		RenderSceneInformation();
	}

	void Application::OnViewportResize()
	{
		UpdateProjectionMatrix();
	}

	bool Application::OnInit()
	{
		std::cout << "Example Application\n";

		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		this->mRenderViewport = false;
		mCam.MoveTo(glm::vec3(0.0f, 0.0f, 0.0f), 10.0f);
		UpdateProjectionMatrix();

		return true;
	}

	void Application::RenderCameraPanel()
	{
		if (!ImGui::Begin("Camera"))
		{
			ImGui::End();
			return;
		}

		if (ImGui::CollapsingHeader("Camera Properties", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent();
			static glm::vec3 newPosition{0.0f};
			static float newZoom = 0.0f;
			const glm::vec3 &centrePos = mCam.GetCentrePosition();
			const glm::vec3 &actualPos = mCam.GetActualPosition();

			newPosition = centrePos;
			newZoom = mCam.GetZoom();

			if (ImGui::Button("Reset Camera##cam"))
			{
				newPosition = {0.0f, 0.0f, 0.0f};
				newZoom = 10.0f;
			}

			ImGui::Text("Camera Centre Position:");
			ImGui::DragFloat3("##cam_centre_pos", &newPosition[0]);
			ImGui::SameLine();
			if (ImGui::Button("Reset##cam_pos"))
				newPosition = {0.0f, 0.0f, 0.0f};
			if (newPosition != centrePos)
				mCam.SetCentrePosition(newPosition);

			ImGui::Text("Camera Zoom:");
			ImGui::DragFloat("##cam_zoom", &newZoom, 0.5f);
			ImGui::SameLine();
			if (ImGui::Button("Reset##cam_zoom"))
				newZoom = 10.0f;
			if (newZoom != mCam.GetZoom())
				mCam.SetZoom(newZoom);

			ImGui::Text("Camera Actual Position: X=%.2f, Y=%.2f, Z=%.2f", actualPos.x, actualPos.y, actualPos.z);
			ImGui::Text("Pitch=%.2f, Yaw=%.2f, Roll=%.2f", mCam.GetPitch(), mCam.GetYaw(), mCam.GetRoll());
			ImGui::Unindent();
		}

		if (ImGui::CollapsingHeader("Render Style", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent();
			const char *items[] = {"Solid", "Wireframe"};
			static int selectedItem = 0;
			ImGui::Combo("Render Style##combo", &selectedItem, items, 2);

			for (Layer *layer : m_LayerStack)
				layer->SetRenderStyle(selectedItem == 0 ? GL_FILL : GL_LINE);

			ImGui::Unindent();
		}

		ImGui::End();
	}

	void Application::RenderSceneInformation()
	{
		if (!ImGui::Begin("Scene Information"))
		{
			ImGui::End();
			return;
		}

		if (ImGui::CollapsingHeader("Adaptor Information", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Text("Renderer API: %s", m_VersionStr.c_str());
			ImGui::Text("Vendor: %s", m_VendorStr.c_str());
			ImGui::Text("Shading Language Version: %s", m_ShadingLanguageStr.c_str());
		}

		if (ImGui::CollapsingHeader("Frame Information", ImGuiTreeNodeFlags_DefaultOpen))
		{
			const ImGuiIO &myIO = ImGui::GetIO();
			ImGui::Text("Frame rate: %.0f fps", myIO.Framerate);
			ImGui::Text("Frame time: %.1f ms", myIO.DeltaTime * 1000.0f);
		}

		ImGui::End();
	}
}
