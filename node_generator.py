from xml.dom import minidom
thiene = minidom.parse('data/Thiene.osm')
useful_nodes = minidom.parse('data/useful_nodes.osm')

#Extract the way and the nodes from the suorce files
waylist = thiene.getElementsByTagName('way')
selected_nodes = useful_nodes.getElementsByTagName('nodes')

#Opens the document where will be written the data
osm_file = open('data/final_file.osm', 'w')
osm_file.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"+"\n")
osm_file.write("<osm version=\"0.6\">"+"\n")

print(waylist[0].nodeName())

#Writes all the relevant nodes to the file
#for node in selected_nodes:
 #   osm_file.write(node.toxml()+"\n")

#for way in waylist:
    


osm_file.write("</osm>")
osm_file.close()