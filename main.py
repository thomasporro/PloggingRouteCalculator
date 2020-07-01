from graph_utils import *

def main():

    starting_node='298177687'
    tree ={}
    branching_probabilities = [0, 0.5, 1]
    path_length = 3000
    min_garbage = 3

    graph = load_to_graph('data/final_file.osm')
    cycle_edges = generate_tree_BFS(tree, graph, starting_node)
    show_graph(graph,  find_best_cycle(graph, tree, starting_node, cycle_edges,
                                        branching_probabilities, path_length, min_garbage) )

if __name__ == '__main__':
    main()