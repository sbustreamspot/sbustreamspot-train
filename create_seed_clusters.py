#!/usr/bin/env python

import argparse
from constants import *
import numpy as np
import random
import scipy
import sklearn.preprocessing
from sklearn.metrics import silhouette_score
from sklearn.cluster import MiniBatchKMeans
import sys
from time import time

# finding best number of clusters
# http://stackoverflow.com/questions/15376075/cluster-analysis-in-r-determine-the-optimal-number-of-clusters/15376462#15376462

random.seed(SEED)
np.random.seed(SEED)

parser = argparse.ArgumentParser()
parser.add_argument('--input', help='Training graph shingle vectors',
                    required=True)
args = vars(parser.parse_args())

input_file = args['input']
with open(input_file, 'r') as f:
    num_shingles, chunk_length = map(int, f.readline().strip().split('\t'))
    shingles = f.readline().strip().split('\t')[1:]

    # read shingle count vectors from file
    X = [] # note: row i = graph ID i
    graph_ids = []
    for idx, line in enumerate(f):
        fields = line.strip().split('\t')
        graph_id = fields[0]
        graph_ids.append(graph_id)
        shingle_vector = map(int, fields[1:])
        X.append(shingle_vector)
    X = np.array(X, dtype=np.float)

    # L2-normalize shingle-count vectors
    X = sklearn.preprocessing.normalize(X, norm='l2', axis=1, copy=True)
   
    # kmeans
    best_n_clusters = -1
    best_silhouette_avg = -1
    best_cluster_labels = None
    best_cluster_centers = None
    for n_clusters in range(2, X.shape[0]):
        km = MiniBatchKMeans(n_clusters=n_clusters, init='k-means++', n_init=5,
                             init_size=1000, batch_size=1000, verbose=False,
                             random_state=SEED)
        print 'Clustering, k =', n_clusters,
        t0 = time()
        km.fit(X)
        print 'done in %0.3fs.' % (time() - t0),

        silhouette_avg = silhouette_score(X, km.labels_)
        print 'Silhouette Coefficient: %0.3f' % silhouette_avg

        if silhouette_avg > best_silhouette_avg or\
           (silhouette_avg == best_silhouette_avg and\
            n_clusters > best_n_clusters): # favour more clusters
            best_silhouette_avg = silhouette_avg
            best_n_clusters = n_clusters
            best_cluster_labels = km.labels_
            best_cluster_centers = km.cluster_centers_

            # compute radius of each cluster
            all_cluster_dists = []
            cluster_threshold = [-1] * len(best_cluster_centers) 
            for cluster_idx in range(best_n_clusters):
                cluster_center = best_cluster_centers[cluster_idx]
                cluster_graphs = [i for i in range(X.shape[0])
                                  if best_cluster_labels[i] == cluster_idx]
                cluster_dists = [scipy.spatial.distance.cosine(cluster_center, X[i])
                                 for i in cluster_graphs]
                all_cluster_dists.extend(cluster_dists)

                mean_dist = np.mean(cluster_dists)
                std_dist = np.std(cluster_dists)
                cluster_threshold[cluster_idx] = min(1.0, mean_dist +\
                                                     NUM_DEVS * std_dist)
            mean_all_cluster_dists = np.mean(all_cluster_dists)
            std_all_cluster_dists = np.mean(all_cluster_dists)
            all_cluster_threshold = min(1.0, mean_all_cluster_dists +\
                                        NUM_DEVS * std_all_cluster_dists)

            print str(best_n_clusters) + '\t' + str(X.shape[0]) + '\t',
            print str(chunk_length) + '\t',
            print "{:3.4f}".format(all_cluster_threshold)

            for cluster_idx in range(best_n_clusters):
                cluster_graphs = [i for i in range(X.shape[0])
                                  if best_cluster_labels[i] == cluster_idx]
                threshold = cluster_threshold[cluster_idx]
                print "{:3.4f}".format(threshold) + '\t',
                print '\t'.join([str(graph_ids[graph]) for graph in cluster_graphs])
            print
            sys.stdout.flush()
