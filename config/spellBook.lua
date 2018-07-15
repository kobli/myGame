-- this is a comment - it is ignored
-- here you can create variables, which you can use later to define spells

-- order of spell_body_create coefficients: power radius speed
staticBody = "spell_body_create 1 1 0 die{player}"
bigMovingBody = "spell_body_create 2 3 1 die{player}"
fastMovingBody = "spell_body_create 1 1 3 die{map,player}"
fastestBodyNoPower = "spell_body_create 0 1 9 die{map,player}"
fireEffect = "spell_effect_create fire"


-- this is the list of spells that you will be able to use in game
-- each spell has a name and a sequence of steps it takes to cast it
spellBook = {
	["Big fireball"] = {bigMovingBody, fireEffect},
	["Fast fireball"] = {fastMovingBody, fireEffect},
	["Static fireball"] = {staticBody, fireEffect},
	["Static body"] = {staticBody},
	["Fastest body"] = {fastestBodyNoPower},
	["Heal"] = {staticBody, "spell_effect_create heal"},
}
