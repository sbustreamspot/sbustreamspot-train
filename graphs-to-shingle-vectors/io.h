/* 
 * Copyright 2016 Emaad Ahmed Manzoor
 * License: Apache License, Version 2.0
 * http://www3.cs.stonybrook.edu/~emanzoor/streamspot/
 */

#ifndef STREAMSPOT_IO_H_
#define STREAMSPOT_IO_H_

#include "graph.h"
#include <string>
#include <tuple>
#include <vector>

namespace std {

tuple<uint32_t,vector<edge>> read_edges(string filename);
tuple<vector<vector<uint32_t>>, vector<double>, double>
  read_bootstrap_clusters(string bootstrap_file);

}

#endif
