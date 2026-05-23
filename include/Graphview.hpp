#include "Window.hpp"
#include "matplot/matplot.h"
#include "ncpp/Visual.hh"
#include <string_view>
#include <vector>

struct Graphview : Window {
  const std::string id;
  std::string title;
  std::vector<std::string_view> fp;
  std::string_view current_graph;
  ncpp::Visual* graph_ncv;


  Graphview(ncpp::Plane *parent_pile, const std::string id, std::string title);
};
