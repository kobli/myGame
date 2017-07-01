-- this is a comment - it is ignored
-- here you can create variables, which you can use later to define spells

staticBody = "spell_body_create 1"
bigMovingBody = "spell_body_create 0.9 die{player}"
fastMovingBody = "spell_body_create 0.4 die{player}"
fireEffect = "spell_effect_create fire"


-- this is the list of spells that you will be able to use in game
-- each spell has a name and a sequence of steps it takes to cast it
spellBook = {
	["Big fireball"] = {bigMovingBody, fireEffect},
	["Fast fireball"] = {fastMovingBody, fireEffect},
}
