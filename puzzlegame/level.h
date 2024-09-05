#ifndef LEVEL_H_
#define LEVEL_H_

#include <map>
#include <string>
#include <vector>

enum class Direction { Left, Right, Up, Down };

class Level {
public:

	Level(const std::string& data);

	void set_title(const std::string& title);

	void print() const;

	const std::vector<std::vector<char>>& get_board() const;
	const int& get_steps() const;
	const bool& check_squares_pos(std::pair<int, int> pos) const;
	const bool& check_walls_pos(std::pair<int, int> pos) const;
	const bool& check_boxes_pos(std::pair<int, int> pos) const;
	const bool& check_player_pos(std::pair<int, int> pos) const;
	const std::string& get_title() const;
	const std::pair<int, int>& get_level_dimensions() const;
	const std::pair<int, int>& get_next_position(Direction dir) const;
	const std::pair<int, int>& box_next_position(Direction dir, std::pair<int, int> next_pos) const;
	const Direction& get_prev_dir() const;
	Level clone() const;

	void step(Direction dir);

	bool is_completed() const;
private:
	Direction prev_dir;
	std::vector<std::vector<char>> board;
	int steps;
	std::string title;

	std::pair<int, int> level_dimensions;

	std::vector<std::pair<int, int>> walls;
	std::pair<int, int> player;
	std::vector<std::pair<int, int>> squares;
	std::vector<std::pair<int, int>> boxes;


	void parse_data(const std::string& data);
};

#endif