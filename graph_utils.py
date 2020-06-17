import networkx as nx
import matplotlib.pyplot as plt
from xml.dom import minidom
import random
import math

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
    figManager = plt.get_current_fig_manager()
    figManager.full_screen_toggle()

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
