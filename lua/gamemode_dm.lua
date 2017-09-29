--------------- exposed functions in CPP --------------
--[[
float(basic),float(affected) getEntityAttributeValue(objectID, attributeName)
void setEntityAttributeValue(objectID, attributeName, floatValue)
[entityID] getEntitiesByAttributeValue(attributeName, attributeValue = "")
xPos,yPos,zPos getEntityPosition(objectID)
[author,attributeName,modifierType,modifierValue,permanent] getAttributeModifierHistory(entityID)
void removeBodyComponent(entityID)
void addBodyComponent(entityID, posX, posY, posZ)
void setGameRegValue(key, value)

the gm_info_template string can contain | to separate logical parts and <key> to substite the value of the key
--]]

Config = {}
Config.mapKillCount = 15
-------------------- Cpp interface --------------------

function onGameStart()
	setGameRegValue("gm_info_template", string.format(" Deathmatch | <killCount> / %i ", Config.mapKillCount))
end

function onPlayerJoined(charID)
	setEntityAttributeValue(charID, "max-health", 50)
	setEntityAttributeValue(charID, "killCount", 0)
	characterDie(charID)
end

function onPlayerLeft(charID)
end

function onEntityEvent(entityID, componentT, created, destroyed)
	if componentT == ComponentType["AttributeStore"] and not created and not destroyed then
		onMaybeHPchanged(entityID)
	end
end

-----------------------------------------------------

function chooseSpawnPosition()
	local spawns = getSpawnpoints()
	local s = spawns[math.random(#spawns)]
	return s[1],s[2],s[3]
end

function getSpawnpoints()
	local spawns = getEntitiesByAttributeValue("spawnpoint", "")
	local r = {}
	for i,sID in pairs(spawns) do
		table.insert(r, {getEntityPosition(sID)})
	end
	return r
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
end

function respawn(entityID)
	addBodyComponent(entityID, chooseSpawnPosition())
	setEntityAttributeValue(entityID, "health", 20)
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
	elseif killer ~= nil then
		local killC = getEntityAttributeValue(killer, "killCount") + 1
		setEntityAttributeValue(killer, "killCount", killC)
		if killC == Config.mapKillCount then
			--endRound()
		end
	end
end
