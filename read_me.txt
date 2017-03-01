notes on crossworks setup file
     c_system_include_directories="$(StudioDir)/include;$(PackagesDir)/include;$(ARC_prefix)/include"
		lists include directeries ie finds --> <somefile.h>. the first two entries in the list are probably defalt
	  
	macros="ARC_prefix=$(ProjectDir)/EE444_test_lib/"
		this line sets up where crossworks looks for the desired library lib files. ./ denotes a relative path or using $(ProjectDir)
		
pulling from git should not cause any paths to be redefined as everything is set by the relative project dir path