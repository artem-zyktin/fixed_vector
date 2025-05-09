root_dir = '../'

src_dir   = root_dir .. 'src/'
bin_dir   = root_dir .. 'bin/'
build_dir  = root_dir .. 'build/'


output_dir = bin_dir .. '%{cfg.architechture}/%{cfg.buildcfg}/output'
intermediate_dir = bin_dir .. '%{cfg.architechture}/%{cfg.buildcfg}/intermediate'

workspace('fv')
	configurations{'debug','release'}
	architecture('x64')
	location(build_dir .. '%{_ACTION}')

	targetdir(output_dir)
	objdir(intermediate_dir)

	startproject('test')

project('fv')
	kind('StaticLib')
	language('C++')
	cppdialect('C++20')

	files {
		src_dir .. 'fv/**.cpp',
		src_dir .. 'fv/**.hpp',
	}

	filter('configurations:debug')
		defines{'_DEBUG'}
		symbols('On')

	filter('configurations:release')
		defines('NDEBUG')
		symbols('On')
		optimize('On')

	filter{}

project('gtest')
	kind('StaticLib')
	language('C++')
	cppdialect('C++20')

	includedirs {
		src_dir .. 'googletest/',
		src_dir .. 'googletest/include/'
	}

	files {
		src_dir .. 'googletest/src/gtest-all.cc'
	}

	defines { 'GTEST_HAS_PTHREAD=0' }

	filter('configurations:debug')
		defines {'_DEBUG'}
		symbols('On')

	filter('configurations:release')
		defines{'NDEBUG'}
		symbols('On')
		optimize('On')

	filter{}

project('test')
	kind('ConsoleApp')
	language('C++')
	cppdialect('C++20')

	includedirs {
		src_dir,
		src_dir .. 'googletest/include/'
	}

	files {
		src_dir .. 'test/**.cpp'
	}

	links{'ecs_core','gtest'}
	libdirs{output_dir}

	filter('configurations:debug')
	defines {'_DEBUG'}
	symbols('On')

	filter('configurations:release')
		defines{'NDEBUG'}
		symbols('On')
		optimize('On')

	filter{}
