#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <windows.h>
#include <comutil.h>
#include <sstream>
#include "level.h"
#include "cmdparser.h"
#include <msxml6.h>
#import <msxml6.dll>
#include <chrono>
#include <thread>
#include "tile.h"
#include "renderer.h"
#include <future>
#include <objbase.h>
#include <iterator>
#include <optional>


using namespace sda;
using namespace sda::utils;


bool isFileEmpty(const std::string& filename);
std::vector<Level> load_slc_file(const std::string& path);
std::string BSTRToString(BSTR bstr);
std::vector<Level> ParseLevels(MSXML2::IXMLDOMNodePtr pLevelCollection);
SDL_Window* create_window(int width, int height, bool fullscreen);
Tile load_tileset(SDL_Renderer* renderer, const std::string& path, int width, int height, int effective_height, int offset, int png_width, int png_height, int screen_width, int screen_height);
TTF_Font* load_font(const std::string& path, int size);

template <typename Iterator>
std::optional<typename Iterator::value_type> next_level(Iterator& it, const Iterator& end);


int main(int argc, char** argv)
{
	CmdParser parser;

	parser.addSwitch("--input", "input xml level file", "", false);
	parser.addSwitch("--width", "screen width", "1920");
	parser.addSwitch("--height", "screen height", "1080");
	parser.addSwitch("--fullscreen", "fullscreen mode", "false", false);
	parser.parse(argc, argv);
	std::string inputFile = parser.value("input");
	int width = parser.value_to_int("width");
	int height = parser.value_to_int("height");
	bool fullscreen = parser.value_to_bool("fullscreen");
	if (inputFile.size() == 0) {
		std::cout << "error: input xml level file must be provided" << std::endl;
		return -1;
	}

	bool empty = isFileEmpty(inputFile);
	std::vector<Level> levels = load_slc_file(inputFile);


	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
		std::cerr << "IMG_Init Error: " << IMG_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}
	if (TTF_Init() != 0) {
		std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
		IMG_Quit();
		SDL_Quit();
		return 1;
	}

	try {
		SDL_Window* window = create_window(width, height, fullscreen);

		SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		if (!renderer) {
			std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
			SDL_DestroyWindow(window);
			SDL_Quit();
			return 1;
		}


		SDL_Texture* texture = SDL_CreateTexture(renderer,
			SDL_PIXELFORMAT_RGBA8888,
			SDL_TEXTUREACCESS_TARGET,
			width,
			height);
		if (!texture) {
			std::cerr << "Texture could not be created! SDL_Error: " << SDL_GetError() << std::endl;
			SDL_DestroyRenderer(renderer);
			SDL_DestroyWindow(window);
			SDL_Quit();
			return 1;
		}

		try {

			Tile big_set = load_tileset(renderer, "Tilesheet/sokoban_tilesheet@2.png", 128, 128, 80, 48, 1664, 1024, width, height);
			Tile small_set = load_tileset(renderer, "Tilesheet/sokoban_tilesheet.png", 64, 64, 40, 24, 832, 512, width, height);
			TTF_Font* font = load_font("roboto-regular.ttf", 20);

			Renderer render(renderer, big_set, small_set, font);

			auto levels_it = levels.begin();
			auto levels_end = levels.end();

			std::optional<Level> optional_level = next_level(levels_it, levels_end);
			if (optional_level) {
				Level reference_level = *optional_level;
				Level level = reference_level.clone();
				bool running = true;
				bool skip = false;

				SDL_Event event;
				while (running) {
					if (level.is_completed() || skip) {
						optional_level = next_level(levels_it, levels_end);
						if (optional_level) {
							reference_level = *optional_level;
							level = reference_level.clone();
							skip = false;
						}
						else {
							break;
						}
					}

					SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
					SDL_RenderClear(renderer);
					render.render(renderer, &level);
					SDL_RenderPresent(renderer);

					while (SDL_PollEvent(&event)) {
						switch (event.type) {
						case SDL_QUIT:
							running = false;
							break;
						case SDL_KEYDOWN:
							switch (event.key.keysym.sym) {
							case SDLK_ESCAPE:
								running = false;
								break;
							case SDLK_LEFT:
								level.step(Direction::Left);
								break;
							case SDLK_RIGHT:
								level.step(Direction::Right);
								break;
							case SDLK_UP:
								level.step(Direction::Up);
								break;
							case SDLK_DOWN:
								level.step(Direction::Down);
								break;
							case SDLK_r:
								level = reference_level.clone();
								break;
							case SDLK_n:
								skip = true;
								break;
							}
							break;
						default:
							break;
						}
					}
				}
			}

			TTF_CloseFont(font);
		}
		catch (const std::exception& e) {
			std::cerr << e.what() << std::endl;
		}

		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);

	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		TTF_Quit();
		IMG_Quit();
		SDL_Quit();
		return 1;
	}

	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	return 0;
}

