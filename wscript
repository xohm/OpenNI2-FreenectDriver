#!/usr/bin/env python
# encoding: utf-8

import platform


APPNAME = 'FreenectDriver'
VERSION = '1.0'

top = '.'
out = 'build'


def options(opt):
	opt.load('compiler_cxx')
	
def configure(conf):
	#conf.env.CXXFLAGS += ['-O2', '-fPIC', '-std=c++0x']
	conf.env.CXXFLAGS = ['-w', '-O2']
	conf.load('compiler_cxx')
	conf.check_cxx(lib='freenect', uselib_store='freenect')
	
	#if platform.system() == 'Darwin':
		#conf.env.CXX = ['clang++'] # remove once OSX gets >= gcc-4.6
		#conf.env.FRAMEWORKPATH += ['/usr/local/lib']

def build(bld):
	bld.shlib(
		target = APPNAME,
		name = APPNAME,
		vnum = VERSION,
		install_path = None,
		includes = ['extern/OpenNI-Linux-x64-2.2.0/Include', '/usr/include/libfreenect'],
		source = bld.path.ant_glob('src/*.cpp'),
		
		use = 'freenect',
	)
