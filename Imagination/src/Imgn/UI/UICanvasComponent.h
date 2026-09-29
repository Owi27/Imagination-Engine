#pragma once
#include "Imgn/ImgnComponent.h"
#include "Imgn/UI/UICanvas.h"

#include <fstream>
#include <vector>

namespace Imgn
{
	class IMGN_API UICanvasComponent : public Component
	{
		IMGN_COMPONENT_ID("Imgn.UICanvasComponent");

		unique<UICanvas> _canvas;
		std::vector<UIWidgetRecord> _children;

		void WriteString(std::fstream& pStream, const std::string& pValue) const;
		std::string ReadString(std::fstream& pStream) const;
		void RebuildChildren();

		void OnDestroy() override;
		void Dream(float pDeltaTime) override;

	public:
		UICanvasComponent() : Component("UICanvas")
		{
			_canvas = Unique<UICanvas>();
		}

		UICanvas* GetCanvas() const { return _canvas.get(); }

		void Serialize(std::fstream& pStream) override;
		void Deserialize(std::fstream& pStream) override;
	};
}