std::vector<Level> load_slc_file(const std::string& path) {
	std::vector<Level> levels;


	MSXML2::IXMLDOMDocument2Ptr doc;
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to initialize COM");
	}
	hr = doc.CreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER);

	if (FAILED(hr)) {
		std::cerr << "FAILED(hr)" << hr << std::endl;
	}
	doc->PutpreserveWhiteSpace(VARIANT_TRUE);

	VARIANT_BOOL success = VARIANT_FALSE;

	VARIANT filename;

	filename.vt = VT_BSTR;

	std::cout << path.c_str() << std::endl;

	filename.bstrVal = _bstr_t(path.c_str());

	VARIANT_BOOL load_success = doc->load(filename);
	if (load_success != VARIANT_TRUE) {

		_com_error error(hr);
		std::cerr << "COM Error during SLC file loading: " << error.ErrorMessage() << std::endl;
		std::cout << success << std::endl;
		std::cerr << "FAILED(hr) = 0x" << std::hex << hr << std::endl;
		std::cerr << "error" << std::endl;
		std::cerr << "Failed to load XML" << std::endl;
		MSXML2::IXMLDOMParseErrorPtr pError = doc->parseError;
		std::wcerr << L"Error: " << pError->reason << std::endl;
		pError.Release();
	}


	MSXML2::IXMLDOMNodePtr root = doc->documentElement;
	if (!root) {
		std::cerr << "No root element found" << std::endl;

	}

	std::string level_data;
	std::string level_title;
	bool reading_level = false;

	MSXML2::IXMLDOMNodePtr pLevelCollection;
	MSXML2::IXMLDOMNodeListPtr pChildNodes = root->selectNodes(_bstr_t(L"LevelCollection"));
	pLevelCollection = pChildNodes->item[0];

	levels = ParseLevels(pLevelCollection);
	pChildNodes.Release();
	pLevelCollection.Release();
	root.Release();
	doc.Release();
	CoUninitialize();
	return levels;
}

bool isFileEmpty(const std::string& filename) {
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		std::cerr << "Error opening file: " << filename << std::endl;
		return false;
	}
	file.seekg(0, file.end);
	int length = file.tellg();
	file.seekg(0, file.beg);
	file.close();
	return  0;
}

std::string BSTRToString(BSTR bstr) {

	if (!bstr) return "";

	int len = WideCharToMultiByte(CP_UTF8, 0, bstr, -1, NULL, 0, NULL, NULL);

	std::string str(len, 0);
	WideCharToMultiByte(CP_UTF8, 0, bstr, -1, &str[0], len, NULL, NULL);

	return str;
}

std::vector<Level> ParseLevels(MSXML2::IXMLDOMNodePtr pLevelCollection) {
	std::vector<Level> levels;

	if (!pLevelCollection) return levels;

	MSXML2::IXMLDOMNodeListPtr pLevelNodes = pLevelCollection->selectNodes(_bstr_t(L"Level"));
	if (!pLevelNodes) return levels;

	long length;
	pLevelNodes->get_length(&length);
	for (long i = 0; i < length; ++i) {
		MSXML2::IXMLDOMNodePtr pLevelNode = pLevelNodes->item[i];
		if (!pLevelNode) continue;

		std::string levelData;
		std::string levelTitle;

		MSXML2::IXMLDOMNamedNodeMapPtr pAttributes = pLevelNode->attributes;
		if (pAttributes) {
			MSXML2::IXMLDOMNodePtr pIdAttr = pAttributes->getNamedItem(_bstr_t(L"Id"));
			if (pIdAttr) {
				BSTR bstrId;
				pIdAttr->get_text(&bstrId);
				levelTitle = BSTRToString(bstrId);
				SysFreeString(bstrId);
			}
			pIdAttr.Release();
		}

		MSXML2::IXMLDOMNodeListPtr pChildNodes = pLevelNode->childNodes;
		if (pChildNodes) {
			long childLength;
			pChildNodes->get_length(&childLength);
			for (long j = 0; j < childLength; ++j) {
				MSXML2::IXMLDOMNodePtr pChildNode = pChildNodes->item[j];
				if (pChildNode && pChildNode->nodeType == NODE_ELEMENT) {
					BSTR bstrText;
					pChildNode->get_text(&bstrText);
					levelData += BSTRToString(bstrText);
					levelData += '\n';
					SysFreeString(bstrText);
				}
				pChildNode.Release();
			}
		}


		Level level(levelData);
		level.set_title(levelTitle);
		levels.push_back(level);
		pChildNodes.Release();
		pAttributes.Release();
		pLevelNode.Release();
	}

	pLevelNodes.Release();
	return levels;
}

SDL_Window* create_window(int width, int height, bool fullscreen) {

	Uint32 window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

	SDL_Window* window = SDL_CreateWindow("puzzleGame-cpp",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width,
		height,
		window_flags);

	if (window == nullptr) {
		std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		throw std::runtime_error("Failed to create SDL window");
	}
	if (fullscreen) {
		if (SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN) < 0) {
			std::cerr << "Unable to set fullscreen mode! SDL_Error: " << SDL_GetError() << std::endl;

			SDL_DestroyWindow(window);
			SDL_Quit();
			throw std::runtime_error("Failed to set fullscreen mode");
		}
	}

	return window;
}

Tile load_tileset(SDL_Renderer* renderer, const std::string& path, int width, int height, int effective_height, int offset, int png_width, int png_height, int screen_width, int screen_height) {
	SDL_Surface* surface = IMG_Load(path.c_str());
	if (!surface) {
		throw std::runtime_error("Failed to load image: " + path + ", SDL_Error: " + std::string(SDL_GetError()));
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);

	if (!texture) {
		throw std::runtime_error("Failed to create texture from surface: SDL_Error: " + std::string(SDL_GetError()));
	}

	Tile tileset(texture, width, height, effective_height, offset, png_width, png_height, screen_width, screen_height);
	return tileset;
}

TTF_Font* load_font(const std::string& path, int size) {
	TTF_Font* font = TTF_OpenFont(path.c_str(), size);
	if (!font) {
		throw std::runtime_error("Failed to load font: " + path + ", SDL_ttf Error: " + TTF_GetError());
	}
	return font;
}

template <typename Iterator>
std::optional<typename Iterator::value_type> next_level(Iterator& it, const Iterator& end) {
	if (it != end) {
		return *(it++);
	}
	else {
		return std::nullopt;
	}
}
