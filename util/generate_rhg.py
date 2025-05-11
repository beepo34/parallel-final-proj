import networkit as nk
import argparse
import os
import tqdm
import random

"""
https://networkit.github.io/dev-docs/notebooks/Generators.html
"""


def make_weighted_copy(G_unweighted):
    G_weighted = nk.Graph(G_unweighted.numberOfNodes(), weighted=True, directed=G_unweighted.isDirected())

    for u, v in G_unweighted.iterEdges():
        w = random.randint(1, 10)
        G_weighted.addEdge(u, v, w)

    return G_weighted

def main(n_nodes, avg_degree, gamma, weighted = False, outfile_name = None):
    hg = nk.generators.HyperbolicGenerator(n = n_nodes, k = avg_degree, gamma = gamma)
    hgG = hg.generate()
    print(f"Nodes: {hgG.numberOfNodes()}, Edges:{hgG.numberOfEdges()}")
    if weighted:
        hgG = make_weighted_copy(hgG)

    if outfile_name is None:
        outfile_name = f"rhg_{n_nodes}_{avg_degree}_{gamma}"
        if weighted:
            outfile_name += "_w"

    
    nk.graphio.writeGraph(hgG, f"{os.environ['SCRATCH']}/random_hyperbolic_graphs/{outfile_name}.metis", nk.Format.METIS)


if __name__=="__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument("--number_vertices", "-n", type = int, default = 2**10) #number of nodes - 2^20 to 2^25 in VieCut paper
    parser.add_argument("--average_degree", "-k", type = int, default = 2**5) #average vertex degree - 2^5 to 2^8 in VieCut paper
    parser.add_argument("--gamma", "-g", type = int, default = 5) #exponent for the power distribution - strictly 5 in VieCut paper
    parser.add_argument("--weighted", "-w", action='store_true') #if this graph should be weighted or not

    args = parser.parse_args()
    main(args.number_vertices, args.average_degree, args.gamma, weighted = args.weighted)