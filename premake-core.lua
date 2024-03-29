require "Dependencies/premake"
require "Dependencies/premake-fbx"

require "premake"

project( prefix .. "Platform" )

	Helium.DoModuleProjectSettings( "Source/Core", "HELIUM", "Platform", "PLATFORM" )

	files
	{
		"Source/Core/Platform/*.cpp",
		"Source/Core/Platform/*.h",
		"Source/Core/Platform/*.inl",
	}

	excludes
	{
		"Source/Core/Platform/*Tests.*",
	}

	filter "system:windows"
		excludes
		{
			"Source/Core/Platform/*Posix.*",
			"Source/Core/Platform/*Mac.*",
			"Source/Core/Platform/*Lin.*",
		}

	filter "system:macosx"
		excludes
		{
			"Source/Core/Platform/*Win.*",
			"Source/Core/Platform/*Lin.*",
		}

	filter "system:linux"
		excludes
		{
			"Source/Core/Platform/*Win.*",
			"Source/Core/Platform/*Mac.*",
		}

	filter { "system:linux", "kind:SharedLib" }
		links
		{
			"pthread",
			"dl",
		}

	filter {}

project( prefix .. "PlatformTests" )

	Helium.DoTestsProjectSettings()

	files
	{
		"Source/Core/Platform/*Tests.*",
	}

	links
	{
		prefix .. "Platform"
	}

project( prefix .. "Foundation" )

	Helium.DoModuleProjectSettings( "Source/Core", "HELIUM", "Foundation", "FOUNDATION" )

	files
	{
		"Source/Core/Foundation/**",
	}

	excludes
	{
		"Source/Core/Foundation/*Tests.*",
	}

	filter "kind:SharedLib"
		links
		{
			prefix .. "Platform",
		}

	filter {}

project( prefix .. "FoundationTests" )

	Helium.DoTestsProjectSettings()

	files
	{
		"Source/Core/Foundation/*Tests.*",
	}

	links
	{
		prefix .. "Foundation",
		prefix .. "Platform",
	}

project( prefix .. "Application" )

	Helium.DoModuleProjectSettings( "Source/Core", "HELIUM", "Application", "APPLICATION" )

	files
	{
		"Source/Core/Application/**",
	}

	excludes
	{
		"Source/Core/Application/*Tests.*",
	}

	filter "kind:SharedLib"
		links
		{
			prefix .. "Foundation",
			prefix .. "Platform",
		}

	filter {}

project( prefix .. "ApplicationTests" )

	Helium.DoTestsProjectSettings()

	files
	{
		"Source/Core/Application/*Tests.*",
	}

	links
	{
		prefix .. "Application",
		prefix .. "Foundation",
		prefix .. "Platform",
	}

project( prefix .. "Reflect" )

	Helium.DoModuleProjectSettings( "Source/Core", "HELIUM", "Reflect", "REFLECT" )

	files
	{
		"Source/Core/Reflect/**",
	}

	excludes
	{
		"Source/Core/Reflect/*Tests.*",
	}

	filter "kind:SharedLib"
		links
		{
			prefix .. "Foundation",
			prefix .. "Platform",
		}

	filter {}

project( prefix .. "ReflectTests" )

	Helium.DoTestsProjectSettings()

	files
	{
		"Source/Core/Reflect/*Tests.*",
	}

	links
	{
		prefix .. "Reflect",
		prefix .. "Foundation",
		prefix .. "Platform"
	}

project( prefix .. "Persist" )

	Helium.DoModuleProjectSettings( "Source/Core", "HELIUM", "Persist", "PERSIST" )

	files
	{
		"Source/Core/Persist/**",
	}

	excludes
	{
		"Source/Core/Persist/*Tests.*",
	}

	filter "kind:SharedLib"
		links
		{
			prefix .. "Platform",
			prefix .. "Foundation",
			prefix .. "Reflect",
			"mongo-c",
		}

	filter {}

project( prefix .. "PersistTests" )

	Helium.DoTestsProjectSettings()

	files
	{
		"Source/Core/Persist/*Tests.*",
	}

	links
	{
		prefix .. "Persist",
		prefix .. "Reflect",
		prefix .. "Foundation",
		prefix .. "Platform"
	}

project( prefix .. "Mongo" )

	Helium.DoModuleProjectSettings( "Source/Core", "HELIUM", "Mongo", "MONGO" )

	files
	{
		"Source/Core/Mongo/**",
	}

	excludes
	{
		"Source/Core/Mongo/*Tests.*",
	}

	filter "kind:SharedLib"
		links
		{
			prefix .. "Platform",
			prefix .. "Foundation",
			prefix .. "Reflect",
			prefix .. "Persist",
			"mongo-c",
		}

	filter {}

project( prefix .. "MongoTests" )

	Helium.DoTestsProjectSettings()

	files
	{
		"Source/Core/Mongo/*Tests.*",
	}

	links
	{
		prefix .. "Mongo",
		prefix .. "Persist",
		prefix .. "Reflect",
		prefix .. "Foundation",
		prefix .. "Platform",
		"mongo-c",
	}

project( prefix .. "Inspect" )

	Helium.DoModuleProjectSettings( "Source/Core", "HELIUM", "Inspect", "INSPECT" )

	files
	{
		"Source/Core/Inspect/**",
	}

	excludes
	{
		"Source/Core/Inspect/*Tests.*",
	}

	filter "kind:SharedLib"
		links
		{
			prefix .. "Platform",
			prefix .. "Foundation",
			prefix .. "Application",
			prefix .. "Reflect",
			prefix .. "Persist",
			prefix .. "Math",
		}

	filter {}

project( prefix .. "InspectTests" )

	Helium.DoTestsProjectSettings()

	files
	{
		"Source/Core/Inspect/*Tests.*",
	}

	links
	{
		prefix .. "Inspect",
		prefix .. "Math",
		prefix .. "Persist",
		prefix .. "Reflect",
		prefix .. "Application",
		prefix .. "Foundation",
		prefix .. "Platform"
	}

project( prefix .. "Math" )

	Helium.DoModuleProjectSettings( "Source/Core", "HELIUM", "Math", "MATH" )

	files
	{
		"Source/Core/Math/**",
	}

	excludes
	{
		"Source/Core/Math/*Tests.*",
	}

	filter "kind:SharedLib"
		links
		{
			prefix .. "Reflect",
			prefix .. "Foundation",
			prefix .. "Platform",
			prefix .. "Persist",
		}

	filter {}

project( prefix .. "MathTests" )

	Helium.DoTestsProjectSettings()

	files
	{
		"Source/Core/Math/*Tests.*",
	}

	links
	{
		prefix .. "Math",
		prefix .. "Reflect",
		prefix .. "Foundation",
		prefix .. "Platform"
	}
