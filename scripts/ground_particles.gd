extends CPUParticles3D

@export var skate: Node3D

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	emitting = skate.is_grounded()
	initial_velocity_min = MapLoader.s_speed.length() * 1.5
	initial_velocity_max = MapLoader.s_speed.length() * 1.5
	#mat.direction = Vector3(-1, 0, 0)
