#pragma once

constexpr static char LogLevelDoc[] =
R"(Enumeration of logging levels.
)";

constexpr static char init_loggingDoc[] =
R"(Initialize the logging system.

Args:
	level (LogLevel): The logging level to use.
	console (bool): Whether to log to the console.
	file (bool): Whether to log to a file.

Returns:
	None
)";

constexpr static char set_log_levelDoc[] =
R"(Set the logging level.

Args:
	level (LogLevel): The logging level to use.

Returns:
	None
)";

constexpr static char get_dlpack_capsule_infoDoc[] =
R"(Get information about a DLPack capsule.

Args:
	array (ndarray): The array to get information about.

Returns:
	dict: A dictionary containing information about the array.
)";
