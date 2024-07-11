#ifndef RENDERER_H
#define RENDERER_H

#include "DataUtil.h"

namespace nv {
	template<RenderObject... Objects>
	void renderCopy(const Layers<Objects>&... objLayers) {
		auto renderImpl = [&](const auto& layers) {
			for (const auto& [layer, objLayer] : layers) {
				for (const auto& obj : objLayer) {
					obj.render();
				}
			}
		};
		((renderImpl(objLayers)), ...);
	}

	template<RenderObject... Objects>
	void cameraMove(int dx, int dy, const Layers<Objects>&... objLayers) {
		auto renderImpl = [&](const auto& layers) {
			for (const auto& [layer, objLayer] : layers) {
				for (const auto& obj : objLayer) {
					obj.move(dx, dy);
				}
			}
		};
		((renderImpl(objLayers)), ...);
	}
}

#endif