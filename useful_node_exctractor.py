from xml.dom import minidom

# Import the file
doc = minidom.parse('data\Thiene.osm')

# Extract the nodes
nodelist = doc.getElementsByTagName('node')

# For each node create a new attribute to count how many
# time that node is inside a way
for node in nodelist:
    new_attribute = doc.createAttribute("arity")
    new_attribute.value = "0"
    node.setAttributeNode(new_attribute)

# Extract the ways
waylist = doc.getElementsByTagName('way')

# Check all the nodes of all ways
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

    # Gets all the child nodes
    child_nodes = way.getElementsByTagName('nd')
    
    # Counter used to check if the node is at the start
    # or at the end of the way
    length_counter = 0
    
    # For each node inside a way it increases the value of arity
    for node in child_nodes:
        for parent in nodelist:
            if(parent.getAttribute('id')==node.getAttribute('ref')):
                # Retrieve the value of arity and convert it into an int value,
                # than it upgrades it
                value_string = parent.getAttribute('arity')
                value = int(value_string)
                value += 1

                # If a node is at the end or at the start of a way, it increases its
                # value, so we can keep also the closed alleys
                if(length_counter == 0 or length_counter == (len(child_nodes)-1)):
                    value += 1
                
                # Sets the new value of the arity attribute
                parent.setAttribute("arity", str(value))
                break
                
        length_counter += 1


# Opens the document where will be written the data
osm_file = open('data/useful_nodes.osm', 'w')
osm_file.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"+"\n")
osm_file.write("<osm version=\"0.6\">"+"\n")

# If a node have a arity attribute greater than 1 is copied into
# the file
for node in nodelist:
    # We skip this node beceause it causes problem but we don't know why.
    # Anyway is an useless node
    if(node.getAttribute("id")=="7558462927"): continue

    # Prints the node inside the file
    value = int(node.getAttribute("arity"))
    if(value>1): osm_file.write(node.toxml()+"\n")

# Close the file
osm_file.write("</osm>")
osm_file.close()