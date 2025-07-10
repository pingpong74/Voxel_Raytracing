workspace "Voxy Engine"
    configurations { "Release", "Debug" }

project "Voxy"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"

    targetdir "bin/%{cfg.buildcfg}"
    objdir "bin/temp"

    files { "Src/**.hpp", "Src/**.cpp" }

    includedirs {
        "/usr/include",            -- system headers
        "/usr/include/glm",        -- glm
        "/usr/include/GLFW",       -- GLFW headers
        "/usr/include/vulkan"      -- Vulkan headers
    }

    libdirs {
        "/usr/lib"
    }

    links {
        "glfw",
        "vulkan"
    }

    filter { "configurations:Debug" }
        defines { "DEBUG" }
        symbols "On"

    filter { "configurations:Release" }
        defines { "RELEASE" }
        optimize "On"

    filter {} -- Reset filter after configs

    -----------------------
    -- Shader Compilation
    -----------------------

    local shaderDir = "Shaders/"
    local spvDir = "Shaders/spv/"

    local raytracingExtensions = { "rgen", "rchit", "rmiss", "rahit", "rint", "rcall" }

    local shaderStageMap = {
        rgen  = "raygen",
        rmiss = "miss",
        rchit = "closesthit",
        rahit = "anyhit",
        rint  = "intersection",
        rcall = "callable"
    }

    for _, ext in ipairs(raytracingExtensions) do
        local stage = shaderStageMap[ext]
        files { shaderDir .. "**." .. ext }

        filter { "files:" .. shaderDir .. "**." .. ext }
            buildmessage ("Compiling %{file.name} to SPIR-V")
            buildcommands {
                "mkdir -p " .. spvDir,
                "glslc --target-spv=spv1.5 %{file.relpath} -o " .. spvDir .. "%{file.basename}.spv"
            }
            buildoutputs {
                spvDir .. "%{file.basename}.spv"
            }
        filter {}
    end
