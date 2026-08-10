#pragma once
#include "Imgn/ImgnLayer.h"

namespace Imgn
{
	class IMGN_API LayerStack
	{
		int _insertIdx = 0;
		std::vector<unique<Layer>> _layers;

	public:
		LayerStack()
		{
		}

		~LayerStack()
		{
		}

		// Cannot copy unique_ptr ownership
		LayerStack(const LayerStack&) = delete;
		LayerStack& operator=(const LayerStack&) = delete;

		// Can transfer ownership
		LayerStack(LayerStack&&) noexcept = default;
		LayerStack& operator=(LayerStack&&) noexcept = default;

		void AddLayer(unique<Layer> pLayer);
		void AddOverlay(unique<Layer> pLayer);
		void RemoveLayer(Layer* pLayer);
		void RemoveOverlay(Layer* pLayer);

		std::vector<unique<Layer>>::iterator begin() { return _layers.begin(); };
		std::vector<unique<Layer>>::iterator end() { return _layers.end(); };
	};
}