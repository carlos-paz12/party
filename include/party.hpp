#include "map.hpp"

#include <cstdlib>  // EXIT_SUCCESS
#include <ctime>    // time()
#include <fstream>  // std::ifstream
#include <iostream> // std::cout, std::endl
#include <memory>   // std::unique_ptr
#include <optional> // std::optional
#include <random>   // std::default_random_engine, std::uniform_int_distribution
#include <stdexcept> // std::runtime_error
#include <string>    // std::string
#include <unordered_map>
#include <utility>
#include <vector> // std::vector

// ==========================
// Loader
// ==========================
inline std::pair<std::vector<std::vector<cell_t>>, Position>
load_level_from_file(const std::string &filename) {
  std::vector<std::vector<cell_t>> matrix;
  std::ifstream file(filename);
  if (not file) {
    throw std::runtime_error("Erro ao abrir o arquivo do nível.");
  }

  std::string line{};
  Position start_pos;
  std::size_t row{0};

  while (std::getline(file, line)) {
    std::vector<cell_t> row_cells;
    for (std::size_t col{0}; col < line.size(); ++col) {
      char ch{line[col]};
      if (ch == '&') {
        start_pos = Position(row, col);
        row_cells.push_back(PATH);
      } else {
        row_cells.push_back(char_2_cell.at(ch));
      }
    }
    matrix.push_back(row_cells);
    ++row;
  }

  return {matrix, start_pos};
}

// ==========================
// Player
// ==========================
class Player {
private:
  std::string m_name{};
  std::size_t m_coins{};
  std::size_t m_stars{};
  Position m_position{};

public:
  Player(const std::string &name, const Position &start_pos)
      : m_name(name), m_coins(0), m_stars(0), m_position(start_pos) {}

  void add_coins(std::size_t amount) { m_coins += amount; }

  void reduce_coins(std::size_t amount) {
    m_coins = (amount >= m_coins) ? 0 : m_coins - amount;
  }

  void add_stars(std::size_t stars) { m_stars += stars; }

  void reset_position(const Position &start) { m_position = start; }

  Position get_position() const { return m_position; }

  void print_status() const {
    std::cout << m_name << " 👤 -> Moedas: " << m_coins
              << " | Estrelas: " << m_stars << " | Posição: ("
              << m_position.x_asis << ", " << m_position.y_asis << ")\n";
  }

  [[nodiscard]] std::string get_name() const { return m_name; }
};

// ==========================
// Controller
// ==========================
enum state_t {
  UNDEF = 0,
  WELCOME,
  PLAYING,
  ROLLING_DICE,
  GAME_OVER,
};

class GameController {
private:
  static std::vector<Player> m_players;
  static std::size_t m_turns;
  static std::unique_ptr<Map> m_map;
  static std::size_t turns_played;
  static state_t m_state;
  static std::unique_ptr<Player> m_current_player;

public:
  static void init(int argc, char *argv[]) {
    m_state = UNDEF;
    auto [matrix, start_pos] = load_level_from_file(argv[1]);
    m_map = std::make_unique<Map>(matrix, start_pos);
    m_players.emplace_back("Alice", start_pos);
    m_players.emplace_back("Bob", start_pos);
    m_current_player = std::make_unique<Player>(m_players.at(0));
  }

  static void run() {
    for (turns_played = 0; turns_played < 5; ++turns_played) {
      process();
      update();
      render();
    }
  }

private:
  static void process() {
    if (m_state == WELCOME) {
      std::cin.get();
      return;
    }
    if (m_state == PLAYING) {
      std::cout << "Vez de " << m_current_player->get_name();
      char action{};
      std::cin >> action;
      std::cin.ignore();
    }
  }

  static void update() {
    if (m_state == UNDEF) {
      m_state = WELCOME;
      return;
    }

    if (m_state == WELCOME) {
      m_state = PLAYING;
      return;
    }

    if (m_state == PLAYING) {
      // Alternar jogador a cada turno
      m_current_player =
          std::make_unique<Player>(m_players[turns_played % m_players.size()]);

      // Sortear número de passos (1-9)
      std::default_random_engine rng(static_cast<unsigned>(time(nullptr)));
      std::uniform_int_distribution<int> steps_dist(1, 9);
      int steps = steps_dist(rng);
      std::cout << m_current_player->get_name() << " sorteou " << steps
                << " passos!\n";

      Position current_pos = m_current_player->get_position();
      Position new_pos = current_pos;

      // Mover passo a passo
      for (int i = 0; i < steps; ++i) {
        // Gerar direção aleatória (0-3: cima, direita, baixo, esquerda)
        std::uniform_int_distribution<int> dir_dist(0, 3);
        int direction = dir_dist(rng);

        // Calcular nova posição temporária
        Position temp_pos = new_pos;
        switch (direction) {
        case 0:
          temp_pos.x_asis--;
          break; // Cima
        case 1:
          temp_pos.y_asis++;
          break; // Direita
        case 2:
          temp_pos.x_asis++;
          break; // Baixo
        case 3:
          temp_pos.y_asis--;
          break; // Esquerda
        }

        // Verificar se a posição é válida
        if (temp_pos.x_asis >= 0 &&
            temp_pos.x_asis < static_cast<int>(m_map->rows()) &&
            temp_pos.y_asis >= 0 &&
            temp_pos.y_asis < static_cast<int>(m_map->cols())) {

          cell_t cell = m_map->get_cell(temp_pos.x_asis, temp_pos.y_asis);
          if (cell == PATH || cell == WIN_COIN || cell == LOST_COIN ||
              cell == STAR) {
            new_pos = temp_pos;

            // Processar efeito da célula
            switch (cell) {
            case WIN_COIN:
              m_current_player->add_coins(10);
              std::cout << " +10 moedas!";
              break;
            case LOST_COIN:
              m_current_player->reduce_coins(5);
              std::cout << " -5 moedas!";
              break;
            case STAR:
              m_current_player->add_stars(1);
              std::cout << " +1 estrela!";
              break;
            default:
              break;
            }
          }
        }
        std::cout << " [" << new_pos.x_asis << "," << new_pos.y_asis << "]";
      }

      // Atualizar posição final
      m_current_player->reset_position(new_pos);
      std::cout << "\n"
                << m_current_player->get_name() << " parou em ["
                << new_pos.x_asis << "," << new_pos.y_asis << "]\n";
    }
  }

  static void render() {
    if (m_state == WELCOME) {
      std::cout << "BEM VINDO!!!";
    }
    if (m_state == PLAYING) {
      std::cout << "\nEstado do tabuleiro:\n";
      m_map->display(m_current_player->get_position());

      std::cout << "\nJogadores:\n";
      for (const auto &player : m_players) {
        player.print_status();
      }
      std::cout << '\n';
    }
  }
};

// ==========================
// Controller Static Members
// ==========================
std::vector<Player> GameController::m_players{};
std::size_t GameController::m_turns{0};
std::size_t GameController::turns_played{0};
std::unique_ptr<Map> GameController::m_map = nullptr;
state_t GameController::m_state{UNDEF};
std::unique_ptr<Player> GameController::m_current_player = nullptr;