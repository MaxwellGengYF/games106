target("homework")
_config_project({
	project_kind = "binary"
})
add_files("homework0/*.cpp")
add_deps("vulkan-base")
add_rules("games106_vulkan")
target_end()
