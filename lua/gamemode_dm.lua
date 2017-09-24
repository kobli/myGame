--------------- exposed functions in CPP --------------
--[[
ID createCharacter(xPos, yPos, zPos)
void removeWorldEntity(objectID)
void setClientControlledObjectID(sessionID, objectID)
ID getClientControlledObjectID(sessionID)
float(basic),float(affected) getEntityAttributeValue(objectID, attributeName)
void setEntityAttributeValue(objectID, attributeName, floatValue)
[entityID] getEntitiesByAttributeValue(attributeName, attributeValue = "")
xPos,yPos,zPos getEntityPosition(objectID)
void setSharedRegValue(sessionID, key, value)
[author,attributeName,modifierType,modifierValue,permanent] getAttributeModifierHistory(entityID)
void removeBodyComponent(entityID)
void addBodyComponent(entityID, posX, posY, posZ)

the gm_info_template string can contain | to separate logical parts and <key> to substite the value of the key
--]]

Config = {}
Config.mapKillCount = 15
-------------------- Cpp interface --------------------

OBJCONTROLLINGCLIENTS = {}

function onClientConnect(sessionID)
	setClientControlledObj(sessionID, spawnCharacter())
	setSharedRegValue(sessionID, "gm_info_template", string.format(" Deathmatch | <killCount> / %i | <gm_map_time_left> ", Config.mapKillCount))
end

function onClientDisconnect(sessionID)
	local charID = getClientControlledObjectID(sessionID)
	setClientControlledObj(sessionID, NULLID)
	removeWorldEntity(charID)
end

function onEntityEvent(entityID, componentT, created, destroyed)
	if componentT == ComponentType["AttributeStore"] and not created and not destroyed then
		onMaybeHPchanged(entityID)
	end
end

-----------------------------------------------------

function setClientControlledObj(sessionID, objID)
	if objID ~= NULLID then
		OBJCONTROLLINGCLIENTS[objID] = sessionID
	end
	setClientControlledObjectID(sessionID, objID)
end

function spawnCharacter()
	local charID = createCharacter(chooseSpawnPosition())
	setEntityAttributeValue(charID, "health", 20)
	setEntityAttributeValue(charID, "max-health", 50)
	setEntityAttributeValue(charID, "killCount", 0)
	return charID
end

function chooseSpawnPosition()
	local spawns = getSpawnpoints()
	local s = spawns[math.random(#spawns)]
	return s[1],s[2],s[3]
end

function onMaybeHPchanged(entityID)
	local realHP, virtualHP = getEntityAttributeValue(entityID, "health")
	local realMaxHP, virtualMaxHP = getEntityAttributeValue(entityID, "max-health")
	if realHP ~= nil and realMaxHP ~= nil then
		if realHP < 1 then
			characterDie(entityID)
		elseif realHP > realMaxHP then
			setEntityAttributeValue(entityID, "health", realMaxHP)
		end
	end
end

function characterDie(entityID)
	onCharacterDeath(entityID)
	removeBodyComponent(entityID)
	respawn(entityID)
end

function onCharacterDeath(entityID)
	giveScoreForKilling(entityID)
	local objOwner = OBJCONTROLLINGCLIENTS[entityID]
	if entityID ~= getClientControlledObjectID(objOwner) then
		print("error: clients controlled char ID probably changed in the engine")
	end
end

function respawn(entityID)
	addBodyComponent(entityID, chooseSpawnPosition())
	setEntityAttributeValue(entityID, "health", 20)
end

function getSpawnpoints()
	local spawns = getEntitiesByAttributeValue("spawnpoint", "")
	local r = {}
	for i,sID in pairs(spawns) do
		table.insert(r, {getEntityPosition(sID)})
	end
	return r
end

function giveScoreForKilling(entityID)
	local lethalHit = {}
	for k,v in pairs(getAttributeModifierHistory(entityID)) do
		if v["attributeName"] == "health" and v["modifierValue"] < 0 and v["permanent"] == true then
			lethalHit = v
			break
		end
	end
	local killer = lethalHit["author"]
	if killer == entityID then
		print("Good job, you have killed yourself.")
	else
		local killC = getEntityAttributeValue(killer, "killCount") + 1
		setEntityAttributeValue(killer, "killCount", killC)
		if killC == Config.mapKillCount then
			--endRound()
		end
	end
end
