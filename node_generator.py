from xml.dom import minidom
thiene = minidom.parse('data/Thiene.osm')
useful_nodes = minidom.parse('data/useful_nodes.osm')

# Extract the way and the nodes from the source files
waylist = thiene.getElementsByTagName('way')
selected_nodes = useful_nodes.getElementsByTagName('node')

# Opens the document where will be written the data
osm_file = open('data/final_file.osm', 'w')
osm_file.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"+"\n")
osm_file.write("<osm version=\"0.6\">"+"\n")

# Writes all the relevant nodes to the file
for node in selected_nodes:
    osm_file.write(node.toxml()+"\n")

# For way in waylist:
for way in waylist:
    # Extract the tags to check if the way is a building
    tags_child_nodes = way.getElementsByTagName('tag')

    # Boolean to check if one tag is "building"
    there_is_building = True
    for tags_child in tags_child_nodes:
        if(tags_child.getAttribute("k") == "highway"):
            there_is_building = False
        
    # If it's a building skip the way
    if(there_is_building):
        continue
        
    # Get all the way's nodes
    child_nodes = way.getElementsByTagName("nd")

    # Check all the possible match
    for node in child_nodes:
        # Boolean value used to see if the node is in the checked ones
        is_absent = True
        for check_node in selected_nodes:
            # If the id is present continue
            if(node.getAttribute("ref")==check_node.getAttribute("id")):
                is_absent = False
                break
            
        # If the nodes isn't in the checked ones it
        # remove the child from the way
        if(is_absent):
            way.removeChild(node)

    # Print the way into the final file
    osm_file.write(way.toxml() + "\n")

# Close the file
osm_file.write("</osm>")
osm_file.close()