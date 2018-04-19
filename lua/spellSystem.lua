require("lua/list")


MAPOBJID = 10

AttributeAffectorModifierType = {}
AttributeAffectorModifierType.ADD = 0
AttributeAffectorModifierType.MUL = 1

-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

EffectData = {}

function EffectData:new(effectID, castingTime, modifiedAttribute, affectorModifierType, modifierValue, permanent, period)
	local meta = {__index = EffectData}
	local value = {
		effectID 						 = effectID,
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

Config.Body.invocT = 0.6				-- invocation time [seconds]
Config.Body.minRadiusCoef = 0.1 -- minimum radius coefficient of the spell body sphere [?]
Config.Body.baseSpeed = 20			-- base traveling speed of the body [?]
Config.Body.baseRadius = 2			-- base radius of the body sphere [?]

Config.Spell.maxSpeed = 60			-- maximum traveling speed of the spell[?]
Config.Spell.maxRadius = 20			-- maximum radius of the spell body sphere [?]
Config.Spell.maxPower = 5				-- maximum power of the spell [?]

BODYEFFECTID = 0
Config.Effects = {}
Config.Effects["fire"] = EffectData:new(1, 1.3, "health", AttributeAffectorModifierType.ADD, -30, true, 0)
Config.Effects["water"] = EffectData:new(2, 1.3, "health", AttributeAffectorModifierType.ADD, -30, true, 0)
Config.Effects["heal"] = EffectData:new(3, 1.1, "health", AttributeAffectorModifierType.ADD, 20, true, 0)

Config.Wizard.maxBodiesAlive = 50


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
		walking = false,
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
	if(string.match(inc, "[^%s]+_now%s*.*") ~= nil) then
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
		if not List.empty(self.incantationQ) and not self.walking then 
			self:execIncantation(List.popleft(self.incantationQ))
		end
	end
	self:updateStatus()
end

function Wizard:updateStatus()
	local progress = 0
	if self.invocT ~= nil and self.invocT > 0 then
		progress = math.min(self.invocT, math.max(0, self.invocT-self.invocRemainT))
	end
	local p,r,s
	local effects = {}
	if self.spellInHands ~= nil then
		p = self.spellInHands:getPower()/Config.Spell.maxPower
		r = self.spellInHands:getRadius()/Config.Spell.maxRadius
		s = self.spellInHands:getSpeed()/Config.Spell.maxSpeed
		for k, v in pairs(self.spellInHands.effects) do
			table.insert(effects, v.effectID)
		end
	end
	local invocEffId = effectIdFromIncantation(self.invocIncantation)
	updateWizardStatus(self.ID, self.invocIncantation or "", invocEffId or -1, self.invocT, progress, 
	p, r, s, effects,
	Config.Wizard.maxBodiesAlive-self.bodiesInUse, Config.Wizard.maxBodiesAlive,
	self:getCommandQueueAsEffectIDs())
end

function Wizard:getCommandQueueAsEffectIDs()
	local r = {}
	for k,v in pairs(List.asTable(self.incantationQ)) do
		local commandProductID = effectIdFromIncantation(v)
		if commandProductID ~= nil then
			table.insert(r, commandProductID)
		end
	end
	return r
end

function effectIdFromIncantation(inc)
	if inc == nil then
		return nil
	end
	local c, args = incantationToCommandAndArgs(inc)
	if string.find(c, "spell_effect") then
		return Config.Effects[args].effectID
	elseif string.find(c, "spell_body") then
		return 0
	else
		return nil
	end
end

function incantationToCommandAndArgs(inc)
	return string.match(inc, "(spell_[^%s]+)%s*(.*)")
end

function Wizard:execIncantation(inc)
	command, argStr = incantationToCommandAndArgs(inc)
	if Wizard.Command[command] ~= nil then
		self.invocIncantation = inc
		self.invoc = coroutine.create(
		function(self, argStr)
			self.Command[command](self, argStr)
			self.invoc = nil
			self.invocIncantation = nil
			self.invocT = 0
			self.invocRemainT = 0
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
	self.invocIncantation = nil
	self.invocT = 0
	self.invocRemainT = 0
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

function Wizard:setWalking(walking)
	self.walking = walking
	if walking then 
		self:resetInvocation()
	end
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

function Spell:new(ID, author, baseBody)
	dout("spell created")
	local meta = {__index = Spell}
	local value = {
		ID = ID,
		author = author,
		baseBody = baseBody,
		bodies = {baseBody},
		effects = {},
		collisionsInLastTick = List.new()
	}
	return setmetatable(value, meta)
end

function Spell:destroy()
	self.ID = nil
	self.author = nil
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

function Spell:hasEffect(effectID)
		for k,v in pairs(self.effects) do
			if v.effectID == effectID then
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
	for k,v in pairs(List.asTable(self.collisionsInLastTick)) do
		local collidedWithEntType = entityIdToTypeName(v)
		if collidedWithEntType == "map" then
			if entityInGround(self.ID) then
				setEntityVelocity(self.ID, 0, 0, 0)
			end
		end
		if self:shouldDieOnCollisionWith(collidedWithEntType) then
			self:die()
			return
		end
	end
	self.collisionsInLastTick = List.new()
end

function Spell:handleCollision(otherID)
	List.pushright(self.collisionsInLastTick, otherID)
end

function Spell:shouldDieOnCollisionWith(entityTypeName)
	return self.baseBody:dieOnCollisionWith(entityTypeName)
end

function Spell:getPower()
	local s = 0
	for k,v in pairs(self.bodies) do
		s = s + v:getPower()
	end
	return math.min(stackedBodiesMultipltier(#self.bodies)*s, Config.Spell.maxPower)
end

function Spell:getRadius()
	local s = 0
	for k,v in pairs(self.bodies) do
		s = s + v:getRadius()
	end
	return math.min(stackedBodiesMultipltier(#self.bodies)*s, Config.Spell.maxRadius)
end

function Spell:getSpeed()
	local s = 0
	for k,v in pairs(self.bodies) do
		s = s + v:getSpeed()
	end
	return math.min(stackedBodiesMultipltier(#self.bodies)*s, Config.Spell.maxSpeed)
end

function Spell:die()
	-- on each colliding character
	for k,colEntID in pairs(List.asTable(self.collisionsInLastTick)) do
		dout("col with on death: ",colEntID)
		-- apply all effects
		for k,ed in pairs(self.effects) do
			addAttributeAffector(colEntID, self.author, ed.modifiedAttribute, ed.affectorModifierType, ed.modifierValue*self:getPower(), ed.permanent, ed.period);
		end
	end
	removeSpell(self.ID)
	SPELLS[self.ID] = nil
	self:destroy()
end

function Spell:getEffectID()
	if self.effects[1] ~= nil then
		return self.effects[1].effectID
	else
		return BODYEFFECTID
	end
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

function entityIdToTypeName(entID)
	what = ""
	if SPELLS[entID] ~= nil then
		what = "spell"
	elseif WIZARDS[entID] ~= nil then
		what = "player"
	else
		what = "map"
	end
	return what
end

function log(n, base)
	return math.log(n)/math.log(base)
end

function stackedBodiesMultipltier(bodyC)
	return 1
	--return (log(bodyC+1, 1.5)-0.71)/bodyC
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

function wizardWalking(wizID, walking)
	if WIZARDS[wizID] ~= nil then
		WIZARDS[wizID]:setWalking(walking)
	end
end

-------------------- commands --------------------

-- argStr: <power radius speed [ratio]> <die [<player> <map>]>
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
		self.spellInHands = Spell:new(nil, self.ID, body)
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
	local sEffectID = self.spellInHands:getEffectID()
	if sElevation == nil or sElevation > 90 or sElevation < -90 then
		sElevation = 0
		dout("incorrect spellElevation "..argStr.." - defaulting to 0")
	end
	local sID = wizardLaunchSpell(self.ID, sRadius, sSpeed, sElevation, sEffectID)
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
	local effect = Config.Effects[effectName]
	if effect.effectID == nil then
		dout("such effect does not exist")
		return
	end
	-- we need something to append the effect to
	if self.spellInHands == nil then
		dout("No spell to append the effect to.")
		return
	end
	--
	if self.spellInHands:hasEffect(effect.effectID) then
		dout("The currenly held spell already contains this effect.")
		return
	end
	dout("starting effect invocation...")
	-- invocation delay
	self:doInvocation(effect.castingTime)
	-- create and append the effect
	self.spellInHands:appendEffect(effect)
end
