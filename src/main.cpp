#include "Textbox.hpp"
#include "Window.hpp"
#include "ncc_utils.hpp"
#include "nckeys.h"
#include "notcurses.h"

#include <fmt/base.h>
#include <fmt/format.h>
#include <ncpp/Cell.hh>
#include <ncpp/NotCurses.hh>
#include <ncpp/Plane.hh>
#include <ncpp/Visual.hh>
#include <ncpp/ncpp.hh>

ncpp::Plane* get_stdplane(ncpp::NotCurses& nc) {
  return nc.get_stdplane();
}

int main() {
  notcurses_options opts = {};
  ncpp::NotCurses nc(opts);

  ncpp::Plane* stdplane = get_stdplane(nc);
  Window window(nc, "window_1", 20, 20, 20, 50, "Window 1");
  int* const x = nullptr;
  Textbox* textbox = new Textbox(window.window_pile, 10, 10, "textbox_1", "myButton", 50);
  Textbox* textbox_2 = new Textbox(window.window_pile, 14, 10, "widget_2", "myButton_2");
  unsigned int rgb =  0x4B75FF;
  fill_plane(*textbox->plane, rgb);
  window.children.push_back(textbox);
  window.children.push_back(textbox_2);

  ncinput ncin;
  nc.mouse_enable(NCMICE_ALL_EVENTS);

  while (true) {
    ncpile_render(*window.window_pile);
    ncpile_rasterize(*window.window_pile);
    const char32_t key = nc.get(true, &ncin);
    if (key == NCKEY_ESC) {
      break;
    } else if (nckey_mouse_p(key) and NCTYPE_PRESS) {
    } else {
      textbox->plane->putstr(textbox->get_id().c_str());
    }
    // const char32_t key = nc.get(true, &ncin);
    // myWindow.plane->putc(0,0,key);
  }

  return 0;
}
