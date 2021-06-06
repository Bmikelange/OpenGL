
no_project = true
dofile "./premake4.lua"

gfx_projet_dir = path.getabsolute(".")

master_projet_files = {	gfx_projet_dir .. "/src/Tp2/*.cpp"
	}
	
project("Tp2")
    language "C++"
    kind "ConsoleApp"
    targetdir ( gfx_projet_dir .. "/bin" )
    includedirs { gfx_projet_dir .. "/src/Tp2"}
    files ( gkit_files )
    files ( master_projet_files )
    links { "GLEW", "SDL2", "SDL2_image", "GL","GLU","pthread" }
    buildoptions { "-fopenmp"}
    linkoptions {"-fopenmp"}
    defines {"WITH_SDL2_STATIC"}
