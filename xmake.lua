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
    "levilamina 197a033a9de6be25469b9e932e80369255f56930",
    "levibuildscript",
    "bsci main"
)

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

option("CoralFans")
    set_default("server")
    set_showmenu(true)
    set_values("server", "client")
option_end()

target("CoralFans") -- Change this to your mod name.
    add_rules("@levibuildscript/linkrule")
    add_rules("@levibuildscript/modpacker")
    add_cxflags( "/EHa", "/utf-8", "/W4", "/w44265", "/w44289", "/w44296", "/w45263", "/w44738", "/w45204")
    add_defines("NOMINMAX", "UNICODE")
    add_packages(
        "levilamina",
        "levibuildscript",
        "bsci"
    )
    set_exceptions("none") -- To avoid conflicts with /EHa.
    set_kind("shared")
    set_languages("c++20")
    set_symbols("debug")
    set_toolchains("msvc")
    add_files("src/**.cpp")
    add_includedirs("src")
    add_installfiles("src/lang/**.*", {destination = "bin/CoralFans/lang"})
    -- if is_config("target_type", "server") then
    --     add_includedirs("src-server")
    --     add_files("src-server/**.cpp")
    -- else
    --     add_includedirs("src-client")
    --     add_files("src-client/**.cpp")
    -- end
