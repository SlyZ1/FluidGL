class_name TrickGraph
extends RefCounted

class Trick:
	var name: String
	var input: String
	var duration: float
	var next: Array[String] = []

	func _init(p_name: String, p_input: String, p_duration: float, p_next: Array) -> void:
		name = p_name
		input = p_input
		duration = p_duration
		next.assign(p_next)

var _initial_nodes : Array[String] = []
var _nodes: Dictionary = {}

func load_json_file(file_path: String) -> Variant:
	if not FileAccess.file_exists(file_path):
		print("Error: File not found at path: ", file_path)
		return null

	var file = FileAccess.open(file_path, FileAccess.READ)
	if not file:
		print("Error: Could not open file at path: ", file_path)
		return null

	var json_text = file.get_as_text()

	var json = JSON.new()
	var error = json.parse(json_text)

	if error != OK:
		print("JSON Parse Error: ", json.get_error_message(), " in file ", file_path, " at line ", json.get_error_line())
		return null
	
	return json.get_data()
	

func _init(file_path : String) -> void:
	var data = load_json_file(file_path)
	_initial_nodes.assign(data["initial"])
	data = data["states"]
		
	_nodes.clear()
	
	for node_dict in data:
		var node_name: String = node_dict.get("name", "")
		var node_input: String = node_dict.get("input", "")
		var node_duration: float = node_dict.get("duration", "")
		var node_next: Array = node_dict.get("next", [])
		
		var node = Trick.new(node_name, node_input, node_duration, node_next)
		_nodes[node_name] = node


## Returns the MyGraphNode object by its name, or null if not found.
func get_node(node_name: String) -> Trick:
	return _nodes.get(node_name, null)

## Returns an array of neighboring node names for a given node.
func get_neighbours(node_name: String) -> Array[String]:
	if node_name == "":
		return _initial_nodes
	var node = get_node(node_name)
	if node:
		return node.next
	return []

## Prints the graph structure to the Output console for debugging.
func print_graph() -> void:
	for node_name in _nodes:
		var node: Trick = _nodes[node_name]
		print("Node: ", node.name, " | Input: ", node.input, " | Next: ", node.next)
