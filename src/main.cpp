#include <fmt/base.h>
#include <fmt/format.h>
#include <ncpp/NotCurses.hh>
#include <ncpp/Plane.hh>
#include <ncpp/Visual.hh>
#include <ncpp/ncpp.hh>
#include "Window.hpp"
#include "notcurses.h"

ncpp::Plane* get_stdplane(ncpp::NotCurses& nc){
  return nc.get_stdplane();
}


int main() {
  notcurses_options opts = {};
  ncpp::NotCurses nc(opts);
  ncpp::Plane* stdplane =  get_stdplane(nc);

  Window myWindow = new_window("my_window", 2, 2, 12, 12,0x2A0081, 0xF5C700, *stdplane, "The Worstie");
  ncinput ncin;

  nc.render();
  const char32_t key = nc.get(true, &ncin);


  const char32_t key_2 = nc.get(true, &ncin);
  move_window(myWindow, 30, 30);
  nc.render();
  const char32_t key_3 = nc.get(true, &ncin);






  return 0;
}
