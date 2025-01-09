#pragma once

#include <string_view>
#include <functional>
#include "GraphicsEngine/GEng_API.h"

namespace GEng {
    class Event;
    class EventDispatcher;

    GENG_API enum class EventType {
        None = 0,
        WindowClose,
        WindowResize,
        WindowFocus,
        WindowMoved,
        AppTick,
        AppUpdate,
        AppRender,
        KeyPressed,
        KeyReleased,
        KeyTyped,
        MouseButtonPressed,
        MouseButtonReleased,
        MouseMove,
        MouseWheelScroll
    };

#define BIT(nr) (1u << (nr))

    enum EventCategory {
        None = 0,
        EventCategoryApplication = BIT(0),
        EventCategoryInput = BIT(1),
        EventCategoryKeyboard = BIT(2),
        EventCategoryMouse = BIT(3),
        EventCategoryMouseButton = BIT(4),
        EventCategoryWindow = BIT(5)
    };

#define EVENT_CLASS_FUNCTIONS(type)                                             \
	 static EventType GetStaticType() { return EventType ::type; }               \
	 virtual EventType GetEventType() const override { return GetStaticType(); } \
	 virtual std::string_view GetName() const override { return std::string_view(#type); }

#define EVENT_CLASS_CATEGORY_FNS(cat) \
	virtual int GetCategoryFlags() const override { return cat; }

    class GENG_API  Event {
        friend class GENG_API  EventDispatcher;

    public:
        [[nodiscard]] virtual EventType GetEventType() const = 0;
        [[nodiscard]] virtual std::string_view GetName() const = 0;
        [[nodiscard]] virtual int GetCategoryFlags() const = 0;

        bool IsCategory(EventCategory cat) { return GetCategoryFlags() & cat; }
        void Handle() { m_Handled = true; }
        bool IsHandled() const { return m_Handled; }

    protected:
        bool m_Handled = false;
    };

    class GENG_API  EventDispatcher {
    private:
        Event &m_Event;

    public:
        template<typename T>
        using EventFn = std::function<bool(T &)>;

        explicit EventDispatcher(Event &event) : m_Event(event) {
        }

        template<typename T>
        bool Dispatch(EventFn<T> func) {
            if (m_Event.GetEventType() == T::GetStaticType() && !m_Event.m_Handled) {
                m_Event.m_Handled = func((T &) m_Event);
                return true;
            }
            return false;
        }

        const Event &GetEvent() const { return m_Event; }
    };
}
