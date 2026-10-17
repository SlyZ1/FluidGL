extends Node3D

class_name MapLoader

@export var spawn_distance = 20
@export var kill_distance = 20
@export var angle: float = -15
@export var speed: float = 1
@export var container: Node3D
@export var sections: Array[PackedScene]
@export var skate: Node3D

static var s_speed: Vector3 = Vector3.ZERO

var _spawned_sections: Array[Section] = []
# Section "fantome" pour le prochain spawn
var _next_spawn: Section

func _ready() -> void:
	
	_next_spawn = _new_section(Vector3.ZERO)
	
func _new_section(snap_to: Vector3) -> Section:
	var new_section: Section = sections[0].instantiate() as Section
	container.add_child(new_section)
	new_section.snap(snap_to)
	new_section.visible = false
	return new_section

func _control_spawn():
	# Spawn le prochain fantome
	if _next_spawn == null && _spawned_sections.size() > 0:
		_next_spawn = _new_section(_spawned_sections.back().snap_front.global_position)
	
	# Check si le fantome est valide
	if _next_spawn && _next_spawn.snap_back.global_position.length_squared() <= spawn_distance * spawn_distance:
		_next_spawn.visible = true
		_spawned_sections.append(_next_spawn)
		_next_spawn = null
		
	# Check si le plus vieux est hors cam
	var front_section: Section = _spawned_sections.front()
	if (front_section 
		and front_section.snap_front.global_position.length_squared() > kill_distance * kill_distance
		and front_section.snap_front.global_position.dot(Vector3(1,0,0)) < 0
	):
		front_section.queue_free()
		_spawned_sections.pop_front()
	
func _control_speed(delta: float):
	for section: Section in _spawned_sections:
		section.position -= speed * Vector3.RIGHT * delta
	if _next_spawn: _next_spawn.position -= speed * Vector3.RIGHT * delta
	if _spawned_sections.size() > 0: s_speed = speed * _spawned_sections[0].global_basis.x

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	container.rotation_degrees.z = angle
	_control_spawn()
	_control_speed(delta)
