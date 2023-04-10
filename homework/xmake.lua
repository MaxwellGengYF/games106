do
	target("shader_compiler")
	_config_project({
		project_kind = "binary",
	})
	add_files("shader_compiler/**.cpp")
	add_rules("platform_defines")
	add_defines("SHADER_COMPILER_EXPORT")
	target_end()
end

do
	target("homework")
	_config_project({
		project_kind = "binary",
		enable_exception = true
	})
	add_files("homework0/*.cpp")
	add_deps("vulkan-base")
	add_rules("games106_vulkan")
	target_end()
end
