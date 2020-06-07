import networkx as nx
import matplotlib.pyplot as plt
from xml.dom import minidom
import math

# EXAMPLE show_graph(load_to_graph('data/final_file.osm'))


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
            G.add_edge(intersections[indx].getAttribute('ref'), intersections[indx+1].getAttribute('ref'))
    
    return G


def show_graph(G, path = None) :
    """
    Display nx graph G
    params: 
        - G: graph to display
        - path: edges to draw in a different color none by default
    """

    pos=nx.get_node_attributes(G,'pos')
    plt.figure()
    nx.draw(G, pos=pos, node_size=1)

    #fullscreen
    figManager = plt.get_current_fig_manager()
    figManager.full_screen_toggle()

    plt.show()

# TODO comments adaguately this lines
def calculate_distance(first_lat, first_long, second_lat, second_long):
    RADIUS = 6371000
    phi_1 = degree_to_radiants(first_lat)
    phi_2 = degree_to_radiants(second_lat)
    delta_phi = degree_to_radiants(second_lat-first_lat)
    delta_lambda = degree_to_radiants(second_long-first_long)

    a = (math.sin(phi_2/2)**2) * math.cos(phi_1) * math.cos(phi_2) * (math.sin(delta_lambda/2)**2)

    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1-a))

    return RADIUS * c


def degree_to_radiants(angle):
    return angle * math.pi / 180
