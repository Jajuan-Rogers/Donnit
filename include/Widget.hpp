#include "notcurses.h"
#include <functional>
#include <ncpp/Plane.hh>
#include <optional>
#include <stdexcept>
#include <string>

enum class KeyEvent { KEYPRESS, MOUSE_CLICK };

struct KeyboardEvent {
  KeyEvent type;
  char32_t key;
  std::optional<std::function<void()> *> cb;
};

struct Widget {
  std::string id;
  int x;
  int y;
  unsigned int w;
  unsigned int h;
  ncplane_options *popts = nullptr;
  ncpp::Plane *plane = nullptr;
  ncpp::Plane *stdplane;

  Widget(ncpp::Plane *stdplane, std::string id, int x, int y, unsigned int w,
         unsigned int h, ncplane_options *popts) {
    ncplane_options local_opts;
    if (popts == nullptr) {
      local_opts = {
          .y = y, .x = x, .rows = h, .cols = w, .userptr = nullptr};
      popts = &local_opts;
    }
    struct ncplane *raw_plane = ncplane_create(*stdplane, popts);
    if (raw_plane == nullptr) {
      std::runtime_error{"failed to make plane for widget"};
    }
    this->plane = new ncpp::Plane(raw_plane);
  }

  ~Widget() { delete plane; };
};
