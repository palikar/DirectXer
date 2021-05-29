#include <imgui.h>

#include "Lighting.hpp"


bool ControlLightingImGui(Lighting& light)
{
	TempFormater formater;
	bool lightChanged = false;

	if (ImGui::CollapsingHeader("Ligting"))
	{
		if (ImGui::TreeNode("Directional light"))
		{
			ImGui::Text("Color");
			ImGui::SameLine();
			lightChanged |= ImGui::ColorEdit3("Color:", (float*)&light.dirLightColor);
			lightChanged |= ImGui::SliderFloat("Intensity: ", (float*)&light.dirLightColor.a, 0.0f, 1.0f, "Amount = %.3f");
			lightChanged |= ImGui::SliderFloat("Angle:", (float*)&light.dirLightDir.y, -1.0f, 1.0f, "Direction = %.3f");
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Ambient light"))
		{
			ImGui::Text("Color");
			ImGui::SameLine();
			lightChanged |=ImGui::ColorEdit3("Color", (float*)&light.ambLightColor);
			lightChanged |= ImGui::SliderFloat("Intensity: ", (float*)&light.ambLightColor.a, 0.0f, 1.0f, "Amount = %.3f");
			ImGui::TreePop();
		}

		for (size_t i = 0; i < PointLightsCount; ++i)
		{
			bool treeActive = ImGui::TreeNodeEx(formater.Format("Point light[{}]", i), ImGuiTreeNodeFlags_AllowItemOverlap);
			ImGui::SameLine(ImGui::GetWindowWidth() - 60);
			ImGui::ToggleButton("Active", (bool*)&light.pointLights[i].Active);

			if(treeActive){
				ImGui::Text("Color");
				ImGui::SameLine();
				lightChanged |= ImGui::ColorEdit3("Color", (float*)&light.pointLights[i].Color);
				lightChanged |= ImGui::SliderFloat("Constant: ", (float*)&light.pointLights[i].Params.r, 0.0f, 2.0f, "%.3f");
				lightChanged |= ImGui::SliderFloat("Linear: ", (float*)&light.pointLights[i].Params.g, 0.0f, 2.0f, "%.3f");
				lightChanged |= ImGui::SliderFloat("Quadreatic: ", (float*)&light.pointLights[i].Params.b, 0.0f, 2.0f, "%.3f");
				ImGui::TreePop();
			}
		}

		for (size_t i = 0; i < SpotLightsCount; ++i)
		{

			bool treeActive = ImGui::TreeNode(formater.Format("Spot light[{}]", i));
			ImGui::SameLine(ImGui::GetWindowWidth() - 60);
			ImGui::ToggleButton("Active", (bool*)&light.spotLights[i].Active);
			if (treeActive)
			{
				ImGui::Text("Color");
				ImGui::SameLine();
				lightChanged |= ImGui::ColorEdit3("Color", (float*)&light.spotLights[i].color);
				lightChanged |= ImGui::SliderFloat("Intensity", (float*)&light.spotLights[i].color.a, 0.0f, 1.0f, "%.3f");

				lightChanged |= ImGui::SliderFloat("Outer Rad", (float*)&light.spotLights[i].Params.r, 0.0f, 5.0f, "%.3f");
				lightChanged |= ImGui::SliderFloat("Inner Rad", (float*)&light.spotLights[i].Params.g, 0.0f, 5.0f, "%.3f");
				ImGui::TreePop();
			}
		}
	}

	return lightChanged;
}
