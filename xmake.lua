add_rules("mode.debug", "mode.release")

add_repositories("LeviMC-repo https://github.com/LiteLDev/xmake-repo.git")
add_repositories("oeotyan-repo https://github.com/OEOTYAN/xmake-repo.git")

add_requires("levilamina 26.32.2", {configs = {target_type = get_config("target_type")}})

add_requires("levibuildscript")

add_requires("bsci v26.32.2", {configs = {target_type = get_config("target_type")}})

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

option("target_type")
    set_default("server")
    set_showmenu(true)
    set_values("server", "client")
option_end()

target("CoralFans")
    add_rules("@levibuildscript/linkrule")
    add_rules("@levibuildscript/modpacker")
    if is_plat("windows") then
        add_defines("NOMINMAX", "UNICODE", "_AMD64_")
        set_exceptions("none") -- To avoid conflicts with /EHa.
        add_cxflags( "/EHa", "/utf-8", "/W4", "/w44265", "/w44289", "/w44296", "/w45263", "/w44738", "/w45204")
        add_cxflags(
            "/EHs",
            "-Wno-microsoft-cast",
            "-Wno-invalid-offsetof",
            "-Wno-c++2b-extensions",
            "-Wno-microsoft-include",
            "-Wno-overloaded-virtual",
            "-Wno-ignored-qualifiers",
            "-Wno-missing-field-initializers",
            "-Wno-potentially-evaluated-expression",
            "-Wno-pragma-system-header-outside-header",
            {tools = {"clang_cl"}}
        )
        set_toolchains("clang-cl")
    end
    add_defines("COMMITID=\"$(shell git rev-parse HEAD)\"")
    add_defines("CF_VERSION=\"$(shell git describe --tags --abbrev=0 --always)\"")
    add_packages("levilamina", "bsci")
    set_kind("shared")
    set_languages("c++20")
    set_symbols("debug")
    add_headerfiles("src/**.h")
    add_files("src/**.cpp")
    add_includedirs("src")
    if is_config("target_type", "server") then
    --  add_includedirs("src-server")
    --  add_files("src-server/**.cpp")
    else
    --  add_includedirs("src-client")
    --  add_files("src-client/**.cpp")
    end

    after_build(function (target)
        local bindir = path.join(os.projectdir(), "bin")
        local outputdir = path.join(bindir, target:name())
        -- Copy i18n files.
        local orilangfile = path.join(os.projectdir(), "src", "lang")
        local langfile = path.join(outputdir, "lang")
        os.rm(langfile)
        os.cp(orilangfile, langfile)
    end)
