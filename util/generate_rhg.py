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

def main(n_nodes, avg_degree, gamma, weighted = False, outfile_path = None, outfile_name = None):
    hg = nk.generators.HyperbolicGenerator(n = n_nodes, k = avg_degree, gamma = gamma)
    hgG = hg.generate()
    print(f"Nodes: {hgG.numberOfNodes()}, Edges:{hgG.numberOfEdges()}")
    if weighted:
        hgG = make_weighted_copy(hgG)

    if outfile_path is None:
        outfile_path = f"{os.environ['SCRATCH']}/random_hyperbolic_graphs"
    if outfile_name is None:
        outfile_name = f"rhg_{n_nodes}_{avg_degree}_{gamma}"
        if weighted:
            outfile_name += "_w"

    
    nk.graphio.writeGraph(hgG, f"{outfile_path}/{outfile_name}.metis", nk.Format.METIS)


if __name__=="__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument("--number_vertices", "-n", type = int, default = 2) #number of nodes - 2^20 to 2^25 in VieCut paper
    parser.add_argument("--exponent", "-e", type = int, default = 10) #exponent for n argument
    parser.add_argument("--average_degree", "-k", type = int, default = 2**5) #average vertex degree - 2^5 to 2^8 in VieCut paper
    parser.add_argument("--gamma", "-g", type = int, default = 5) #exponent for the power distribution - strictly 5 in VieCut paper
    parser.add_argument("--weighted", "-w", action='store_true') #if this graph should be weighted or not
    parser.add_argument("--file_path", type = str, default = None) #folder path for output
    parser.add_argument("--file_name", type = str, default = None) #file name for output

    args = parser.parse_args()
    num_vertices = args.number_vertices ** args.exponent
    main(num_vertices, args.average_degree, args.gamma, weighted = args.weighted, outfile_path = args.file_path, outfile_name = args.file_name)