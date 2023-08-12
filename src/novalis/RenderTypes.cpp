#include "RenderTypes.h"

int nv::RenderObj::IDCount = 0;

nv::RenderObj::RenderObj() noexcept {
	IDCount++;
	m_ID = IDCount;
}

const std::string& nv::RenderObj::getName() const noexcept {
	return m_name;
}

int nv::RenderObj::getID() const noexcept {
	return m_ID;
}

int nv::RenderObj::getLayer() const noexcept {
	return m_layer;
}

nv::Texture::Texture(SDL_Texture* tex) noexcept 
	: raw(tex) {}

nv::Texture::~Texture() noexcept {
	SDL_DestroyTexture(raw);
}

nv::Sprite::Sprite(SDL_Renderer* renderer, std::string absPath) : RenderObj()
{
	FileData objData{ absPath };
	
	auto nameData = objData.getDataSection("NAME {");

	if (!nameData) {
		throw std::runtime_error("Error: no name specified for object in " + absPath);
	}
	if (nameData.value()->data().size() != 1) { //nameData is never empty
		throw std::runtime_error("Error: incorrect name formatting for object in " + absPath);
	}
	
	m_name = std::move(nameData.value()->data().front());

	auto spriteSheetData = objData.getDataSection("SPRITE_SHEET {");

	if (!spriteSheetData.has_value()) {
		throw std::runtime_error("Error: no sprite sheet data specified in " + absPath);
	}

	const auto& spriteSheetPaths = spriteSheetData.value()->data(); //spriteSheetData is never empty

	if (spriteSheetPaths.size() == 0) {
		throw std::runtime_error("Error: no image paths specified for sprite sheet in " + absPath);
	}

	for (const auto& path : spriteSheetPaths) {
		using namespace std::filesystem;
		m_spriteSheet.push_back(
			std::make_shared<Texture>(IMG_LoadTexture(renderer, (workingDirectory() + path).c_str())
			)
		);
	}

	auto parseRects = [](const std::vector<std::string>& lines) {
		std::vector<Rect> ret;
		ret.reserve(lines.size());
		for (const auto& line : lines) {
			int x, y, w, h;
			parseUnderscoredNums(line, x, y, w, h);
			ret.emplace_back(x, y, w, h);
		}
		return ret;
	};

	auto renderData = objData.getDataSection("SCREEN_RES {");

	if (!renderData.has_value()) {
		throw std::runtime_error("Error: no render data specified for obj in " + absPath);
	}
	if (renderData.value()->data().size() == 0) { //renderData is never empty
		throw std::runtime_error("Error: no render data in " + absPath);
	}

	for (const auto& sprite : m_spriteSheet) {
		if (sprite->raw == nullptr) {
			throw std::runtime_error("Error: sprite for " + absPath + " is nullptr\n");
		}
	}

	if (m_spriteSheet.size() >= 0) {
		m_currentSprite = m_spriteSheet[0]->raw;
	}
	m_layer = BACKGROUND_LAYER + 1;
}

nv::Background::Background(SDL_Renderer* renderer, std::string data) : RenderObj() {
	auto ampersand = std::ranges::find(data, '@');
	if (ampersand == data.end()) {
		throw std::runtime_error(R"(Error: no "@" found in )" + data);
	}
	std::string imgPath{ data.begin(), ampersand };
	println(imgPath);
	m_tex = std::make_shared<Texture>(IMG_LoadTexture(renderer, imgPath.c_str()));
	
	std::string numString{ ampersand + 1, data.end() };
	int x = 0, y = 0;
	parseUnderscoredNums(numString, x, y);

	m_ren = { x * NV_SCREEN_WIDTH, y * NV_SCREEN_HEIGHT, NV_SCREEN_WIDTH, NV_SCREEN_HEIGHT };
	m_layer = BACKGROUND_LAYER;
}

void nv::Background::render(SDL_Renderer* renderer) {
	SDL_RenderCopy(renderer, m_tex->raw, nullptr, &m_ren.rect);
}

void nv::Background::move(int dx, int dy) {
	m_ren.move(dx, dy);
}

void nv::Text::loadFonts() {
	std::string fontPath = workingDirectory() + "novalis_assets/fonts/";
	auto loadFont = [&fontPath](std::string name) {
		return TTF_OpenFont((fontPath + name).c_str(), 24);
	};
	fonts = {
		std::make_pair(FontType::Libertine, loadFont("Libertine.ttf")),
		std::make_pair(FontType::WorkSans, loadFont("WorkSans.ttf")),
	};
}

