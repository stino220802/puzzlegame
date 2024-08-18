#ifndef TILE_H_
#define TILE_H_

#include <map>
#include <string>
#include <vector>
#include <SDL.h>
#include <stdexcept>
#include <iostream>
class Tile {

public:
	Tile(SDL_Texture* texture, int width, int height, int effective_height, int offset, int png_width, int png_height, int screen_width, int screen_height)
		: texture(texture), width(width), height(height), effective_height(effective_height), offset(offset), png_width(png_width), png_height(png_height), screen_width(screen_width), screen_height(screen_height) {}


	SDL_Texture* get_texture() const { return texture; }

	int get_width() const { return width; }

	int get_height() const { return height; }

	int get_effective_height() const { return effective_height; }

	int get_offset() const { return offset; }

	int get_png_width() const { return png_width; }

	int get_png_height() const { return png_height; }

	int get_screen_width() const { return screen_width; }

	int get_screen_height() const { return screen_height; }

	SDL_Rect get_tile_rect(int col, int row) const {

		int tiles_per_row = png_width / width;
		int tiles_per_col = png_height / height;

		int x = col * width;
		int y = row * height;

		SDL_Rect rect = { x, y, width, height };
		return rect;
	}

	std::pair<int, int> get_coordinates(std::pair<int, int> pos) {

		int x, y = 0;
		x = width * pos.first;
		y = effective_height * pos.second;
		return std::make_pair(x, y);
	}


private:
	SDL_Texture* texture;
	int width;
	int height;
	int effective_height;
	int offset;
	int png_width;
	int png_height;
	int screen_height;
	int screen_width;

};

#endif