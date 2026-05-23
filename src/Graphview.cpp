

#include "Graphview.hpp"
#include "matplot/freestanding/plot.h"
#include <string_view>
#include <vector>
void new_plot(Graphview &gv, std::string_view fp){
  std::vector<double> a = {1.0,2.0,3.0,4.0};
  matplot::plot(a);
  matplot::save("./my_graph", "png");
}
