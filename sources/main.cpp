#include "../include/party.hpp"

int main(int argc, char *argv[]) {
  GameController::init(argc, argv);
  GameController::run();

  return EXIT_SUCCESS;
}
