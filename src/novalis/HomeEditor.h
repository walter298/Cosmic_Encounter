#ifndef HOME_EDITOR_H
#define HOME_EDITOR_H

#include "EditorUtil.h"

namespace nv {
	namespace editor {
		class HomeEditor : public Editor {
		private:
			EditorDest m_dest = EditorDest::None;
		public:
			HomeEditor(ImGuiIO& io, SDL_Renderer* renderer);
			EditorDest getDestination() const noexcept;
			void showGui(bool& running) override;
		};

		void runHomeEditor(EditorDest& dest, bool& running);
	}
}

#endif