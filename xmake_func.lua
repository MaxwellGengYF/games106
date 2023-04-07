-- Global config
rule("games106_vulkan")
on_load(function(target)
	local function get_path()
		local path = os.getenv("VULKAN_SDK")
		if path then
			return path
		end
		return os.getenv("VK_SDK_PATH")
	end
	local vk_path = get_path()
	if not vk_path then
		return
	end
	target:add("linkdirs", path.join(vk_path, "Lib"))
	target:add("links", "vulkan-1")
	target:add("includedirs", path.join(vk_path, "Include"))
	if is_plat("windows") then
		target:add("defines", "VK_USE_PLATFORM_WIN32_KHR")
	elseif is_plat("linux") then
		target:add("defines", "VK_USE_PLATFORM_DIRECTFB_EXT")
	elseif is_plat("macosx") then
		target:add("defines", "VK_USE_PLATFORM_MACOS_MVK")
	end
end)
rule_end()

rule("games106_basic_settings")
on_config(function(target)
	local _, cc = target:tool("cxx")
	if is_plat("linux") then
		-- Linux should use -stdlib=libc++
		-- https://github.com/LuisaGroup/LuisaCompute/issues/58
		if (cc == "clang" or cc == "clangxx" or cc == "gcc" or cc == "gxx") then
			target:add("cxflags", "-stdlib=libc++", {
				force = true
			})
		end
	end
end)
on_load(function(target)
	local _get_or = function(name, default_value)
		local v = target:values(name)
		if v == nil then
			return default_value
		end
		return v
	end
	local project_kind = _get_or("project_kind", "phony")
	target:set("kind", project_kind)
	local c_standard = target:values("c_standard")
	local cxx_standard = target:values("cxx_standard")
	if type(c_standard) == "string" and type(cxx_standard) == "string" then
		target:set("languages", c_standard, cxx_standard)
	else
		target:set("languages", "clatest", "cxx20")
	end

	local enable_exception = _get_or("enable_exception", nil)
	if enable_exception then
		target:set("exceptions", "cxx")
	else
		target:set("exceptions", "no-cxx")
	end
	if is_plat("windows") then
		target:add("defines", "NOMINMAX")
	end
	if is_mode("debug") then
		target:set("runtimes", "MDd")
		target:set("optimize", "none")
		target:set("warnings", "none")
		target:add("cxflags", "/GS", "/Gd", {
			tools = {"clang_cl", "cl"}
		})
		target:add("cxflags", "/Zc:preprocessor", {
			tools = "cl"
		});
	else
		target:set("runtimes", "MD")
		target:set("optimize", "aggressive")
		target:set("warnings", "none")
		target:add("cxflags", "/GS-", "/Gd", {
			tools = {"clang_cl", "cl"}
		})
		target:add("cxflags", "/GL", "/Zc:preprocessor", {
			tools = "cl"
		})
	end
	if _get_or("use_simd", false) then
		target:add("vectorexts", "sse", "sse2")
	end
	if _get_or("no_rtti", false) then
		target:add("cxflags", "/GR-", {
			tools = {"clang_cl", "cl"}
		})
		target:add("cxflags", "-fno-rtti", "-fno-rtti-data", {
			tools = {"clang", "gcc"}
		})
	end
end)
rule_end()
-- In-case of submod, when there is override rules, do not overload
if _config_rules == nil then
	_config_rules = {"games106_basic_settings"}
end
if _disable_unity_build == nil then
	_disable_unity_build = not get_config("enable_unity_build")
end

if _configs == nil then
	_configs = {}
end
_configs["use_simd"] = true
_configs["enable_exception"] = true
function _config_project(config)
	if type(_configs) == "table" then
		for k, v in pairs(_configs) do
			set_values(k, v)
		end
	end
	if type(_config_rules) == "table" then
		add_rules(_config_rules)
	end
	if type(config) == "table" then
		for k, v in pairs(config) do
			set_values(k, v)
		end
	end
	local batch_size = config["batch_size"]
	if type(batch_size) == "number" and batch_size > 1 and (not _disable_unity_build) then
		add_rules("c.unity_build", {
			batchsize = batch_size
		})
		add_rules("c++.unity_build", {
			batchsize = batch_size
		})
	end
end
