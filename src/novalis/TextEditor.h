#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <array>

#include "EditorUtil.h"

namespace nv {
	namespace editor {
		void runTextEditor(Text& text, nv::Renderer& renderer, bool& running);
	}
}

#endif