#pragma once

#include <glm/glm.hpp>
#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Events/Events.h"


namespace DH
{
	// Size vector of dimension 2
	using svec2 = glm::vec<2, size_t>;

	DH_API enum class MouseButton : int
	{
		None = -1,
		Right,
		Centre,
		Left
	};

	class DH_API  MouseButtonPressedEvent : public Event
	{
	public:
		MouseButtonPressedEvent(MouseButton buttonIn) : button(buttonIn) {}

		EVENT_CLASS_FUNCTIONS(MouseButtonPressed)
		EVENT_CLASS_CATEGORY_FNS(EventCategory::EventCategoryInput |
								 EventCategory::EventCategoryMouse |
								 EventCategory::EventCategoryMouseButton)

		MouseButton GetMouseButton() const { return button; }

	protected:
		MouseButton button;
	};

	class DH_API  MouseButtonReleasedEvent : public Event
	{
	public:
		MouseButtonReleasedEvent(MouseButton buttonIn) : button(buttonIn) {}

		EVENT_CLASS_FUNCTIONS(MouseButtonReleased)
		EVENT_CLASS_CATEGORY_FNS(EventCategory::EventCategoryInput |
								 EventCategory::EventCategoryMouse |
								 EventCategory::EventCategoryMouseButton)

		MouseButton GetMouseButton() const { return button; }

	protected:
		MouseButton button;
	};

	class DH_API  MouseMoveEvent : public Event
	{
	public:
		MouseMoveEvent(glm::dvec2 pos) : mMousePos(pos) {}

		EVENT_CLASS_FUNCTIONS(MouseMove)
		EVENT_CLASS_CATEGORY_FNS(EventCategory::EventCategoryInput | EventCategory::EventCategoryMouse)

		glm::dvec2 GetMousePos() const { return mMousePos; }

	protected:
		glm::dvec2 mMousePos;
	};

	class DH_API  MouseScrollEvent : public Event
	{
	public:
		MouseScrollEvent(float xOffIn, float yOffIn) : xOffset(xOffIn), yOffset(yOffIn) {}

		EVENT_CLASS_FUNCTIONS(MouseWheelScroll)
		EVENT_CLASS_CATEGORY_FNS(EventCategory::EventCategoryInput | EventCategory::EventCategoryMouse)

		[[nodiscard]] float GetXOffset() const { return xOffset; }
		[[nodiscard]] float GetYOffset() const { return yOffset; }

	protected:
		float xOffset = 0.0f;
		float yOffset = 0.0f;
	};

}