#ifndef ERRCODE_HXX
#define ERRCODE_HXX

namespace error {
	enum error {
		UNPRIVILEGED = -1, // Permission denied
		ALLOCATION = -2, // Couldn't allocate enough memory
		INVALID_SETUP = -3, // The provided setup isn't valid for the operation
		RESOURCE_UNAVAILABLE = -4, // A resource needed to continue operation isn't obtainable
		RESOURCE_EXPECTED = -5, // A resource was expected for the operation, but wasn't the one desired
		RESOURCE_BUSY = -6, // Resource is being used by someone else
		INVALID_PARAM = -7, // The data given is invalid
		UNIMPLEMENTED = -8, // Functionality is not implemented/unavailable
		EXPLICIT_FAILURE = -9, // A failure is registered by the system and it's explicit
		TIMEOUT = -10, // A timeout for waiting a resource has occured
	};
}

#endif
