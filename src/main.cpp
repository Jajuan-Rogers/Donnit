#include "Widget.hpp"
#include <fmt/base.h>
#include <fmt/format.h>
#include <ncpp/Cell.hh>
#include <ncpp/NotCurses.hh>
#include <ncpp/Plane.hh>
#include <ncpp/Visual.hh>
#include <ncpp/ncpp.hh>
#include "nckeys.h"
#include "notcurses.h"

ncpp::Plane* get_stdplane(ncpp::NotCurses& nc){
  return nc.get_stdplane();
}



int main() {
  notcurses_options opts = {};
  ncpp::NotCurses nc(opts);
  ncpp::Plane* stdplane =  get_stdplane(nc);

  Widget widget(stdplane,"hello world",10,10,20,20,nullptr);
  widget.plane->putstr(0,0,"HELLOOO");
  ncinput ncin;
  nc.mouse_enable(NCMICE_ALL_EVENTS);

  while (true){
    nc.render();
    const char32_t key = nc.get(true, &ncin);
    if (key == NCKEY_ESC){
      break;
    }else if (nckey_mouse_p(key)) {
      widget.plane->putstr(0,0,"A FUCKING MOUSE !");
    }
    // const char32_t key = nc.get(true, &ncin);
    // myWindow.plane->putc(0,0,key);
  }

  return 0;
}
