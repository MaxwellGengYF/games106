set_xmakever("2.7.8")
add_rules("mode.release", "mode.debug")
if is_arch("x64", "x86_64", "arm64") then
	-- disable ccache in-case error
	set_policy("build.ccache", false)
	-- set bin dir
	if is_mode("debug") then
		set_targetdir(path.join(os.projectdir(), "bin_debug"))
	else
		set_targetdir(path.join(os.projectdir(), "bin_release"))
	end
	includes("xmake_func.lua")
	target("glm")
	set_kind("headeronly")
	add_includedirs("external/glm/glm", {
		public = true
	})
	target_end()

	target("imgui")
	_config_project({
		project_kind = "static"
	})
	add_includedirs("external/imgui", {
		public = true
	})
	add_files("external/imgui/**.cpp")
	target_end()

	target("ktx")
	_config_project({
		project_kind = "static"
	})
	add_includedirs("external/ktx/include", {
		public = true
	})
	add_includedirs("external/ktx/other_include")
	local ktx_path = "external/ktx/lib/"
	local file_lists = {
		"hashlist",
		"hashtable",
		"swap",
		"texture",
		"filestream",
		"memstream",
		"checkheader"
	}
	for i, v in ipairs(file_lists) do
		add_files(ktx_path .. v .. '.c')
	end
	target_end()

	target("vulkan-base")
	_config_project({
		project_kind = "static"
	})
	add_syslinks("Gdi32", {public = true})
	add_files("base/**.cpp")
	add_deps("glm", "imgui", "ktx")
	add_includedirs("base", "external/tinygltf", {public = true})
	add_rules("games106_vulkan")
	target_end()

	-- see homework/xmake.lua
	includes("homework")
else
	target("_games106_illegal_env")
	set_kind("phony")
	on_load(function(target)
		utils.error("Only support windows-x64, linux-x86_64 or MacOSX-arm64. Please check your architecture or platform.")
	end)
	target_end()
end
