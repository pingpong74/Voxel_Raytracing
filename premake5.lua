workspace "Voxy Engine"
    configurations { "Release" , "Debug" }

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
        defines {"DEBUG"}
        symbols "On"

    filter { "configurations:Release" }
        defines { "RELEASE" }
        optimize "On"
