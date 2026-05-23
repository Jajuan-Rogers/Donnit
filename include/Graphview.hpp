#pragma once
#include "Window.hpp"
#include "ncpp/Visual.hh"

#include <ncpp/NotCurses.hh>
#include <ncpp/Plane.hh>
#include <string_view>
#include <utility>
#include <vector>

struct Graphview : Window {
  unsigned int img_w;
  unsigned int img_h;
  std::string title;
  std::vector<std::string_view> image_paths;
  unsigned char* current_graph;
  ncpp::Visual* graph_ncv;

  Graphview(
      ncpp::NotCurses* nc,
      const std::string id,
      int y,
      int x,
      unsigned int w,
      unsigned int h,
      std::string title = "_")
      : Window(*nc, id, y, x, w, h, title) {
      };
};


void new_plot(Graphview &gv, std::string_view fp);
