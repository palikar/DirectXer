#include <imgui.h>

#include "Lighting.hpp"


bool ControlLightingImGui(Lighting& light)
{
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
		
		if (ImGui::TreeNode("Point light"))
		{
			ImGui::Checkbox("Active", (bool*)&light.pointLights[0].Active);
			
			ImGui::Text("Color");
			ImGui::SameLine();
			lightChanged |= ImGui::ColorEdit3("Color", (float*)&light.pointLights[0].Color);
			lightChanged |= ImGui::SliderFloat("Constant: ", (float*)&light.pointLights[0].Params.r, 0.0f, 2.0f, "Amount = %.3f");
			lightChanged |= ImGui::SliderFloat("Linear: ", (float*)&light.pointLights[0].Params.g, 0.0f, 2.0f, "Amount = %.3f");
			lightChanged |= ImGui::SliderFloat("Quadreatic: ", (float*)&light.pointLights[0].Params.b, 0.0f, 2.0f, "Amount = %.3f");
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Spot light"))
		{
			ImGui::Checkbox("Active", (bool*)&light.spotLights[0].Active);
			
			ImGui::Text("Color");
			ImGui::SameLine();
			lightChanged |= ImGui::ColorEdit3("Color", (float*)&light.spotLights[0].color);
			lightChanged |= ImGui::SliderFloat("Intensity", (float*)&light.spotLights[0].color.a, 0.0f, 1.0f, "%.3f");
			
			lightChanged |= ImGui::SliderFloat("Outer Rad", (float*)&light.spotLights[0].Params.r, 0.0f, 5.0f, "%.3f");
			lightChanged |= ImGui::SliderFloat("Inner Rad", (float*)&light.spotLights[0].Params.g, 0.0f, 5.0f, "%.3f");
			ImGui::TreePop();
		}		
	}
	
	return lightChanged;
}
