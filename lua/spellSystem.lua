-- TODO
-- when a spell hits a player and has no effect -> append its bodies to his spell
-- InvocationQ limit
require("lua/list")


MAPOBJID = 10

AttributeAffectorModifierType = {}
AttributeAffectorModifierType.ADD = 0
AttributeAffectorModifierType.MUL = 1

-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

EffectData = {}

function EffectData:new(castingTime, modifiedAttribute, affectorModifierType, modifierValue, permanent, period)
	local meta = {__index = EffectData}
	local value = {
		castingTime          = castingTime,
		modifiedAttribute    = modifiedAttribute,
		affectorModifierType = affectorModifierType,
		modifierValue        = modifierValue,
		permanent            = permanent,
		period               = period,
	}
	return setmetatable(value, meta)
end

-------------------- config --------------------

Config = {}
Config.Body = {}
Config.Spell = {}
Config.Effect = {}
Config.Wizard = {}

Config.Body.invocT = 2					-- invocation time [seconds]
Config.Body.minRadiusCoef = 0.1 -- minimum radius coefficient of the spell body sphere [?]
Config.Body.baseSpeed = 20			-- base traveling speed of the body [?]
Config.Body.baseRadius = 2			-- base radius of the body sphere [?]

Config.Spell.maxSpeed = 60			-- maximum traveling speed of the spell[?]
Config.Spell.maxRadius = 20			-- maximum radius of the spell body sphere [?]

Config.Effects = {}
Config.Effects["fire"] = EffectData:new(0.2, "health", AttributeAffectorModifierType.ADD, -30, true, 0)
Config.Effects["heal"] = EffectData:new(0.1, "health", AttributeAffectorModifierType.ADD, 20, true, 0)

Config.Wizard.maxBodiesAlive = 5


-------------------- globals --------------------
WIZARDS = {}
SPELLS = {}

-------------------- classes ------------------

Wizard = {Command = {}}

function Wizard:new(ID)
	local meta = {__index = Wizard}
	local value = {
		ID = ID,
		incantationQ = List.new(),
		bodiesInUse = 0,
		invoc = nil, 	-- coroutine - only when the wizard is invocating
		invocT = 0,
		invocRemainT = 0,
		invocIncantation = nil,
		spellInHands = nil,
	}
	return setmetatable(value, meta)
end

function Wizard:returnBody()
	if self.bodiesInUse == 0 then
		dout("return of a non-missing body")
	else
		self.bodiesInUse = self.bodiesInUse - 1
	end
end

function Wizard:handleIncantation(inc)
	dout(LF,"======= Spell system <handleIncantation> called =======",inc)
	dout("Wizard ID: "..self.ID)
	if(string.match(inc, "[^%s]+_now%s.+") ~= nil) then
		-- execute the _now commands immediately
		dout("exec now")
		self:resetInvocation()
		self:execIncantation(inc)
	else
		-- the others: just add them to the queue
		dout("exec later")
		List.pushright(self.incantationQ, inc)
	end
end

function Wizard:update(delta)
	if self.invoc ~= nil then
		self.invocRemainT = self.invocRemainT - delta
		if self.invocRemainT <= 0 then
			coroutine.resume(self.invoc)
		end
	else
		-- start invocation of the next incantation
		if not List.empty(self.incantationQ) then 
			self:execIncantation(List.popleft(self.incantationQ))
		end
	end
end

function Wizard:execIncantation(inc)
	command, argStr = string.match(inc, "(spell_[^%s]+)%s+(.*)")
	if Wizard.Command[command] ~= nil then
		self.invocIncantation = inc
		self.invoc = coroutine.create(
		function(self, argStr)
			self.Command[command](self, argStr)
			self.invoc = nil
		end
		)
		coroutine.resume(self.invoc, self, argStr)
	else
		dout("bad command: ", command)
	end
end

function Wizard:doInvocation(duration)
	self.invocT = duration
	self.invocRemainT = duration
	coroutine.yield()
end

function Wizard:abortInvocation()
	self.invoc = nil 
end

function Wizard:emptyInvocationQ()
	self.incantationQ = List.new()
end

function Wizard:resetInvocation()
	if self.invoc ~= nil then
		List.pushleft(self.incantationQ, self.invocIncantation)
	end
	self:abortInvocation()
end

function Wizard:cancelSpell()
	self.spellInHands:destroy()
	self.spellInHands = nil
end




-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

Body = {}

function Body:new(author, powerCoef, radiusCoef, speedCoef, dieOn)
	dout("body created")
	local meta = {__index = Body}
	local value = {
		author = author,
		powerCoef = powerCoef,
		radiusCoef = radiusCoef,
		speedCoef = speedCoef,
		dieOn = dieOn,
	}
	return setmetatable(value, meta)
end

