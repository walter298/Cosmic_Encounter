#ifndef HOME_EDITOR_H
#define HOME_EDITOR_H

#include "EditorUtil.h"

namespace nv {
	namespace editor {
		struct HomeEditor {
			EditorDest imguiRender() noexcept;
			void sdlRender() {}
		};
	}
}

#endif