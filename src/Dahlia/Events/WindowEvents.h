#pragma once

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Events/Events.h"
#include <sstream>
#include <glm/glm.hpp>

namespace DH
{
	// Size vector of dimension 2
	using svec2 = glm::vec<2, size_t>;

	class DH_API  WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(size_t widthIn, size_t heightIn) : width(widthIn),
															 height(heightIn) {}

		EVENT_CLASS_FUNCTIONS(WindowResize)
		EVENT_CLASS_CATEGORY_FNS(EventCategory::EventCategoryWindow)

		size_t GetWidth() const { return width; }
		size_t GetHeight() const { return height; }
		svec2 GetDimensions() const { return {width, height}; }

	protected:
		size_t width = 0, height = 0;
	};

	class DH_API  WindowMovedEvent : public Event
	{
	public:
		WindowMovedEvent(svec2 newPos) : winPos(newPos) {}

		EVENT_CLASS_FUNCTIONS(WindowMoved)
		EVENT_CLASS_CATEGORY_FNS(EventCategory::EventCategoryWindow)

		svec2 GetPosition() const { return winPos; }

	protected:
		svec2 winPos = {0, 0};
	};

	class DH_API  WindowFocusEvent : public Event
	{
	public:
		WindowFocusEvent(bool lostFocusIn) : lostFocus(lostFocusIn) {}

		EVENT_CLASS_FUNCTIONS(WindowFocus)
		EVENT_CLASS_CATEGORY_FNS(EventCategory::EventCategoryWindow)

		bool DidLoseFocus() const { return lostFocus; }

	protected:
		bool lostFocus = false;
	};

	class DH_API  WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() {}
		EVENT_CLASS_FUNCTIONS(WindowClose)
		EVENT_CLASS_CATEGORY_FNS(EventCategory::EventCategoryWindow)
	};
}
