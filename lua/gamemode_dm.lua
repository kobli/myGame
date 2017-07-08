--------------- exposed functions in CPP --------------
--[[
ID createCharacter(xPos, yPos, zPos)
void removeWorldEntity(objectID)
void setClientControlledObjectID(sessionID, objectID)
ID getClientControlledObjectID(sessionID)
float(basic),float(affected) getEntityAttributeValue(objectID, attributeName)

TODO
map.getSpawnPoints
--]]

-------------------- Cpp interface --------------------


function onClientConnect(sessionID)
	local charID = createCharacter(0,20,0)
	setClientControlledObjectID(sessionID, charID)
end

function onClientDisconnect(sessionID)
	local charID = getClientControlledObjectID(sessionID)
	setClientControlledObjectID(sessionID, NULLID)
	removeWorldEntity(charID)
end

function onEntityEvent(entityID, componentT, created, destroyed)
	if componentT == ComponentType["AttributeStore"] and not created and not destroyed then
		print("HP updated!??!?")
		local realHP, virtualHP = getEntityAttributeValue(entityID, "health")
		if realHP == 0 then
			setClientControlledObjectID(0, NULLID)
			--removeWorldEntity(entityID)
			--local charID = createCharacter(0,20,0)
			--setClientControlledObjectID(0, charID)
		end
	end
end
