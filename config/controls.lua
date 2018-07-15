require "./config/keymap"
-- You can edit this file to adjust controlls.
-- See keymap.lua for key names.

settings = {
	["NAME"] = "My name"
}

controls = {
	[KEY_W] = "FORWARD",
	[KEY_S] = "BACKWARD",
	[KEY_A] = "LEFT",
	[KEY_D] = "RIGHT",

	[ESCAPE] = "spell_abort_now",
	[LBUTTON] = "LAUNCH_SPELL_DIRECT",
	[KEY_F] = "CAST Big fireball",
	[KEY_G] = "CAST Fast fireball",
	[KEY_H] = "CAST Static fireball",
	[KEY_B] = "CAST Static body",
	[KEY_V] = "CAST Fastest body",
	[KEY_Q] = "CAST Heal",
}
