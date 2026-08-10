#include "pch.hpp"
#include "ImgnLayerStack.h"

namespace Imgn
{
	void LayerStack::AddLayer(unique<Layer> pLayer)
	{
		_layers.emplace(_layers.begin() + _insertIdx++, std::move(pLayer));
	}

	void LayerStack::AddOverlay(unique<Layer> pLayer)
	{
		_layers.emplace_back(std::move(pLayer));
	}

	void LayerStack::RemoveLayer(Layer* pLayer)
	{
		std::vector<unique<Layer>>::iterator iter = std::find_if(_layers.begin(), _layers.end(), [pLayer](const unique<Layer>& layer){ return layer.get() == pLayer; });
		if (iter != _layers.end())
		{
			_layers.erase(iter);
			_insertIdx--;
		}
	}

	void LayerStack::RemoveOverlay(Layer* pLayer)
	{
		std::vector<unique<Layer>>::iterator iter = std::find_if(_layers.begin(), _layers.end(), [pLayer](const unique<Layer>& layer){ return layer.get() == pLayer; });
		if (iter != _layers.end()) _layers.erase(iter);
	}
}