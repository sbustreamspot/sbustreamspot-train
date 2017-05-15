/* 
 * Copyright 2016 Emaad Ahmed Manzoor
 * License: Apache License, Version 2.0
 * http://www3.cs.stonybrook.edu/~emanzoor/streamspot/
 */

#include <algorithm>
#include <bitset>
#include <cassert>
#include <chrono>
#include <deque>
#include <iostream>
#include <queue>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "docopt.h"
#include "graph.h"
#include "hash.h"
#include "io.h"
#include "param.h"

using namespace std;

static const char USAGE[] =
R"(StreamSpot.

    Usage:
      streamspot --edges=<edge file>
                 --chunk-length=<chunk length>

      streamspot (-h | --help)

    Options:
      -h, --help                              Show this screen.
      --edges=<edge file>                     Incoming stream of edges.
      --chunk-length=<chunk length>           Parameter C.
)";

int main(int argc, char *argv[]) {
  unordered_map<string,uint32_t> shingle_id;
  unordered_set<string> unique_shingles;

  // for timing
  chrono::time_point<chrono::steady_clock> start;
  chrono::time_point<chrono::steady_clock> end;
  chrono::nanoseconds diff;

  // arguments
  map<string, docopt::value> args = docopt::docopt(USAGE, { argv + 1, argv + argc });

  string edge_file(args["--edges"].asString());
  uint32_t chunk_length = args["--chunk-length"].asLong();

  cerr << "StreamSpot Graphs-to-Shingle-Vectors (";
  cerr << "C=" << chunk_length << "";
  cerr << ")" << endl;

  // FIXME: Tailored for this configuration now
  assert(K == 1);

  uint32_t num_graphs;
  vector<edge> train_edges;
  cerr << "Reading training edges..." << endl;
  start = chrono::steady_clock::now();
  tie(num_graphs, train_edges) = read_edges(edge_file);
  end = chrono::steady_clock::now();
  diff = chrono::duration_cast<chrono::nanoseconds>(end - start);
  cerr << "\tReading edges took: ";
  cerr << static_cast<double>(diff.count()) << "us" << endl;

  // per-graph data structures
  unordered_map<uint32_t,graph> graphs;
  unordered_map<uint32_t,shingle_vector> shingle_vectors;

  // construct training graphs
  cerr << "Constructing " << num_graphs << " training graphs..." << endl;
  start = chrono::steady_clock::now();
  for (edge& e : train_edges) {
    update_graphs(e, graphs);
  }
  end = chrono::steady_clock::now();
  diff = chrono::duration_cast<chrono::nanoseconds>(end - start);
  cerr << "\tGraph construction took: ";
  cerr << static_cast<double>(diff.count()) << "us" << endl;

  // construct shingle vectors
  cerr << "Constructing shingle vectors:" << endl;
  start = chrono::steady_clock::now();
  construct_shingle_vectors(shingle_vectors, shingle_id, graphs, chunk_length);
  end = chrono::steady_clock::now();
  diff = chrono::duration_cast<chrono::nanoseconds>(end - start);
  cerr << "\tShingle vector construction took: ";
  cerr << static_cast<double>(diff.count()) << "us" << endl;

  // print shingles
  cout << shingle_id.size() << "\t" << chunk_length << endl;
  vector<string> shingles(shingle_id.size());
  for (auto& kv : shingle_id) {
    shingles[kv.second] = kv.first;
  }

  cout << "shingles" << "\t";
  for (uint32_t i = 0; i < shingles.size() - 1; i++) {
    cout << shingles[i] << "\t";
  }
  cout << shingles[shingles.size() - 1] << endl;

  // print shingle vectors
  for (auto& kvg : graphs) {
    cout << kvg.first << "\t"; // graph id
    for (uint32_t j = 0; j < shingle_id.size() - 1; j++) {
      cout << shingle_vectors[kvg.first][j] << "\t"; // shingle frequency
    }
    cout << shingle_vectors[kvg.first][shingle_id.size() - 1] << endl;
  }

  return 0;
}

void allocate_random_bits(vector<vector<uint64_t>>& H, mt19937_64& prng,
                          uint32_t chunk_length) {
  // allocate random bits for hashing
  for (uint32_t i = 0; i < L; i++) {
    // hash function h_i \in H
    H[i] = vector<uint64_t>(chunk_length + 2);
    for (uint32_t j = 0; j < chunk_length + 2; j++) {
      // random number m_j of h_i
      H[i][j] = prng();
    }
  }
#ifdef DEBUG
    cout << "64-bit random numbers:\n";
    for (int i = 0; i < L; i++) {
      for (int j = 0; j < chunk_length + 2; j++) {
        cout << H[i][j] << " ";
      }
      cout << endl;
    }
#endif
}
