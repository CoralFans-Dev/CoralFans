add_rules("mode.debug", "mode.release")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")
add_repositories("oeotyan-repo https://github.com/OEOTYAN/xmake-repo.git")
add_repositories("coralfansdev-repo https://github.com/CoralFans-Dev/xmake-repo.git")

-- add_requires("levilamina x.x.x") for a specific version
-- add_requires("levilamina develop") to use develop version
-- please note that you should add bdslibrary yourself if using dev version
-- if is_config("target_type", "server") then
--     add_requires("levilamina 1.0.0-rc.1", {configs = {target_type = "server"}})
-- else
--     add_requires("levilamina 1.0.0-rc.1", {configs = {target_type = "client"}})
-- end

add_requires(
    "levilamina 1.4.1", {configs = {target_type = "server"}},
    "levibuildscript",
    "bsci main"
)

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

target("CoralFans") -- Change this to your mod name.
    add_rules("@levibuildscript/linkrule")
    add_cxflags(
        "/EHa",
        "/utf-8",
        "/W4",
        "/w44265",
        "/w44289",
        "/w44296",
        "/w45263",
        "/w44738",
        "/w45204"
    )
    add_defines("NOMINMAX", "UNICODE")
    add_defines("COMMITID=\"$(shell git rev-parse HEAD)\"")
    add_defines("VERSION=\"$(shell git describe --tags --abbrev=0 --always)\"")
    add_files("src/**.cpp")
    add_includedirs("src")
    add_packages(
        "levilamina",
        "bsci"
    )
    add_shflags("/DELAYLOAD:bedrock_server.dll") -- To use symbols provided by SymbolProvider.
    set_exceptions("none") -- To avoid conflicts with /EHa.
    set_kind("shared")
    set_languages("c++20")
    set_symbols("debug")

    after_build(function (target)
        local mod_packer = import("scripts.after_build")

        local tag = os.iorun("git describe --tags --abbrev=0 --always")
        local major, minor, patch, suffix = tag:match("v(%d+)%.(%d+)%.(%d+)(.*)")
        if not major then
            print("Failed to parse version tag, using 0.0.0")
            major, minor, patch = 0, 0, 0
        end
        local mod_define = {
            modName = target:name(),
            modFile = path.filename(target:targetfile()),
            modVersion = major .. "." .. minor .. "." .. patch,
        }
        
        mod_packer.pack_mod(target,mod_define)
    end)
