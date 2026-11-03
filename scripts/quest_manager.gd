extends Node

@export var quests: Array[Quest]
var _current_quest: Quest = null
var _quest_progression := 0
var appear_tw : Tween
var completed_first_quest := false

func _ready() -> void:
	TrickManager.trick_performed.connect(_on_trick)
	TrickManager.trick_ended.connect(_reset_quest)
	TrickManager.trick_catched.connect(_on_catch)

func _on_catch(_progression: float, _dt: float) -> void:
	_reset_quest()

func _on_quest_end_timeout() -> void:
	_move_quests(680)
	_current_quest = null
	_quest_progression = 0
	$NewQuest.start()

func _on_trick(_name: String, _accuracy: float) -> void:
	if not _current_quest: return

	if _name == _current_quest.steps[_quest_progression]:
		get_node("../UI/Input" + str(_quest_progression + 1)).correct()
		_quest_progression += 1
		TrickManager._xp_buffer += _current_quest.reward
		if _quest_progression >= len(_current_quest.steps):
			_move_quests(680)
			_current_quest = null
			_quest_progression = 0
		$NewQuest.start()
		$QuestEnd.stop()
	else:
		_reset_quest()

func _on_new_quest_timeout() -> void:
	_current_quest = quests.pick_random()
	while _current_quest.reward > 100 and not completed_first_quest:
		_current_quest = quests.pick_random()
	print(_current_quest.reward)
	for i in range(len(_current_quest.steps)):
		var input : String = TrickManager._tricks.get_node(_current_quest.steps[i]).input
		get_node("../UI/Input" + str(i + 1)).set_input_ref(input)
	_move_quests(550)
	$QuestEnd.start()

func _move_quests(target_y: float) -> void:
	for i in range(len(_current_quest.steps)):
		var whom = get_node("../UI/Input" + str(i + 1))
		get_tree().create_tween().tween_property(whom, "position:y", target_y, 1.4).set_trans(Tween.TRANS_ELASTIC).set_ease(Tween.EASE_OUT).set_delay(0.3 * i)

func _reset_quest() -> void:
	for i in range(_quest_progression):
		get_node("../UI/Input" + str(i + 1)).incorrect()
	_quest_progression = 0
