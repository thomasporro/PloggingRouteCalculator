import networkx as nx
import matplotlib.pyplot as plt
from xml.dom import minidom
import random
import math, time

# EXAMPLE show_graph(load_to_graph('data/final_file.osm'))
#       graph = load_to_graph('data/final_file.osm')
#       nx.get_edge_attributes(graph, 'garbage')

def load_to_graph(file_path):
    """
    Load nodes and edges from .osm file into a networkx graph
    params: file_path = path to the .osm file
    returns: networkx graph
    """
    
    G = nx.Graph(file=file_path)
    
    xml_doc = minidom.parse(file_path)
    
    nodelist = xml_doc.getElementsByTagName('node')
    waylist = xml_doc.getElementsByTagName('way')

    # load nodes
    for node in nodelist:
        G.add_node(node.getAttribute('id'), pos=(float(node.getAttribute('lat')), float(node.getAttribute('lon'))) )


    # load edges
    for way in waylist:
        intersections = way.getElementsByTagName('nd')
        for indx in range(0, len(intersections)-1):
            pos = nx.get_node_attributes(G,'pos')
            length = geodesic_dist(float(pos[intersections[indx].getAttribute('ref')][0]),
                                    float(pos[intersections[indx].getAttribute('ref')][1]),
                                     float(pos[intersections[indx+1].getAttribute('ref')][0]),
                                      float(pos[intersections[indx+1].getAttribute('ref')][1]))

            garbage = 0
            rand = random.random()
            if 0.85 < rand < 0.95: garbage=1
            elif rand > 0.95: garbage=2

            G.add_edge(intersections[indx].getAttribute('ref'),
                        intersections[indx+1].getAttribute('ref'),
                         length = length, garbage = garbage)
    return G


def show_graph(G, path = None) :
    """
    Display nx graph G
    params: 
        - G: graph to display
        - path: list of edges to draw in a different color, None by default
            example [('6463933402', '6463933405'), ('6463933404', '6463933405'), ('6463933405', '6463933406')]
    """

    pos=nx.get_node_attributes(G,'pos')
    plt.figure()
    nx.draw(G, pos=pos, node_size=1)

    if path!=None:
        nx.draw_networkx_edges(G,pos,edgelist=path,edge_color='r',width=2)

    #fullscreen
    #figManager = plt.get_current_fig_manager()
    #figManager.full_screen_toggle()

    plt.show()


def geodesic_dist(lat1, lon1, lat2, lon2):
    """
    Approximate the distance on the earth between two 
    points in longitude, latitude format.
    params: coordinates of the points in degrees lat1, lon1, lat2, lon2.
    returns: distance in m between the points.
    """
    # approximate radius of earth in m
    R = 6373000.0

    dlon = math.radians(lon2) - math.radians(lon1)
    dlat = math.radians(lat2) - math.radians(lat1)

    a = math.sin(dlat / 2)**2 + math.cos(lat1) * math.cos(lat2) * math.sin(dlon / 2)**2
    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a))

    return R * c


def generate_tree_BFS(tree, graph, root):
    """
    Generate tree in a dict of pairs (son:parent) from
    graph using bfs starting frome the node root.
    params: tree is an empty dict where the tree will be saved, graph and starting node
    returns: list of edges that generate cycles
    """
    cycle_edges=[]
    tree[root] = 0
    front =[root]
    visited_nodes=[]
    cycle_edges=[]

    for node in front:
        neighbors = graph.neighbors(node)
        visited_nodes.append(node)
        for neighbor in neighbors:
            if tree[node] != neighbor and (node, neighbor) not in cycle_edges:
                if neighbor in visited_nodes and (node, neighbor) not in cycle_edges:
                    cycle_edges.append((neighbor, node))
                if neighbor not in front:
                    front.append(neighbor)
                    tree[neighbor]=node

    return cycle_edges

def find_best_cycle(graph, tree, root, cycle_edges, deviation_probs, length, min_g):
    """
    find cycles by selecting edges that close a path in the tree and
    return the best one with respect to the parameters
    params:
        graph: the graph 
        tree: tree produced by generate_tree_BFS
        root: root node of the tree
        cycle_edges: edges that connect two nodes of the tree forming a cycle
        deviation_probs: list of probabilities of taking another
            cycle edge encountered on the path to the root
        length: length of the path to find
        min_g: minimum quantityof garbage on the path
    returns best cycle found
    """
    best_cycle=[]
    best_len=0
    
    cycle_nodes_dict={}
    for edge in cycle_edges:
        for node in edge:
            if node in cycle_nodes_dict:
                cycle_nodes_dict[node].append(edge)
            else:
                cycle_nodes_dict[node] =[edge]
    
    garbage_edges=nx.get_edge_attributes(graph, 'garbage')
    length_edges=nx.get_edge_attributes(graph, 'length')

    for edge in cycle_edges:
        # for both nodes in the edge return to the root through the tree
        for prob in deviation_probs:
            # at each node encountered connected to a cycle edge change path with probability prob
            cycle=[]
            node_a=edge[0]
            node_b=edge[1]
            
            while tree[node_a]!=0: #not root
                cycle.append((node_a, tree[node_a]))
                node_a = tree[node_a]
                
                if node_a in cycle_nodes_dict and random.random()<prob:
                    
                    selected_edge=cycle_nodes_dict[node_a][random.randint(0, len(cycle_nodes_dict[node_a])-1) ]
                    if node_a == selected_edge[0]:
                        node_a=selected_edge[1]
                    else:
                        node_a=selected_edge[0]

            cycle.reverse() #just to keep the edges of the cycle in order

            cycle.append(edge)
            
            while tree[node_b]!=0:
                cycle.append((node_b, tree[node_b]))
                node_b = tree[node_b]
                if node_b in cycle_nodes_dict and random.random()<prob:
                    
                    selected_edge=cycle_nodes_dict[node_b][random.randint(0, len(cycle_nodes_dict[node_b])-1) ]
                    if node_b == selected_edge[0]:
                        node_b=selected_edge[1]
                    else:
                        node_b=selected_edge[0]

            total_l=0
            total_g=0

            for edge in cycle:
                if edge in length_edges:
                    total_l+=length_edges[edge]
                    total_g+=garbage_edges[edge]
                else:
                    total_l+=length_edges[edge[::-1]]
                    total_g+=garbage_edges[edge[::-1]]

            if abs(best_len-length) > abs(total_l-length) and total_g>=min_g:
                best_len=total_l
                best_cycle=cycle

    print('len: '+str(best_len))

    return best_cycle

