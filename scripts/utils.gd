class_name Utils

static func collider_has_layer(collider: CollisionObject3D, layer: int) -> bool:
	return (collider.collision_layer & (1 << (layer - 1))) != 0