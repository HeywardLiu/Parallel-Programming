#include "page_rank.h"

#include <stdlib.h>
#include <cmath>
#include <omp.h>
#include <utility>

#include "../common/CycleTimer.h"
#include "../common/graph.h"

// pageRank --
//
// g:           graph to process (see common/graph.h)
// solution:    array of per-vertex vertex scores (length of array is num_nodes(g))
// damping:     page-rank algorithm's damping parameter
// convergence: page-rank algorithm's convergence threshold
//

/*
    For PP students: Implement the page rank algorithm here.  You
    are expected to parallelize the algorithm using openMP.  Your
    solution may need to allocate (and free) temporary arrays.

    Basic page rank pseudocode is provided below to get you started:

    // initialization: see example code above
    score_old[vi] = 1/numNodes;

    while (!converged) {

        // compute score_new[vi] for all nodes vi:
    
        if(vertex vi have outgoing edges):
            score_new[vi] = sum over all nodes vj reachable from incoming edges
                                { score_old[vj] / number of edges leaving vj  }
            score_new[vi] = (damping * score_new[vi]) + (1.0-damping) / numNodes;   

        else
            score_new[vi] += sum over all nodes v in graph with no outgoing edges
                                { damping * score_old[v] / numNodes }

        // compute how much per-node scores have changed
        // quit once algorithm has converged

        global_diff = sum over all nodes vi { abs(score_new[vi] - score_old[vi]) };
        converged = (global_diff < convergence)
    }

*/

// Parallel Version
void pageRank(Graph g, double *solution, double damping, double convergence)
{

    // initialize vertex weights to uniform probability. Double
    // precision scores are used to avoid underflow for large graphsm

    int numNodes = num_nodes(g);  // Init
    double equal_prob = 1.0 / numNodes;

    #pragma omp parallel for
    for (int i = 0; i < numNodes; ++i)
    {
        solution[i] = equal_prob;
    }

    bool converged = false;
    double *old_solution = new double[numNodes];
    while (!converged)
    {
        double global_diff = 0.0;
        double no_outgoing_score = 0.0;

        #pragma omp parallel for
        for(int i = 0; i < numNodes; i++) {  // Update scores
            old_solution[i] = solution[i];
        }

        #pragma omp parallel for reduction(+: no_outgoing_score)
        for(int i = 0; i < numNodes; i++) {  // Calculate new scores: without outgoing edge
            if (outgoing_size(g, i) == 0)   
                no_outgoing_score += damping * old_solution[i] / numNodes;
        }

        #pragma omp parallel for reduction(+: global_diff)
        for(int i = 0; i < numNodes; i++) {  // Calculate new scores: with outgoing edge(s)
            double sum = 0.0;
            const Vertex *in_start = incoming_begin(g, i);
            const Vertex *in_end = incoming_end(g, i);
            for (const Vertex *v = in_start; v != in_end; v++)
            {
                sum += old_solution[*v] / (double)outgoing_size(g, *v);
            }

            sum = (damping * sum) + (1.0 - damping) / numNodes;

            sum += no_outgoing_score;
            solution[i] = sum;
            global_diff += fabs(sum - old_solution[i]);
        }
        converged = global_diff < convergence;
    }

    delete old_solution;
}