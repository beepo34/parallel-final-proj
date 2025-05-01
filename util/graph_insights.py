import argparse
import tqdm
import numpy as np
import matplotlib.pyplot as plt

def main(infile):
    file_name = infile.split('.')[0].split('/')[-1]
    print(file_name)
    with open(infile, 'r') as infile:
        [num_vertices, num_edges, edge_weights] = [None, None, None]

        while num_vertices is None:
            new_line = infile.readline()
            if new_line[0] == '%': #comment line
                continue
            else:
                [num_vertices, num_edges, edge_weights] = [int(x) for x in new_line.strip().split(' ')]
                edge_weights = True if edge_weights == 1 else False
        
        edges = [0 for i in range(num_vertices)]
        weights = [0 for i in range(num_vertices)] if edge_weights else None
        lowest_edge_ct, highest_edge_ct = float('inf'), float('-inf')
        lowest_weight, highest_weight = float('inf'), float('-inf')
        zero_vertices = [] #vertices with 0 edges
        one_vertices = [] #vertices with 1 edge

        #assume no more comments in the file
        for i in tqdm.tqdm(range(num_vertices)):
            data = [int(_) for _ in infile.readline().strip().split(' ')]
            edges[i] = len(data) // 2 if edge_weights else len(data)
            lowest_edge_ct = min(lowest_edge_ct, edges[i])
            highest_edge_ct = max(highest_edge_ct, edges[i])

            if edge_weights:
                if len(data) > 0:
                    weights[i] = sum(data[1::2]) #get every weight
                lowest_weight = min(lowest_weight, weights[i])
                highest_weight = max(highest_weight, weights[i])
            

        print(f"There are {len(zero_vertices)} vertices with no edges: {zero_vertices}")
        print(f"There are {len(one_vertices)} vertices with one edge: {one_vertices}")
        print(f"Lowest edge ct: {lowest_edge_ct}, Highest edge ct: {highest_edge_ct}")
        if edge_weights:
            print(f"Lowest weight: {lowest_weight}, Highest weight: {highest_weight}")

        bin_width = 3 if num_vertices > 10000 else 1

        
        counts, bins = np.histogram(edges, bins = np.arange(start=lowest_edge_ct, stop=highest_edge_ct + bin_width, step=bin_width))
        plt.stairs(counts, bins)
        plt.xlabel("Number of Edges")
        plt.ylabel("Count")
        plt.title(f"{file_name} edges (V = {num_vertices}, E = {num_edges})")
        plt.savefig(f"figures/{file_name}_edges.png")
    
        if edge_weights:
            plt.clf()
            bin_width = 100 if num_vertices > 10000 else 5
            counts, bins = np.histogram(weights, bins = np.arange(start=lowest_weight, stop=highest_weight + bin_width, step=bin_width))
            plt.stairs(counts, bins)
            plt.xlabel("Per-Vertex Summed Edge Weight")
            plt.ylabel("Count")
            plt.title(f"{file_name} weights (V = {num_vertices}, E = {num_edges})")
            plt.savefig(f"figures/{file_name}_weights.png")


if __name__=="__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--infile", type = str, required = True)

    args = parser.parse_args()
    main(args.infile)