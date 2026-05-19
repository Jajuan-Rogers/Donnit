#include "ncpp/Visual.hh"
#include <cstdint>
#include <ncpp/Plane.hh>
#include <span>
#include <variant>
#include <vector>

using ChildWidgets =
    std::span<std::variant<const ncpp::Plane, const ncpp::Visual>>;

struct Window {
  std::string id;
  int x;
  int y;
  int h;
  int w;
  uint32_t bg_color;
  uint32_t fg_color;
  ncpp::Plane plane;
  std::vector<std::variant<ncpp::Plane, ncpp::Visual>> planes;
};

void new_window(std::string id, int x, int y, int w, int h,
                ChildWidgets cildren);

void move_winodw(Window window, int x, int y);
void update_window(Window window);
