-- TODO
-- apply effect on body death
-- when a spell hits a player and has no effect -> append its bodies to his spell
-- InvocationQ limit
require("lua/list")

-------------------- config --------------------

Config = {}
Config.Body = {}

Config.Body.invocT = 2		-- invocation time [seconds]
Config.Body.maxC = 50			-- maximum # of bodies 
Config.Body.maxSpeed = 50			-- maximum traveling speed of the body [?]
Config.Body.maxRadius = 2			-- maximum radius of the body sphere [?]

--Stats = {}


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
		dout("return of a not-missing body")
	else
		self.bodiesInUse = self.bodiesInUse + 1
	end
end

function Wizard:handleIncantation(inc)
	dout(LF,"======= Spell system <handleIncantation> called =======",inc)
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

function Body:new(author, radSpeedRatio, dieOn)
	dout("body created")
	local meta = {__index = Body}
	local value = {
		author = author,
		radSpeedRatio = radSpeedRatio,
		dieOn = dieOn,
	}
	return setmetatable(value, meta)
end

function Body:destroy()
	self.author:returnBody()
	self.author = nil
	dout("body destroyed")
end

function Body:getRadius()
	return self.radSpeedRatio*Config.Body.maxRadius
end

function Body:getSpeed()
	return (1-self.radSpeedRatio)*Config.Body.maxSpeed
end

function Body:dieOnCollisionWith(str)
	return self.dieOn ~= nil and self.dieOn[str] ~= nil
end

-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

Spell = {}

function Spell:new(ID, baseBody)
	dout("spell created")
	local meta = {__index = Spell}
	local value = {
		ID = ID,
		baseBody = baseBody,
		bodies = {baseBody}
	}
	return setmetatable(value, meta)
end

function Spell:destroy()
	baseBody = nil
	for k,v in pairs(self.bodies) do
		v:destroy()
		self.bodies[k] = nil
	end
	dout("spell destroyed")
end

function Spell:appendBody(body)
	table.insert(self.bodies, body)
	dout("spell now contains "..#self.bodies.." bodies")
end

function Spell:handleCollision(otherID)
	what = ""
	if otherID == 0 then
		what = "terrain"
	elseif otherID == self.ID then
		dout("spell hit itself - WTF?")
	elseif SPELLS[otherID] ~= nil then
		what = "spell"
	else
		what = "player"
	end
	dout("spell "..self.ID.." hit "..what)
	if self.baseBody:dieOnCollisionWith(what) then
		self:die()
	end
end

function Spell:die()
	--TODO if applyEffectOnDeath ... 
	removeSpell(self.ID)
	SPELLS[self.ID] = nil
	self.ID = nil
	self:destroy()
end

-------------------- globals --------------------
WIZARDS = {}
SPELLS = {}

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
end

function removeWizard(ID)
	if WIZARDS[ID] == nil then
		dout("removeWizard: trying to remove non-existing wizard ("..ID..")")
	else
		WIZARDS[ID].ID = nil
		WIZARDS[ID] = nil
	end
end

function handleIncantation(wizID, inc)
	if WIZARDS[wizID] == nil then
		dout("handleIncantation: wizard ID "..wizID.." does not exist")
	else
		WIZARDS[wizID]:handleIncantation(inc)
	end
end

--TODO rename to updateWizards
function update(delta)
	for k,v in pairs(WIZARDS) do
		v:update(delta)
	end
end

function handleCollision(obj1ID, obj2ID)
	if SPELLS[obj1ID] ~= nil then
		SPELLS[obj1ID]:handleCollision(obj2ID)
	end
end

-------------------- commands --------------------

-- TODO argStr: <radius/speed ratio [0.-1.]> <die [<player> <terrain>]>
function Wizard.Command:spell_body_create(argStr)
	dout("create body: "..argStr)
	-- validate and parse arg str
	local args = parseBodyArgStr(argStr)
	local radSpeedRatio = tonumber(args[1])
	dout(radSpeedRatio)
	if radSpeedRatio ~= nil then
		if radSpeedRatio >= 0.1 and radSpeedRatio <= 1 then 
			dout("OK")
		else
			dout("wrong radSpeedRatio")
		end
	else
		dout("invalid argument string")
		return
	end
	-- check body limit
	if self.bodiesInUse >= Config.Body.maxC then 
		dout("not enough free bodies")
		return
	end
	-- invocation delay
	self:doInvocation(Config.Body.invocT)
	local body = Body:new(self, radSpeedRatio, args["die"])
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

function Wizard.Command:spell_launch()
	if self.spellInHands == nil then
		dout("no spell to launch")
		return
	end
	dout("launching the spell")
	local sRadius = self.spellInHands.baseBody:getRadius()
	local sSpeed = self.spellInHands.baseBody:getSpeed()
	local sID = wizardLaunchSpell(self.ID, sRadius, sSpeed)
	if sID ~= 0 then
		dout("probably launched")
		self.spellInHands.ID = sID
		SPELLS[sID] = self.spellInHands
	else
		dout("launching the spell failed in the engine")
	end
	self.spellInHands = nil
end
Wizard.Command.spell_launch_now = Wizard.Command.spell_launch
