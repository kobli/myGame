--------------- exposed functions in CPP --------------
--[[
ID createCharacter(xPos, yPos, zPos)
void removeWorldEntity(objectID)
void setClientControlledObjectID(sessionID, objectID)
ID getClientControlledObjectID(sessionID)

TODO
attrStore.getAttribute
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
	-- when HP changed, check if character died
end