void nv::Text::closeFonts() {
	for (auto& [type, font] : fonts) {
		TTF_CloseFont(font);
	}
	fonts.clear();
}

void nv::Text::init(SDL_Renderer* renderer)
{
	if (m_tex->raw != nullptr) {
		SDL_DestroyTexture(m_tex->raw);
		m_tex->raw = nullptr;
	}

	if (m_text.size() > 0) {
		SDL_Surface* surface = TTF_RenderText_Solid_Wrapped(fonts.at(m_fontType), m_text.c_str(), m_color, m_wrapLength);
		m_tex->raw = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_FreeSurface(surface);
	}
}

nv::Text::Text(SDL_Renderer* renderer, std::string absPath) : RenderObj() {
	m_tex = std::make_shared<Texture>(nullptr);

	FileData data{ absPath };

	auto nameDataRes = data.getDataSection("NAME {");
	if (!nameDataRes) {
		throw std::runtime_error("Error: no message data specified.");
	}
	m_name = std::move(nameDataRes.value()->data().front());

	auto textDataRes = data.getDataSection("TEXT {");
	if (!textDataRes) {
		throw std::runtime_error("Error: no message data specified.");
	}
	m_text = std::move(textDataRes.value()->data()[0]);
	parseUnderscoredNums(textDataRes.value()->data()[1], m_color.r, m_color.g, m_color.b, m_color.a);
	parseUnderscoredNums(textDataRes.value()->data()[2], m_wrapLength);

	auto renderDataRes = data.getDataSection("RENDER_DIMENSIONS {");
	if (!renderDataRes) {
		throw std::runtime_error("Error: no render dimension data specified.");
	}
	parseUnderscoredNums(renderDataRes.value()->data()[0], m_ren.rect.w, m_ren.rect.h);

	auto backgroundDataRes = data.getDataSection("BACKGROUND {");
	if (!backgroundDataRes) {
		throw std::runtime_error("Error: no background data specified.");
	}
	parseUnderscoredNums(
		backgroundDataRes.value()->data()[0],
		m_background.rect.x, m_background.rect.y, 
		m_background.rect.w, m_background.rect.h
	);
	parseUnderscoredNums(
		backgroundDataRes.value()->data()[1],
		m_background.color.r, m_background.color.g, m_background.color.b, m_background.color.a
	);

	m_fontType = FontType::Libertine;

	m_layer = BACKGROUND_LAYER + 1;

	init(renderer);
}

nv::Text::Text(SDL_Renderer* renderer, std::string text, FontType fontType, SDL_Color color, Uint32 wrapLength) 
	: RenderObj(), 
	m_fontType(std::move(fontType)), 
	m_text(std::move(text)), 
	m_color(std::move(color)), 
	m_wrapLength(std::move(wrapLength))
{
	m_tex = std::make_shared<Texture>(nullptr);
	init(renderer);
}

void nv::Text::render(SDL_Renderer* renderer) noexcept {
	m_background.render(renderer);
	SDL_RenderCopy(renderer, m_tex->raw, nullptr, &m_ren.rect);
}

void nv::Text::setRenPos(int x, int y) {
	int dx = x - m_ren.rect.x;
	int dy = y - m_ren.rect.y;
	m_ren.setPos(x, y);
	m_background.move(dx, dy);
}

void nv::Text::setSize(int w, int h) {
	int dw = w - m_ren.rect.w;
	int dh = h - m_ren.rect.h;
	m_ren.setSize(w, h);
	m_background.scale(dw, dh);
}

void nv::Text::move(int dx, int dy) {
	m_background.move(dx, dy);
	m_ren.move(dx, dy);
}

void nv::Text::setText(SDL_Renderer* renderer, std::string text) {
	m_text = std::move(text);
	init(renderer);
}

void nv::Text::setFont(SDL_Renderer* renderer, FontType font) {
	m_fontType = std::move(font);
	init(renderer);
}

void nv::Text::setColor(SDL_Renderer* renderer, SDL_Color color) {
	m_color = std::move(color);
	init(renderer);
}

void nv::Text::setWrapLength(SDL_Renderer* renderer, Uint32 wrapLength) {
	m_wrapLength = std::move(wrapLength);
	init(renderer);
}