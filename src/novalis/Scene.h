#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <type_traits>
#include <algorithm>
#include <ranges>
#include <memory>

#include "Event.h"
#include "Instance.h"
#include "Button.h"
#include "GlobalMacros.h"
#include "RenderTypes.h"

namespace nv {
	class Scene {
	public:
		enum class EndReason {
			Quit,
			NextScene
		};
	private:
		EndReason m_endReason;

		std::vector<std::function<void()>> m_objectPositionSetters;

		std::vector<RenderObjPtr> m_owningObjects; //only the objects specific to the scene
		std::vector<RenderObj*> m_objects; //all the objects in the scene

		std::vector<Event> m_events;
		std::vector<Button*> m_buttons;

		bool m_running = false;

		std::vector<RectPtr> m_collisionBoxes;

		void setObjectPositions();
	protected:
		Renderer& m_renderer;

		void endScene(EndReason end) noexcept;
	public:
		Scene(std::string path, NovalisInstance& instance);
		Scene() = default;

		EndReason endReason() const noexcept;

		void addObj(RenderObjPtr obj);
		void addObj(RenderObj* obj);
		
		void addEvent(Event evt);
		void cancelEvent(int ID);

		void addButton(Button* btn);
		void removeButton(int ID);

		void camMove(int dx, int dy);

		void execute();
	};
}

#endif