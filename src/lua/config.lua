config = {
	border_normal_color = 0x220022,
	border_width = 3
}

function kass.setup()
	print "hello from setup"
end

function kass.new_window(window)
	print("new window created: ", window.id)
	print("list of windows:")
	for k, v in pairs(kass.clients) do print(k, v) end
end

function kass.key_press(key)

end
