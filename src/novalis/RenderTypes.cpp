#include "RenderTypes.h"

const std::string& nv::RenderObj::getName() const noexcept {
	return m_name;
}

nv::ID<nv::RenderObj> nv::RenderObj::getID() const noexcept {
	return m_ID;
}

nv::Texture::Texture(SDL_Texture* tex) noexcept 
	: raw(tex) {}

nv::Texture::~Texture() noexcept {
	SDL_DestroyTexture(raw);
}

nv::Sprite::Sprite(SDL_Renderer* renderer, std::string absFilePath)
{
	std::ifstream spriteFile{ absFilePath };
	assert(spriteFile.is_open());

	auto jsonData = json::parse(spriteFile);

	//load each image as a texture
	auto imgPaths = jsonData.at("image_paths").get<std::vector<std::string>>();
	m_spriteSheet.reserve(imgPaths.size());
	for (const auto& path : imgPaths) {
		m_spriteSheet.push_back(std::make_shared<Texture>(IMG_LoadTexture(renderer, path.c_str())));
	}
	assert(m_spriteSheet.size() > 0);
	m_currentSprite = m_spriteSheet[0]->raw;

	m_ren = jsonData.at("ren").get<Rect>();
	//m_world = jsonData.at("world").get<Rect>();
}

void nv::Sprite::render(SDL_Renderer* renderer) noexcept {
	SDL_RenderCopy(renderer, m_currentSprite, nullptr, &m_ren.rect);
}

void nv::Sprite::move(int dx, int dy) noexcept {
	renMove(dx, dy);
	worldMove(dx, dy);
}

nv::Background::Background(SDL_Renderer* renderer, std::string absFilePath) {
	std::ifstream file{ absFilePath };
	auto jsonData = json::parse(file);
	m_name = jsonData.at("name").get<std::string>();
	m_rens = jsonData.at("rens").get<std::vector<Rect>>();
	
	//load textures
	auto imagePaths = jsonData.at("image_paths").get<std::vector<std::string>>();
	for (const auto& path : imagePaths) {
		m_backgrounds.push_back(std::make_shared<Texture>(IMG_LoadTexture(renderer, path.c_str())));
	}
}

void nv::Background::render(SDL_Renderer* renderer) noexcept {
	for (auto [background, ren] : std::ranges::zip_view(m_backgrounds, m_rens)) {
		SDL_RenderCopy(renderer, background->raw, nullptr, &ren.rect);
	}
}

void nv::Background::renMove(int dx, int dy) noexcept {
	for (auto& ren : m_rens) {
		ren.move(dx, dy);
	}
}

void nv::Text::loadFonts() {
	std::string fontPath = workingDirectory() + "novalis_assets/fonts/";
	auto loadFont = [&fontPath](std::string name) {
		return TTF_OpenFont((fontPath + name).c_str(), 24);
	};
	fonts = {
		std::pair{ FontType::Libertine, loadFont("Libertine.ttf") },
		std::pair{ FontType::WorkSans, loadFont("WorkSans.ttf") }
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

	std::cout << m_text << std::endl;
	println(m_ren.rect.x, m_ren.rect.y, m_ren.rect.w, m_ren.rect.h);

	if (m_text.size() > 0) {
		SDL_Surface* surface = TTF_RenderText_Solid_Wrapped(fonts.at(m_fontType), m_text.c_str(), m_color, m_wrapLength);
		m_tex->raw = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_FreeSurface(surface);
	}
}

nv::Text::Text(SDL_Renderer* renderer, std::string absPath) {
	m_tex = std::make_shared<Texture>(nullptr);

	std::ifstream textFile{ absPath };
	assert(textFile.is_open());

	auto jsonData = json::parse(textFile);

	textFile.close();

	m_name       = jsonData.at("name").get<std::string>();
	m_text       = jsonData.at("text").get<std::string>();
	m_color      = jsonData.at("color").get<SDL_Color>();
	m_wrapLength = jsonData.at("wrap_length").get<Uint32>();
	m_background = jsonData.at("background").get<Rect>();
	m_ren		 = jsonData.at("ren").get<Rect>();

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