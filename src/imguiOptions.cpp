#include "raylib.h"
#ifndef NO_IMGUI

#include "imgui.h"
#include "rlImGui.h"

#include "globals.cpp"

#include "cube.h"

//********** ImGui & DrawText stuff
ImVec4 RaylibColorToImVec4(Color column) {
    return ImVec4(
        column.r / 255.0f, // Red (0 to 1)
        column.g / 255.0f, // Green (0 to 1)
        column.b / 255.0f, // Blue (0 to 1)
        column.a / 255.0f  // Alpha (0 to 1)
		);
}

// Convert ImGui color format back to Raylib Color (0 to 255)
Color ImVec4ToRaylibColor(ImVec4 column) {
    return Color{
        (unsigned char)(column.x * 255), // Red (0 to 255)
        (unsigned char)(column.y * 255), // Green (0 to 255)
        (unsigned char)(column.z * 255), // Blue (0 to 255)
        (unsigned char)(column.w * 255)  // Alpha (0 to 255)
    };
}

void imguiMenus(Cube& cube, CubeCamera& camera) {

	rlImGuiBegin();
	ImGui::Begin("Main Control");
	ImGui::Checkbox("Keyboard & Mouse", &ops.inputWindow);
	ImGui::Checkbox("Entities", &ops.entitiesWindow);
	ImGui::Checkbox("Cube & Camera", &ops.cubeWindow);
	ImGui::Checkbox("Lights", &ops.lightsWindow);
	ImGui::Checkbox("ImGui Demo", &ops.demoWindow);
	ImGui::Checkbox("Sound Enabled", &ops.soundEnabled);
	ImGui::End();

	
	// ********* KEYBOARD AND MOUSE ********* 
	if (ops.inputWindow) {
		ImGui::Begin("Keyboard & Mouse");

		ImGui::SeparatorText("Keyboard");
		bool isShiftDown = (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT));
		ImGui::Text("kb.pressedKey: ");  ImGui::SameLine(180); 
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%i", kb.pressedKey);
		ImGui::Checkbox("IsShiftDown", &isShiftDown);
		ImGui::Checkbox("hasQueuedKey", &kb.hasQueuedKey);
		ImGui::Spacing();
		ImGui::Text("Press/Release time:"); ImGui::SameLine(180);
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%.3f", kb.pressReleaseTime);
		ImGui::Text("Cube animation speed:"); ImGui::SameLine(180);
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%.3f", cube.animationSpeed); ImGui::SameLine(280);
		ImGui::Text("Pitch change:"); ImGui::SameLine(380);
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%.3f", cube.pitchChange);
		ImGui::Spacing();
	
		ImGui::SeparatorText("Mouse Positions");
	
		ImGui::Text("Screen position: (%0.0f, %0.0f)", mouse.position.x, mouse.position.y);
		Vector3 xzPos = getMouseXZPosition(camera.c3d);
		ImGui::Spacing();
		ImGui::Text("Plane XZ position: (%0.2f, %0.2f)", xzPos.x, xzPos.z);
		ImGui::Spacing();
		ImGui::SeparatorText("Mouse Wheel");
		ImGui::Checkbox("Zoom Enabled", &mouse.zoomEnabled);
		ImGui::Text("Speed"); ImGui::SameLine();
		ImGui::DragFloat("##1", (float *)&mouse.zoomSpeed, 0.1f, 0.1f, 10.0f);
		ImGui::Text("Max distance"); ImGui::SameLine();
		ImGui::DragFloat("##2", (float *)&mouse.minZoomDistance, 0.1f, 0.1f, 10.0f);
		ImGui::Text("Min distance"); ImGui::SameLine();
		ImGui::DragFloat("##3", (float *)&mouse.maxZoomDistance, 1.0f, 10.0f, 1000.0f);
		ImGui::End();
	}

	// ********* CUBE AND CAMERA  ********* 
	if (ops.cubeWindow) {
		ImGui::Begin("Cube, Camera & Other");
		ImGui::SeparatorText("Cube");
		if (cube.state == Cube::QUIET)
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "QUIET");
		else if (cube.state == Cube::ROLLING)
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "ROLLING");
		else if (cube.state == Cube::PUSHING)
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "PUSHING");
		else 
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "ERROR");			
		
		ImGui::Text("pIndex (ix, iz):"); ImGui::SameLine(140);
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "(%i, %i)", cube.pIndex.x, cube.pIndex.z);
		
		
		ImGui::DragFloat3("position", (float *)&cube.position, 1.0f, -1000.0f, 1000.0f);
		ImGui::DragFloat3("next position", (float *)&cube.nextPosition, 1.0f, -1000.0f, 1000.0f);
		ImGui::DragFloat3("direction", (float *)&cube.direction, 1.0f, -1000.0f, 1000.0f);
		ImGui::DragFloat3("rotationAxis", (float *)&cube.rotationAxis, 1.0f, -1000.0f, 1000.0f);
		ImGui::DragFloat3("rotationPivot", (float *)&cube.rotationPivot, 1.0f, -1000.0f, 1000.0f);
		ImGui::DragFloat("rotationAngle", (float *)&cube.rotationAngle, 1.0f, 200.0f, 2000.0f);
		ImVec4 cubeFacesColor = RaylibColorToImVec4(cube.facesColor);
		if (ImGui::ColorEdit4("faces color", &cubeFacesColor.x)) {
			cube.facesColor = ImVec4ToRaylibColor(cubeFacesColor);
		}
		ImVec4 cubeWiresColor = RaylibColorToImVec4(cube.wiresColor);
		if (ImGui::ColorEdit4("wires color", &cubeWiresColor.x)) {
			cube.wiresColor = ImVec4ToRaylibColor(cubeWiresColor);
		}
		ImGui::Spacing();
		ImGui::Text("transform Matrix");
		Matrix& t = cube.transform;
		Vector4 v[4] = { 
			{t.m0, t.m4, t.m8 , t.m12}, 
			{t.m1, t.m5, t.m9 , t.m13}, 
			{t.m2, t.m6, t.m10, t.m14}, 
			{t.m3, t.m7, t.m11, t.m15}, 
			
			// {t.m0*RAD2DEG, t.m4*RAD2DEG, t.m8*RAD2DEG, t.m12*RAD2DEG}, 
			// {t.m1*RAD2DEG, t.m5*RAD2DEG, t.m9*RAD2DEG, t.m13*RAD2DEG}, 
			// {t.m2*RAD2DEG, t.m6*RAD2DEG, t.m10*RAD2DEG, t.m14*RAD2DEG}, 
			// {t.m3*RAD2DEG, t.m7*RAD2DEG, t.m11*RAD2DEG, t.m15*RAD2DEG}, 
			
		};
		ImGui::DragFloat4("|", (float *)&v[0], 1.0f, -1000.0f, 1000.0f, "%.2f");
		ImGui::DragFloat4("|", (float *)&v[1], 1.0f, -1000.0f, 1000.0f, "%.2f");
		ImGui::DragFloat4("|", (float *)&v[2], 1.0f, -1000.0f, 1000.0f, "%.2f");
		ImGui::DragFloat4("|", (float *)&v[3], 1.0f, -1000.0f, 1000.0f, "%.2f");
		

		ImGui::SeparatorText("Camera");
		ImGui::DragFloat3("position ", (float *)&camera.c3d.position, 1.0f, -1000.0f, 1000.0f);
		ImGui::DragFloat3("target ", (float *)&camera.c3d.target, 1.0f, -1000.0f, 1000.0f);
		float camAngleX = camera.angleX * RAD2DEG;
		float camAngleY = camera.angleY * RAD2DEG;
		ImGui::DragFloat("angle_x", (float*)&camAngleX, 1.0f, 200.0f, 2000.0f);
		ImGui::DragFloat("angle_y", (float*)&camAngleY, 1.0f, 200.0f, 2000.0f);
		ImGui::Spacing();
	
		ImGui::SeparatorText("Other");
		ImGui::Checkbox("drawAxis", &ops.drawAxis);	
		ImGui::Checkbox("Colored Ground plane", &ops.coloredGround);
		ImGui::End();
	}
	
	// ********* ENTITIES  ********* 
	if (ops.entitiesWindow) {
		
		ImGui::Begin("Entities");
		ImGui::Text("Total number:"); ImGui::SameLine(120);
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%i", entityPool.getCount());
		
		if (ImGui::TreeNode("List of entities")) {
			
			static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg 
				| ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp
				| ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
			
			const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
			ImVec2 outer_size = ImVec2(0.0f, TEXT_BASE_HEIGHT * 8);
			ImGui::BeginTable("entities", 3, flags, outer_size);
			ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
			ImGui::TableSetupColumn("id");
			ImGui::TableSetupColumn("pIndex");
			ImGui::TableSetupColumn("type");
			ImGui::TableHeadersRow();
			
			for (int id = 0; id < entityPool.getCount(); id++) {
				Entity e = entityPool.getEntity(id);
				ImGui::TableNextRow();
				for (int column = 0; column < 3; column++) {
					ImGui::TableSetColumnIndex(column);
					if (column == 0) ImGui::Text("%i", id);
					else if (column == 1) ImGui::Text("(%i, %i)", e.pIndex.x, e.pIndex.z);
					else ImGui::Text("%s", getBoxType(e.type));
				}
			}
			ImGui::EndTable();
			
			ImGui::TreePop();
		}
		ImGui::Spacing();
		
		ImGui::SeparatorText("Ground Cell");
		PositionIndex pIndex;
		getMouseXZindexes(camera.c3d, pIndex);

		ImGui::Text("pIndex:"); ImGui::SameLine(70);
		if (!isValidPositionIndex(pIndex)) {
			
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "INVALID"); ImGui::SameLine(150);
			ImGui::Text("isEmpty: "); ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "-");
			ImGui::Text("EntityId: "); ImGui::SameLine();			
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "-"); ImGui::SameLine(150);
			ImGui::Text("type: "); ImGui::SameLine(); 
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "-"); ImGui::SameLine(300);
			ImGui::Text("hidden: "); ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "-");
		} else {
			
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "(%i, %i)", pIndex.x, pIndex.z);	ImGui::SameLine(150);
			Cell cell = ground.cells[pIndex.x][pIndex.z];
			ImGui::Text("isEmpty: "); ImGui::SameLine();
			ImGui::TextColored(ImVec4(cell.isEmpty ? 1.0f : 0.0f, cell.isEmpty ? 0.0f : 1.0f, 0.0f, 1.0f), 
							   "%s", cell.isEmpty ? "true" : "false");
			ImGui::Text("EntityId: "); ImGui::SameLine();
			if (!cell.isEmpty) {
				Entity e = entityPool.getEntity(cell.entityId);
				ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.9f, 1.0f), "%i", cell.entityId); ImGui::SameLine(150);
				ImGui::Text("type: "); ImGui::SameLine();
				ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.9f, 1.0f), "%s", getBoxType(e.type)); ImGui::SameLine(300);
				ImGui::Text("hidden: "); ImGui::SameLine();			
				ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.9f, 1.0f), "%s", e.hidden ? "true" : "false");
			} else {
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "-"); ImGui::SameLine(150);
				ImGui::Text("type: "); ImGui::SameLine(); 
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "-"); ImGui::SameLine(300);
				ImGui::Text("hidden: "); ImGui::SameLine();
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "-");
			}
		}

		ImGui::Spacing();
		ImGui::SeparatorText("Add/Remove entity");
		ImGui::RadioButton("Obstacle", &ops.entityType, OBSTACLE); ImGui::SameLine();
		ImGui::RadioButton("PushBox", &ops.entityType, PUSHBOX); ImGui::SameLine();
		ImGui::RadioButton("PullBox", &ops.entityType, PULLBOX); ImGui::SameLine();
		ImGui::RadioButton("PushPullBox", &ops.entityType, PUSHPULLBOX); ImGui::SameLine();
		ImGui::RadioButton("Other", &ops.entityType, OTHER);
		
		ImGui::Spacing();
		ImGui::Checkbox("editEnabled", &ops.editEnabled);
		
		ImGui::End();
	}
	

	// ********* LIGHTS  ********* 
	if (ops.lightsWindow) {
		ImGui::Begin("Lighting");
		ImGui::SeparatorText("Lights");
		ImGui::ColorEdit4("ambient", &sld.ambient.x);
		ImGui::Text("lightsCount:"); ImGui::SameLine(180);
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%i", sld.lightsCount);
		ImVec4 lightColor[6];
		for (int i=0; i<=5; i++) {
			const char* type = (sld.lights[i].type == LIGHT_POINT ? "Point" : "Directional");
			const char* lightName = TextFormat("Light %i - %s", i, type);
			if (ImGui::CollapsingHeader( lightName)) {
				const char* posStr = TextFormat("position##%i", i);
				const char* colorStr = TextFormat("color##%i", i);
				const char* enableStr = TextFormat("enable##%i", i);
				const char* hiddenStr = TextFormat("hidden##%i", i);
				ImGui::Checkbox(enableStr, &sld.lights[i].enabled);ImGui::SameLine(140);
				ImGui::Checkbox(hiddenStr, &sld.lights[i].hidden);
				ImGui::DragFloat3(posStr, (float *)&sld.lights[i].position, 0.5f, -20.0f, 20.0f);
				lightColor[i] = RaylibColorToImVec4(sld.lights[i].color);
				if (ImGui::ColorEdit4(colorStr, &lightColor[i].x)) {
					sld.lights[i].color = ImVec4ToRaylibColor(lightColor[i]);
				}
				if (sld.lights[i].type == LIGHT_DIRECTIONAL) {
					const char* targetStr = TextFormat("target##%i", i);
					ImGui::DragFloat3(targetStr, (float *)&sld.lights[i].target, 0.5f, -100.0f, 100.0f);
				}
				ImGui::Spacing();
			}
		}

// ImGui::Checkbox("isEmpty light", &camera.freeLight);        
// ImGui::DragFloat3("position  ", (float *)&camera.light.position, 0.2f, -1000.0f, 1000.0f);
// ImGui::DragFloat3("target  ", (float *)&camera.light.target, 0.2f, -1000.0f, 1000.0f);
// ImVec4 lightColor = RaylibColorToImVec4(camera.light.color);
// if (ImGui::ColorEdit4("color", &lightColor.x)) {
// 	camera.light.color = ImVec4ToRaylibColor(lightColor);
// }
		ImGui::End();
	}

	
	if (ops.demoWindow) {
		ImGui::ShowDemoWindow(&ops.demoWindow);
	}
	rlImGuiEnd();
}
#endif