function Body:destroy()
	self.author:returnBody()
	self.author = nil
	dout("body destroyed")
end

function Body:getPower()
	return self.powerCoef
end

function Body:getRadius()
	return self.radiusCoef*Config.Body.baseRadius
end

function Body:getSpeed()
	return self.speedCoef*Config.Body.baseSpeed
end

function Body:dieOnCollisionWith(str)
	return self.dieOn ~= nil and self.dieOn[str] ~= nil
end

-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

Effect = {}

function Effect:new(name)
	dout("effect created")
	local meta = {__index = Effect}
	local value = {
		name = name,
	}
	return setmetatable(value, meta)
end

-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

Spell = {}

function Spell:new(ID, baseBody)
	dout("spell created")
	local meta = {__index = Spell}
	local value = {
		ID = ID,
		baseBody = baseBody,
		bodies = {baseBody},
		effects = {},
		collisionsInLastTick = List.new()
	}
	return setmetatable(value, meta)
end

function Spell:destroy()
	self.ID = nil
	self.baseBody = nil
	for k,v in pairs(self.bodies) do
		v:destroy()
	end
	self.bodies = {};
	self.effects = {};
	self.collisionsInLastTick = List.new()
	dout("spell destroyed")
end

function Spell:appendEffect(effect)
	table.insert(self.effects, effect)
	dout("spell now contains "..#self.effects.." effects")
end

function Spell:hasEffect(effectName)
		for k,v in pairs(self.effects) do
			if v.name == effectName then
				return true
			end
		end
		return false
end

function Spell:appendBody(body)
	table.insert(self.bodies, body)
	dout("spell now contains "..#self.bodies.." bodies")
end

function Spell:update(delta)
	for i=self.collisionsInLastTick.first, self.collisionsInLastTick.last, 1 do
		if self:shouldDieOnCollisionWith(self.collisionsInLastTick[i]) then
			self:die()
			return
		end
	end
	self.collisionsInLastTick = List.new()
end

function Spell:handleCollision(otherID)
	List.pushright(self.collisionsInLastTick, otherID)
end

function Spell:shouldDieOnCollisionWith(entID)
	what = ""
	if entID == MAPOBJID then
		what = "terrain"
	elseif entID == self.ID then
		dout("spell hit itself - WTF?")
	elseif SPELLS[entID] ~= nil then
		what = "spell"
	else
		what = "player"
	end
	return self.baseBody:dieOnCollisionWith(what)
end

function Spell:getPower()
	local s = 0
	for k,v in pairs(self.bodies) do
		s = s + v:getPower()
	end
	return s
end

function Spell:getRadius()
	local s = 0
	for k,v in pairs(self.bodies) do
		s = s + v:getRadius()
	end
	return math.min(s, Config.Spell.maxRadius)
end

function Spell:getSpeed()
	local s = 0
	for k,v in pairs(self.bodies) do
		s = s + v:getSpeed()
	end
	return math.min(s, Config.Spell.maxSpeed)
end

function Spell:die()
	-- on each colliding character
	for i=self.collisionsInLastTick.first, self.collisionsInLastTick.last, 1 do
		dout("col with on death: ",self.collisionsInLastTick[i])
		-- apply all effects
		for k,v in pairs(self.effects) do
			local ed = Config.Effects[v.name]
			addAttributeAffector(self.collisionsInLastTick[i], ed.modifiedAttribute, ed.affectorModifierType, ed.modifierValue*self:getPower(), ed.permanent, ed.period);
		end
	end
	removeSpell(self.ID)
	SPELLS[self.ID] = nil
	self:destroy()
end

-------------------- helpers --------------------

LF = "\n"
function dout(...)
	io.write("SS: ")
	for i = 1, select("#",...) do
		io.write(tostring(select(i,...))..LF)
	end
end

-- example input: " 0.5 die{sfd,bla} 23"
-- in the result single values can be accessed under indexes (from 1) (in the example 1=0.5, 2=23)
-- sets can be accessed by their name, here die
-- if the set does not have a name, it behaves like single value
function parseBodyArgStr(argStr)
	r = {}
	for i in string.gmatch(argStr,"%S+") do
		key, valStr = string.match(i,"(.*)%{(.*)%}")
		if key ~= nil and key ~= "" then
			values = {}
			for i in string.gmatch(valStr, "[^,]+") do
				values[i] = true
			end
			r[key] = values
		else
			table.insert(r, i)
		end
	end
	return r
end

-------------------- Cpp interface --------------------
-- the ID is unique only for the cpp interface (it is an entity ID - they might be later reused)
-- lifetime of Wizard object is not the same (usually is longer) as lifetime of WizardComponent in the engine
-- update is not called on removed wizards but someone elses update might call a dead wizards method,
-- which might do a callback to the engine, if the wizards ID was valid

function addWizard(ID)
	if WIZARDS[ID] ~= nil then -- wizard with same ID exists
		dout("addWizard: ID "..ID.." is in use. Replacing")
		removeWizard(ID)
	end
	WIZARDS[ID] = Wizard:new(ID)
	dout("Added wizard "..ID)
end

function removeWizard(ID)
	if WIZARDS[ID] == nil then
		dout("removeWizard: trying to remove non-existing wizard ("..ID..")")
	else
		WIZARDS[ID].ID = nil
		WIZARDS[ID] = nil
		dout("Removed wizard "..ID)
	end
end

function handleIncantation(wizID, inc)
	if WIZARDS[wizID] == nil then
		dout("handleIncantation: wizard ID "..wizID.." does not exist")
	else
		WIZARDS[wizID]:handleIncantation(inc)
	end
end

function update(delta)
	for k,v in pairs(WIZARDS) do
		v:update(delta)
	end
	for k,v in pairs(SPELLS) do
		v:update(delta)
	end
end

function handleCollision(obj1ID, obj2ID)
	if SPELLS[obj1ID] ~= nil then
		SPELLS[obj1ID]:handleCollision(obj2ID)
	end
end

-------------------- commands --------------------

-- argStr: <power radius speed [ratio]> <die [<player> <terrain>]>
function Wizard.Command:spell_body_create(argStr)
	dout("create body: "..argStr)
	-- validate and parse arg str
	local args = parseBodyArgStr(argStr)
	local powerCoef = math.abs(tonumber(args[1]))
	local radiusCoef = math.abs(tonumber(args[2]))
	local speedCoef = math.abs(tonumber(args[3]))
	dout(powerCoef, radiusCoef, speedCoef)
	if powerCoef ~= nil and radiusCoef ~= nil and speedCoef ~= nil then
		local denom = powerCoef + radiusCoef + speedCoef
		powerCoef = powerCoef/denom
		radiusCoef = radiusCoef/denom
		speedCoef = speedCoef/denom	
		if radiusCoef < Config.Body.minRadiusCoef then 
			dout("spell body radiusCoef is too small")
			return
		end
	else
		dout("invalid argument string")
		return
	end
	-- check body limit
	if self.bodiesInUse >= Config.Wizard.maxBodiesAlive then 
		dout("not enough free bodies")
		return
	end
	-- invocation delay
	self:doInvocation(Config.Body.invocT)
	local body = Body:new(self, powerCoef, radiusCoef, speedCoef, args["die"])
	if body == nil then
		return
	end
	self.bodiesInUse = self.bodiesInUse + 1	
	if self.spellInHands == nil then
		self.spellInHands = Spell:new(nil, body)
	else
		self.spellInHands:appendBody(body)
	end
end
Wizard.Command.spell_body_create_now = Wizard.Command.spell_body_create --TODO remove

function Wizard.Command:spell_abort_now()
	dout("abort casting and empty the Q")
	self:abortInvocation()
	self:emptyInvocationQ()
end

function Wizard.Command:spell_cancel_now()
	dout("cancel spell and reset casting")
	self:cancelSpell()
	self:resetInvocation()
end

-- argStr: <elevation angle (-90 - 90)>
function Wizard.Command:spell_launch_direct_now(argStr)
	if self.spellInHands == nil then
		dout("no spell to launch")
		return
	end
	dout("launching the spell")
	local sRadius = self.spellInHands:getRadius()
	local sSpeed = self.spellInHands:getSpeed()
	local sElevation = tonumber(argStr)
	if sElevation == nil or sElevation > 90 or sElevation < -90 then
		sElevation = 0
		dout("incorrect spellElevation "..argStr.." - defaulting to 0")
	end
	local sID = wizardLaunchSpell(self.ID, sRadius, sSpeed, sElevation)
	if sID ~= 0 then
		dout("probably launched")
		self.spellInHands.ID = sID
		SPELLS[sID] = self.spellInHands
	else
		dout("launching the spell failed in the engine")
	end
	self.spellInHands = nil
end

-- argStr: <effect name>
function Wizard.Command:spell_effect_create(argStr)
	dout("create effect: "..argStr)
	-- validate and parse arg str
	local effectName = argStr
	if Config.Effects[effectName] == nil then
		dout("such effect does not exist")
		return
	end
	-- we need something to append the effect to
	if self.spellInHands == nil then
		dout("No spell to append the effect to.")
		return
	end
	--
	if self.spellInHands:hasEffect(effectName) then
		dout("The currenly held spell already contains this effect.")
		return
	end
	dout("starting effect invocation...")
	-- invocation delay
	self:doInvocation(Config.Effects[effectName].castingTime)
	-- create and append the effect
	local effect = Effect:new(effectName)
	self.spellInHands:appendEffect(effect)
end
