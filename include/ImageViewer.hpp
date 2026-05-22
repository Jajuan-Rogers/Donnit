#include <array>
#include <ncpp/ncpp.hh>
#include <string>

enum class CardLocation { LIBRARY, HAND, GRAVEYARD, EXILE, BATTLEFIELD };

struct Card {
  std::string name;
  std::string image_fp;
  std::array<double, 2> pos;
  std::array<double, 2> size;
};


