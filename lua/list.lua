List = {}

function List.new ()
	return {first = 0, last = -1, data={}}
end

function List.pushleft (list, value)
	local first = list.first - 1
	list.first = first
	list.data[first] = value
end

function List.pushright (list, value)
	local last = list.last + 1
	list.last = last
	list.data[last] = value
end

function List.popleft (list)
	local first = list.first
	if first > list.last then error("list is empty") end
	local value = list.data[first]
	list.data[first] = nil        -- to allow garbage collection
	list.first = first + 1
	return value
end

function List.popright (list)
	local last = list.last
	if list.first > last then error("list is empty") end
	local value = list.data[last]
	list.data[last] = nil         -- to allow garbage collection
	list.last = last - 1
	return value
end

function List.empty (list)
	return list.first > list.last
end

function List.asTable (list)
	local r = {}
	for i=list.first, list.last, 1 do
		table.insert(r, list.data[i])
	end
	return r
end
