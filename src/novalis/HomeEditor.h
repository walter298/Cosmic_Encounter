#ifndef HOME_EDITOR_H
#define HOME_EDITOR_H

#include "EditorUtil.h"

namespace nv {
	namespace editor {
		enum class EditorDest {
			None,
			Scene,
			Object,
			Text
		};

		void runHomeEditor(EditorDest& dest, bool& running);
	}
}

#endif