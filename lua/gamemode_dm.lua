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

OBJCONTROLLINGCLIENTS = {}

function onClientConnect(sessionID)
	local charID = createCharacter(0,20,0)
	setClientControlledObj(sessionID, charID)
end

function onClientDisconnect(sessionID)
	local charID = getClientControlledObjectID(sessionID)
	setClientControlledObj(sessionID, NULLID)
	removeWorldEntity(charID)
end

function onEntityEvent(entityID, componentT, created, destroyed)
	if componentT == ComponentType["AttributeStore"] and not created and not destroyed then
		print("HP updated!??!?")
		local realHP, virtualHP = getEntityAttributeValue(entityID, "health")
		print("HP: "..realHP)
		if realHP == 0 then
			local objOwner = OBJCONTROLLINGCLIENTS[entityID]
			if entityID ~= getClientControlledObjectID(objOwner) then
				print("error: clients controlled char ID probably changed in the engine")
			end
			print("CHARACTER DIED")
			setClientControlledObj(objOwner, NULLID)
			removeWorldEntity(entityID)
			print("SPAWNING NEW ONE ...")
			local charID = createCharacter(0,20,0)
			print("DONE")
			setClientControlledObj(objOwner, charID)
		end
	end
end

-----------------------------------------------------

function setClientControlledObj(sessionID, objID)
	if objID ~= NULLID then
		OBJCONTROLLINGCLIENTS[objID] = sessionID
	end
	setClientControlledObjectID(sessionID, objID)
end
