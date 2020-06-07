import networkx as nx
import matplotlib.pyplot as plt
from xml.dom import minidom

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

