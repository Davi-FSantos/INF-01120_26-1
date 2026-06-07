-- Set minimum xmake version
set_xmakever("2.8.2")

-- Define project and strict C++ standard
set_project("Music_Machine")
set_version("1.0.0")
set_languages("cxx26")

add_requires("fluidsynth")
add_requires("libremidi")

if is_mode("debug") then
    add_requires("doctest")
end

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
    add_files("include/musicMachine.h")
    add_files("src/audio/MidiPlayer.h")
    add_files("src/audio/MidiWriter.h")
    add_files("src/aboutdialog.h")
    add_files("src/musicMachine.ui")
    add_files("src/aboutdialog.ui")

    -- Compile translation files after building
    after_build(function (target)
        import("detect.sdks.find_qt")
        local qt = find_qt()
        local lrelease
        if qt and qt.bindir then
            local ext = is_plat("windows") and ".exe" or ""
            local names = {"lrelease6" .. ext, "lrelease" .. ext, "lrelease5" .. ext}
            for _, name in ipairs(names) do
                local p = path.join(qt.bindir, name)
                if os.isexec(p) then
                    lrelease = p
                    break
                end
            end
        end

        if not lrelease then
            import("lib.detect.find_program")
            lrelease = find_program("lrelease6") or find_program("lrelease") or find_program("lrelease5")
        end

        if lrelease then
            local ts = path.join(target:scriptdir(), "src/musicMachine_pt_BR.ts")
            local qm_dir = path.join(target:targetdir(), "translations")
            if not os.isdir(qm_dir) then
                os.mkdir(qm_dir)
            end
            local qm = path.join(qm_dir, "musicMachine_pt_BR.qm")
            os.run("%s %s -qm %s", lrelease, ts, qm)
            print("Compiled translation: %s -> %s", ts, qm)
        else
            raise("lrelease tool not found! Please ensure Qt Linguist Tools are installed.")
        end
    end)

    add_includedirs("include", "src", ".")

    -- Link the fluidsynth package
    add_packages("fluidsynth")
    add_packages("libremidi")

    if is_plat("windows") then
        add_defines("_CRT_SECURE_NO_WARNINGS")
    end

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

-- Debug-only test runner target and dependencies
if is_mode("debug") then
    target("test_runner")
        set_kind("binary")
        set_default(false) -- Don't compile by default with 'xmake'
        add_files("tests/test_backend.cpp")
        add_files("src/core/Voice.cpp")
        add_files("src/parsing/TextParser.cpp")
        add_files("src/contracts_handler.cpp")
        add_includedirs("include", "src", ".")
        add_packages("doctest")

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

-- Custom Task: Format all source code
task("format")
    set_menu({
        usage = "xmake format",
        description = "Runs clang-format over all source and header C++ files."
    })
    on_run(function ()
        os.exec("clang-format -i src/*.cpp src/audio/*.cpp src/core/*.cpp src/parsing/*.cpp include/*.h src/*.h tests/*.cpp")
        print("Codebase formatted successfully!")
    end)

-- Custom Task: Lint all source code
task("lint")
    set_menu({
        usage = "xmake lint",
        description = "Runs static analysis using clang-tidy and run_linter.sh."
    })
    on_run(function ()
        os.exec("./run_linter.sh")
    end)

