#pragma once
#ifndef DEBUG_PATH
#ifdef PUPPET_PUBLISH
	#define DEBUG_PATH "debug\\"
#else
	#define DEBUG_PATH "C:\\Users\\Sierra\\source\\repos\\Puppet2\\Puppet2\\assets\\"
#endif
#endif