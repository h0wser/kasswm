config = {
	border_normal_color = 0x220022,
	border_width = 3
}

function get_focused()
	for k, v in pairs(kass.clients) do
		if (v:hasfocus()) then return v end
	end
end

function kass.setup()
	print "hello from setup"
	kass.key.bind("r", {"ctrl"}, function()
		print("Hello my friend")
	end)

	kass.key.bind("return", {"mod4"}, function()
		os.execute("urxvt&")
	end)

	kass.key.bind("s", {}, function()
		get_focused():raise()
	end)

	kass.key.bind("d", {}, function()
		get_focused():lower()
	end)

	kass.key.bind("a", {"ctrl"}, function()
		get_focused():move_relative(20, 0)
	end)
end

function kass.new_window(window)

	window:move(math.random(400), math.random(400))
	window:resize(math.random(400), math.random(400))
	window:show()
	window:focus()

	print("window data: pos:", window:getx(), window:gety())
	print("window data: size:", window:getw(), window:geth())
	print("window data: mapped:", window:mapped())
	print("window data: focus:", window:hasfocus())
end

print("Loaded config")
