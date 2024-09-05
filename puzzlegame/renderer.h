#ifndef RENDERER_H_
#define RENDERER_H_

#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <SDL.h>
#include "tile.h"
#include <SDL_ttf.h>
#include <optional>
#include "cmdparser.h"

class Renderer {
public:
	Renderer(SDL_Renderer* renderer, const Tile& big_set, const Tile& small_set, TTF_Font* font)
		: renderer(renderer), big_set(big_set), small_set(small_set), font(font) {}


	void draw_text(const std::string& text, int x, int y, SDL_Color color, int text_width, int text_height) {
		SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
		if (!surface) {
			std::cerr << "Failed to create surface: " << TTF_GetError() << std::endl;
			return;
		}

		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_FreeSurface(surface);
		if (!texture) {
			std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
			return;
		}
		SDL_QueryTexture(texture, nullptr, nullptr, &text_width, &text_height);
		SDL_Rect dst_rect = { x, y, text_width, text_height };
		SDL_RenderCopy(renderer, texture, nullptr, &dst_rect);
		SDL_DestroyTexture(texture);
	}

	void draw_tile(const Tile& tileset, int col, int row, int x, int y) {
		SDL_Rect src_rect = tileset.get_tile_rect(col, row);
		SDL_Rect dst_rect = { x, y, src_rect.w, src_rect.h };
		SDL_RenderCopy(renderer, tileset.get_texture(), &src_rect, &dst_rect);
	}

	void render(SDL_Renderer* renderer, const Level* level) {
		level_dimensions = level->get_level_dimensions();

		std::optional<Tile> current_tile = ((((level_dimensions.first) > (level_dimensions.second)) ? (level_dimensions.first) : (level_dimensions.second)) > 15)
			? small_set
			: big_set;

		std::pair<int, int> render_size_level = rendering_size(level_dimensions, *current_tile);
		render_screen(renderer, level, *current_tile);

		int screen_width = current_tile->get_screen_width();
		int screen_height = current_tile->get_screen_height();

		double width_ratio = static_cast<double>(screen_width) / static_cast<double>(render_size_level.first);
		int h = screen_height - 32;
		double height_ratio = static_cast<double>(h) / static_cast<double>(render_size_level.second);
		double ratio = (width_ratio < height_ratio) ? ((width_ratio < 1.0) ? width_ratio : 1.0) : ((height_ratio < 1.0) ? height_ratio : 1.0);
		auto scale = [ratio](int sz) -> int {
			return static_cast<int>(std::floor(ratio * static_cast<double>(sz)));
			};
		std::pair<int, int> scaled_render_size = std::make_pair(scale(render_size_level.first), scale(render_size_level.second));

		int x = (screen_width - scaled_render_size.first) / 2;
		int y = (screen_height - 32 - scaled_render_size.second) / 2;
		SDL_Rect rect = { x, y, scaled_render_size.first, scaled_render_size.second };
		status_bar(renderer, level, *current_tile);
		return;
	}
	void render_screen(SDL_Renderer* renderer, const Level* level, Tile current_tile) {

		for (int i = 0; i < level_dimensions.second; i++) {
			for (int j = 0; j < level_dimensions.first; j++) {
				std::pair<int, int > pos = std::make_pair(j, i);
				auto coordinates = current_tile.get_coordinates(pos);
				if (level->check_squares_pos(pos)) {
					draw_tile(current_tile, 11, 0, coordinates.first, coordinates.second);
				}
				else {
					draw_tile(current_tile, 10, 0, coordinates.first, coordinates.second);
				}
				int offset = coordinates.second - current_tile.get_offset();
				if (level->check_walls_pos(pos)) {
					draw_tile(current_tile, 7, 7, coordinates.first, offset);
				}
				if (level->check_boxes_pos(pos)) {
					if (level->check_squares_pos(pos)) {
						draw_tile(current_tile, 6, 4, coordinates.first, offset);
					}
					else {
						draw_tile(current_tile, 6, 0, coordinates.first, offset);
					}
				}
				if (level->check_player_pos(pos)) {
					if (Direction::Left == level->get_prev_dir()) {
						draw_tile(current_tile, 3, 6, coordinates.first, offset);
					}
					else if (Direction::Right == level->get_prev_dir()) {
						draw_tile(current_tile, 0, 6, coordinates.first, offset);
					}
					else if (Direction::Up == level->get_prev_dir()) {
						draw_tile(current_tile, 3, 4, coordinates.first, offset);
					}
					else if (Direction::Down == level->get_prev_dir()) {
						draw_tile(current_tile, 0, 4, coordinates.first, offset);
					}
				}
			}
		}
	}
	std::pair<int, int> rendering_size(std::pair<int, int> t, Tile current_tile) {
		int width = t.first * current_tile.get_width();
		int height = ((t.second > 0) ? current_tile.get_height() + (t.second - 1) * current_tile.get_effective_height() : 0);
		return std::make_pair(width, height);
	}

	void status_bar(SDL_Renderer* renderer, const Level* level, Tile current_tile) {
		int steps = level->get_steps();
		std::string moves = "amount of moves: " + std::to_string(steps) + "     level: " + level->get_title();

		SDL_Color black = { 0, 0, 0, 255 };
		draw_text(moves, 0, (current_tile.get_screen_height() - 32), black, current_tile.get_screen_width(), 32);



	}
private:
	SDL_Renderer* renderer;
	Tile big_set;
	Tile small_set;
	std::pair<int, int> level_dimensions;
	TTF_Font* font;
};

#endif