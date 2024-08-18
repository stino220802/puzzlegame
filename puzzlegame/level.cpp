#include "level.h"
#include <sstream>
#include <algorithm>
#include <iostream>

Level::Level(const std::string& data) {
    parse_data(data);
}

void Level::set_title(const std::string& title) {
    this->title = title;
}

const std::pair<int, int>& Level::get_level_dimensions() const {
    return level_dimensions;
}

const Direction& Level::get_prev_dir() const {
    return prev_dir;
}

const std::vector<std::vector<char>>& Level::get_board() const {
    return board;
}

const int& Level::get_steps() const {
    return steps;
}

const std::string& Level::get_title() const {
    return title;
}

const bool& Level::check_squares_pos(std::pair<int, int> pos) const {
    for (auto& row : squares) {
        if (row == pos) {
            return true;
        }
    }
    return false;
}

const bool& Level::check_walls_pos(std::pair<int, int> pos) const {
    for (auto& row : walls) {
        if (row == pos) {
            return true;
        }
    }
    return false;
}

const bool& Level::check_boxes_pos(std::pair<int, int> pos) const {
    for (auto& row : boxes) {
        if (row == pos) {
            return true;
        }
    }
    return false;
}

const bool& Level::check_player_pos(std::pair<int, int> pos)const {
    if (player == pos) {
        return true;
    }
    return false;
}
void Level::print() const {
    std::cout << "Title: " << title << std::endl;
    for (const auto& row : board) {
        for (char cell : row) {
            std::cout << cell;
        }
        std::cout << std::endl;
    }
}

void Level::parse_data(const std::string& data) {
    std::istringstream stream(data);
    std::string line;
    board.clear();
    int width = 0;
    int height = 0;
    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        std::transform(line.begin(), line.end(), line.begin(), ::toupper);

        std::vector<char> row(line.begin(), line.end());
        board.push_back(row);

        width = std::max(width, static_cast<int32_t>(line.size()));
        height++;
    }

    width--;
    int countHeight = 0;
    for (const auto& row : board) {
        int countWidth = 0;
        for (char cell : row) {
            auto tempPair = std::make_pair(countWidth, countHeight);
            if (cell == '#') {
                walls.push_back(tempPair);
            }
            else if (cell == '.') {
                squares.push_back(tempPair);
            }
            else if (cell == '$') {
                boxes.push_back(tempPair);
            }
            else if (cell == '@') {
                player = tempPair;
            }
            else if (cell == '+') {
                player = tempPair;
                squares.push_back(tempPair);
            }
            else if (cell == '*') {
                boxes.push_back(tempPair);
                squares.push_back(tempPair);
            }
            countWidth++;
        }
        countHeight++;
    }
    level_dimensions = std::make_pair(width, height);
}

Level Level::clone() const {
    return *this;
}

bool Level::is_completed() const {
    return boxes.size() == squares.size() &&
        std::is_permutation(boxes.begin(), boxes.end(), squares.begin());
}
const std::pair<int, int>& Level::get_next_position(Direction dir) const {
    if (dir == Direction::Left) {
        return std::make_pair(player.first - 1, player.second);
    }
    else if (dir == Direction::Right) {
        return std::make_pair(player.first + 1, player.second);
    }
    else if (dir == Direction::Up) {
        return std::make_pair(player.first, player.second - 1);
    }
    else if (dir == Direction::Down) {
        return std::make_pair(player.first, player.second + 1);
    }
}

const std::pair<int, int>& Level::box_next_position(Direction dir, std::pair<int, int> next_pos) const {
    if (dir == Direction::Left) {
        return std::make_pair(next_pos.first - 1, next_pos.second);
    }
    else if (dir == Direction::Right) {
        return std::make_pair(next_pos.first + 1, next_pos.second);
    }
    else if (dir == Direction::Up) {
        return std::make_pair(next_pos.first, next_pos.second - 1);
    }
    else if (dir == Direction::Down) {
        return std::make_pair(next_pos.first, next_pos.second + 1);
    }
}

void Level::step(Direction dir) {
    prev_dir = dir;
    auto next_player_pos = get_next_position(dir);
    if (!check_walls_pos(next_player_pos) && !check_boxes_pos(next_player_pos)) {
        player = next_player_pos;
        steps++;
    }
    else if (check_boxes_pos(next_player_pos)) {
        auto next_box_pos = box_next_position(dir, next_player_pos);
        if (!check_walls_pos(next_box_pos) && !check_boxes_pos(next_box_pos)) {
            player = next_player_pos;
            steps++;
            boxes.push_back(next_box_pos);
            auto it = std::find(boxes.begin(), boxes.end(), next_player_pos);
            if (it != boxes.end()) {
                boxes.erase(it);
            }
        }
    }
    return;
}