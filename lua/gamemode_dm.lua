--------------- exposed functions in CPP --------------
--[[
ID createCharacter(xPos, yPos, zPos)
void removeWorldEntity(objectID)
void setClientControlledObjectID(sessionID, objectID)
ID getClientControlledObjectID(sessionID)
float(basic),float(affected) getEntityAttributeValue(objectID, attributeName)
void setEntityAttributeValue(objectID, attributeName, floatValue)

TODO
map.getSpawnPoints
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
	local charID = createCharacter(0,20,0)
	setEntityAttributeValue(charID, "health", 10)
	setEntityAttributeValue(charID, "max-health", 100)
	return charID
end

function onMaybeHPchanged(entityID)
		local realHP, virtualHP = getEntityAttributeValue(entityID, "health")
		if realHP == 0 then
			onCharacterDeath(entityID)
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
