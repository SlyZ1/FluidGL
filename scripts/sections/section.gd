@tool
extends Node3D

class_name Section

@export_group("Editor")
@export var street_settings: StreetSettings
@export var num_street: int
@export_tool_button("Generate", "Add") var generate_button = _generate
var _streets: Array[Node3D] = []
var _street_container: Node3D

@export_group("Game")
@export var snap_back: Node3D
@export var snap_front: Node3D
@export var rail_containers: Array[Node]

var rails: Array[Path3D]

func _ready() -> void:
	for container in rail_containers:
		for path in container.find_children("*", "Path3D", true, false):
			rails.append(path)

func snap(snap_to: Vector3, parent: Node3D) -> void:
	position = parent.to_local(snap_to) - basis * snap_back.position

func _generate() -> void:
	# Instantie street container si existe pas
	if not _street_container:
		# Si ya déjà un street container dans la scene on le supprime
		var previous_container := find_child("Street Container") as Node3D
		if previous_container: 
			remove_child(previous_container)
			previous_container.queue_free()
			
		_street_container = Node3D.new()
		_street_container.name = "Street Container"
		add_child(_street_container)
		_street_container.owner = self
	
	# Supprime les anciennes streets
	for street in _streets:
		_street_container.remove_child(street)
		street.queue_free()
	_streets = []
	
	# Crée les nouvelles
	for i in range(num_street):
		var instance: Node3D = street_settings.street.instantiate() as Node3D
		_street_container.add_child(instance)
		instance.position = Vector3.RIGHT * street_settings.street_half_size * 2 * i
		instance.owner = self
		instance.name = "Street %d" % i
		_streets.append(instance)
	snap_back.position = - Vector3.RIGHT * (street_settings.street_half_size - street_settings.snappers_inset)
	snap_front.position = Vector3.RIGHT * (street_settings.street_half_size * (2 * num_street - 1) - street_settings.snappers_inset)
