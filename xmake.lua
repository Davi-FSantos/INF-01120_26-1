-- Set minimum xmake version
set_xmakever("2.8.2")

-- Define project and strict C++ standard
set_project("Music_Machine")
set_version("1.0.0")
set_languages("cxx26")

-- Make xmake output the file for clangd
add_rules("plugin.compile_commands.autoupdate", {outputdir = "."})

-- Enforce warnings as errors globally
set_warnings("all", "error")

-- Define the main executable target
target("Music_Machine")
    set_kind("binary")
    add_rules("qt.widgetapp")

    add_files("src/*.cpp")
    add_files("src/audio/*.cpp")
    add_files("src/core/*.cpp")
    add_files("src/parsing/*.cpp")
    add_files("include/mainwindow.h")
    add_files("src/qtmidi.ui")

    add_includedirs("include", "src", ".")

    -- Link the fluidsynth package
    add_packages("fluidsynth")

    -- Separate binaries dynamically inside the bin directory
    set_targetdir("bin/$(os)_$(arch)_$(mode)")

    -- Release Mode Configuration: High Performance
    if is_mode("release") then
        set_optimize("fastest") -- Automatically maps to -O3 or /O2
        
        -- Compiler-specific flags for fast-math and Link Time Optimization (LTO)
        add_cxflags("-ffast-math", "-flto", {tools = {"clang", "gcc"}})
        add_ldflags("-flto", {tools = {"clang", "gcc"}})
        
        add_cxflags("/fp:fast", "/GL", {tools = {"cl"}})
        add_ldflags("/LTCG", {tools = {"cl"}})
    end

    -- Debug Mode Configuration: Memory Inspection
    if is_mode("debug") then
        set_symbols("debug")
        set_optimize("none")
        
        -- Address and Undefined Behavior Sanitizers
        if is_plat("linux", "macosx", "freebsd") then
            add_cxflags("-fsanitize=address,undefined", "-fno-omit-frame-pointer")
            add_ldflags("-fsanitize=address,undefined")
        elseif is_plat("windows") then
            add_cxflags("/fsanitize=address")
        end
    end

-- Custom Task: Build all platforms and architectures in Release mode
task("build_all_release")
    set_menu({
        usage = "xmake build_all_release",
        description = "Compiles the project for Windows, Linux, MacOS, and FreeBSD across x86, ARM, and RISCV."
    })
    on_run(function ()
        import("core.base.task")

        -- Define the matrix of required platforms and architectures
        local targets = {
            {os = "linux", arch = "x86_64"},
            {os = "linux", arch = "arm64"},
            {os = "linux", arch = "riscv64"},
            {os = "windows", arch = "x64"},
            {os = "windows", arch = "arm64"},
            {os = "macosx", arch = "x86_64"},
            {os = "macosx", arch = "arm64"},
            {os = "freebsd", arch = "x86_64"}
        }

        for _, tgt in ipairs(targets) do
            print(string.format("Building: %s %s (Release)", tgt.os, tgt.arch))
            -- Configure the environment for the specific target
            os.execv("xmake", {"f", "-p", tgt.os, "-a", tgt.arch, "-m", "release", "-c"})
            -- Execute the build
            os.execv("xmake", {"build"})
        end
    end
    )
