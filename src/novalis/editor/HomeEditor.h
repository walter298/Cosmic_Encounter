#ifndef HOME_EDITOR_H
#define HOME_EDITOR_H

#include "EditorUtil.h"
#include "../Instance.h"

namespace nv {
	namespace editor {
		class HomeEditor {
		private:
			int m_screenWidth;
			int m_screenHeight;
		public:
			HomeEditor(Instance& instance);
			EditorDest imguiRender() noexcept;
			void sdlRender() {}
		};
	}
}

#endif