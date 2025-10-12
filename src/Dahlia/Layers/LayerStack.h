#pragma once

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Layers/Layer.h"

namespace DH
{
	class DH_API  LayerStack
	{
	public:
		using LayerVec = std::vector<Layer *>;

		LayerStack();
		~LayerStack();

		void PushLayer(Layer *layer);
		void PushOverlay(Layer *overlay);
		void PopLayer(Layer *layer);
		void PopOverlay(Layer *overlay);

		LayerVec::reverse_iterator rbegin() { return m_Layers.rbegin(); }
		LayerVec::reverse_iterator rend() { return m_Layers.rend(); }
		LayerVec::iterator begin() { return m_Layers.begin(); }
		LayerVec::iterator end() { return m_Layers.end(); }

	private:
		LayerVec m_Layers;
		size_t m_LayerInsertIndex;
	};
}
