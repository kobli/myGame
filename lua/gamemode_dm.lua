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
--]]

-------------------- Cpp interface --------------------

OBJCONTROLLINGCLIENTS = {}

function onClientConnect(sessionID)
	setClientControlledObj(sessionID, spawnCharacter())
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
	local spawns = getSpawnpoints()
	local s = spawns[1]
	print(s[1],s[2],s[3])
	local charID = createCharacter(s[1],s[2],s[3])
	setEntityAttributeValue(charID, "health", 20)
	setEntityAttributeValue(charID, "max-health", 50)
	return charID
end

function onMaybeHPchanged(entityID)
	local realHP, virtualHP = getEntityAttributeValue(entityID, "health")
	local realMaxHP, virtualMaxHP = getEntityAttributeValue(entityID, "max-health")
	if realHP ~= nil and realMaxHP ~= nil then
		if realHP < 1 then
			onCharacterDeath(entityID)
		elseif realHP > realMaxHP then
			setEntityAttributeValue(entityID, "health", realMaxHP)
		end
	end
end

function onCharacterDeath(entityID)
	local objOwner = OBJCONTROLLINGCLIENTS[entityID]
	if entityID ~= getClientControlledObjectID(objOwner) then
		print("error: clients controlled char ID probably changed in the engine")
	end
	setClientControlledObj(objOwner, NULLID)
	removeWorldEntity(entityID)
	setClientControlledObj(objOwner, spawnCharacter())
end

function getSpawnpoints()
	local spawns = getEntitiesByAttributeValue("spawnpoint", "")
	local r = {}
	for i,sID in pairs(spawns) do
		table.insert(r, {getEntityPosition(sID)})
	end
	return r
end
