#pragma once
#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>

namespace Imgn
{
	class EditorWorkspace
	{
		ImGuiID _workspaceID = ImHashStr("Imagination.Workspaces.v1");
		ImGuiID _sceneID = ImHashStr("Imagination.ScenePanels.v1");

		ImGuiWindowClass _workspaceClass, _sceneClass;

		bool _resetLayout = false;
		bool _focusScene = false;
		bool _focusBlender = false;

	public:
		EditorWorkspace()
		{
			_workspaceClass.ClassId = ImHashStr("Imagination.WorkspaceClass");
			_workspaceClass.DockingAllowUnclassed = false;

			_sceneClass.ClassId = ImHashStr("Imagination.ScenePanelClass");
			_sceneClass.DockingAllowUnclassed = false;
		}

		~EditorWorkspace()
		{

		}

		const ImGuiWindowClass& GetSceneClass() const { return _sceneClass; }
		void FocusScene() { _focusScene = true; }
		void FocusBlender() { _focusBlender = true; }
		void ResetLayout() { _resetLayout = true; _focusBlender = false; }

        void DrawDockspace()
        {
            if (_resetLayout)
            {
                ImGui::DockBuilderRemoveNode(_sceneID);
                ImGui::DockBuilderRemoveNode(_workspaceID);
                _resetLayout = false;
            }

            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            const ImGuiDockNodeFlags flags = ImGuiDockNodeFlags_NoDockingSplit | ImGuiDockNodeFlags_NoUndocking;

            if (!ImGui::DockBuilderGetNode(_workspaceID))
            {
                ImGui::DockBuilderAddNode(_workspaceID, ImGuiDockNodeFlags_DockSpace | flags);
                ImGui::DockBuilderSetNodePos(_workspaceID, viewport->WorkPos);
                ImGui::DockBuilderSetNodeSize(_workspaceID, viewport->WorkSize);

                ImGui::DockBuilderDockWindow("Scene Editor", _workspaceID);
                ImGui::DockBuilderDockWindow("Blender", _workspaceID);

                ImGui::DockBuilderFinish(_workspaceID);
                _focusScene = true;
            }

            ImGui::DockSpaceOverViewport(_workspaceID, viewport, flags, &_workspaceClass);
        }

        bool BeginSceneEditor()
        {
            ImGui::SetNextWindowClass(&_workspaceClass);

            if (_focusScene)
            {
                ImGui::SetNextWindowFocus();
                _focusScene = false;
            }

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
            const bool visible = ImGui::Begin("Scene Editor", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoFocusOnAppearing);
            ImGui::PopStyleVar();

            const ImVec2 size = ImGui::GetContentRegionAvail();

            if (visible && size.x > 1.f && size.y > 1.f)
            {
                if (!ImGui::DockBuilderGetNode(_sceneID))
                {
                    ImGui::DockBuilderAddNode(_sceneID, ImGuiDockNodeFlags_DockSpace);
                    ImGui::DockBuilderSetNodePos(_sceneID, ImGui::GetCursorScreenPos());
                    ImGui::DockBuilderSetNodeSize(_sceneID, size);

                    ImGuiID scene = _sceneID;
                    ImGuiID hierarchy = ImGui::DockBuilderSplitNode(scene, ImGuiDir_Right, 0.25f, nullptr, &scene);
                    ImGuiID inspector = ImGui::DockBuilderSplitNode(hierarchy, ImGuiDir_Down, 0.60f, nullptr, &hierarchy);

                    ImGui::DockBuilderDockWindow("SceneView", scene);
                    ImGui::DockBuilderDockWindow("Scene Hierarchy", hierarchy);
                    ImGui::DockBuilderDockWindow("Inspector", inspector);
                    ImGui::DockBuilderDockWindow("Style", inspector);
                    ImGui::DockBuilderDockWindow("Dear ImGui Demo", inspector);

                    ImGui::DockBuilderFinish(_sceneID);
                }

                ImGui::DockSpace(_sceneID, ImVec2(0.f, 0.f), ImGuiDockNodeFlags_None, &_sceneClass);
                return true;
            }

            // Preserve this workspace's layout while another tab is selected.
            if (ImGui::DockBuilderGetNode(_sceneID))
            {
                ImGui::DockSpace(_sceneID, ImVec2(0.f, 0.f), ImGuiDockNodeFlags_KeepAliveOnly, &_sceneClass);
            }

            return false;
        }

        void EndSceneEditor()
        {
            ImGui::End();
        }

        void PrepareBlender()
        {
            ImGui::SetNextWindowClass(&_workspaceClass);

            if (_focusBlender)
            {
                ImGui::SetNextWindowFocus();
                _focusBlender = false;
            }
        }
    };
}