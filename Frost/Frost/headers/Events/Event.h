#pragma once
#include "FrostIncludes.h"

namespace Frost {

	// Events are currently blocking but with more time I would've buffered them 
	// through an event queue that gets processed whenever we want it to each frame

	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

#define BIT(x) (1 << x)

	enum EventCategory
	{
		None = 0,
		EventCategoryApplication = BIT(0),
		EventCategoryInput = BIT(1),
		EventCategoryKeyboard = BIT(2),
		EventCategoryMouse = BIT(3),
		EventCategoryMouseButton = BIT(4)
	};


#define EVENT_CLASS_TYPE(type)			static EventType GetStaticType() {return EventType::##type; }\
										virtual EventType GetEventType() {return GetStaticType(); }

#define EVENT_CLASS_CATEGORY(category)	virtual int GetCategoryFlags() const override {return category; }

	class Event
	{
		friend class EventDispatcher;
	
	public:
		virtual EventType GetEventType() const = 0;
		virtual int GetCategoryFlags() const = 0;

		inline bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}
	
	protected:
		bool m_Handled = false; // for future use when/if I implement a buffered event system with a queue
	};



	class EventDispatcher
	{
		template<typename T>
		using EventFn = std::function<bool(T&)>;
	public:
		EventDispatcher(Event& event)
			: m_Event(event)
		{
		}

		template<typename T>
		bool Dispatch(EventFn<T> func)
		{
			if (m_Event.GetEventType() == T::GetStaticType()) // T better not be anything other than an Event object grr
			{
				m_Event.m_Handled = func(*(T*)&m_Event);
				return true;
			}
			return false;
		}

	private:
		Event& m_Event;
	};
}