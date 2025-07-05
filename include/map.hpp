#ifndef MAP_HPP
#define MAP_HPP

#include <algorithm>
#include <deque>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

struct Position {
  int x_asis{};
  int y_asis{};
  Position(int x = 0, int y = 0) : x_asis(x), y_asis(y) {}
};

inline bool operator==(const Position &lhs, const Position &rhs) {
  return lhs.x_asis == rhs.x_asis and lhs.y_asis == rhs.y_asis;
};

enum cell_t {
  NONE,
  PATH,
  INVISIBLE,
  WIN_COIN,
  LOST_COIN,
  STAR,
  PLAYER,
};

enum direction_t { N, S, L, O, NO, NE, SE, SO };

const inline std::unordered_map<char, cell_t> char_2_cell{
    {'#', PATH},      {'.', INVISIBLE}, {'+', WIN_COIN},
    {'-', LOST_COIN}, {'*', STAR},      {'&', PATH} // '&' indica início
};

const inline std::unordered_map<cell_t, std::string> cell_2_symbol{
    {PATH, "🔵"},      {INVISIBLE, "  "}, {WIN_COIN, "🟢"},
    {LOST_COIN, "🔴"}, {STAR, "⭐"},       {PLAYER, "👾"}};

class Map {
private:
  std::vector<std::vector<cell_t>> m_board;
  std::deque<Position> m_path;
  Position m_start_pos;

  void init_path() {

    Position current = m_start_pos;
    m_path.insert(m_path.end(),current);
    std::queue<Position> q;
    q.push(current);

    while (!q.empty()) {

      Position current = q.front();
      q.pop();

      for (direction_t dir : {N, S, L, O, NO, NE, SO, SE}) {
        if (is_blocked(current, dir))
          continue;

        Position next = walk_to(current, dir);
        if (std::find(m_path.begin(), m_path.end(), next) == m_path.end()) {
          m_path.push_back(next);
          q.push(next);
        }
      }
    }
  }

  Position walk_to(const Position &pos, const direction_t &dir) const {
    switch (dir) {
    case N:
      return Position(pos.x_asis - 1, pos.y_asis);
    case S:
      return Position(pos.x_asis + 1, pos.y_asis);
    case L:
      return Position(pos.x_asis, pos.y_asis - 1);
    case O:
      return Position(pos.x_asis, pos.y_asis + 1);
    case NO:
      return Position(pos.x_asis - 1, pos.y_asis - 1);
    case NE:
      return Position(pos.x_asis - 1, pos.y_asis + 1);
    case SE:
      return Position(pos.x_asis + 1, pos.y_asis + 1);
    case SO:
      return Position(pos.x_asis + 1, pos.y_asis - 1);
    default:
      return pos;
    }
  }

  bool is_blocked(const Position &pos, const direction_t &dir) const {
    Position next{walk_to(pos, dir)};
    if (is_outside(next))
      return true;

    auto cell{m_board[next.x_asis][next.y_asis]};
    return cell == INVISIBLE;
  }

  bool is_outside(const Position &pos) const {
    return pos.x_asis < 0 or pos.y_asis < 0 or
           pos.x_asis >= static_cast<int>(m_board.size()) or
           pos.y_asis >= static_cast<int>(m_board[pos.x_asis].size());
  }

public:
  Map(std::vector<std::vector<cell_t>> input_matrix, Position start_pos)
      : m_board(std::move(input_matrix)), m_start_pos(start_pos) {
    init_path();
  }

  cell_t get_cell(std::size_t row, std::size_t col) const {
    return m_board.at(row).at(col);
  }

  Position get_start_pos() const { return m_start_pos; }

  std::size_t rows() const { return m_board.size(); }

  std::size_t cols() const { return m_board.empty() ? 0 : m_board[0].size(); }

  void display(const Position &current_player_position) const {
    for (std::size_t i{0}; i < m_board.size(); ++i) {
      for (std::size_t j{0}; j < m_board[i].size(); ++j) {
        if (static_cast<int>(i) == current_player_position.x_asis &&
            static_cast<int>(j) == current_player_position.y_asis) {
          std::cout << cell_2_symbol.at(PLAYER);
        } else {
          std::cout << cell_2_symbol.at(m_board.at(i).at(j));
        }
      }
      std::cout << '\n';
    }
  }

  Position next_pos() {
    auto next = m_path.front();
    m_path.pop_front();
    m_path.push_back(next);
    return next;
  }
};

#endif //!< MAP_HPP