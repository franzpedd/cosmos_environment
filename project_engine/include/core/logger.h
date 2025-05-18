#pragma once

#include <string>
#include <vector>

// comment-me on release without bugs
#define COSMOS_LOGGER_ENABLED

namespace Cosmos
{
	/// @brief the supported severity level
	typedef enum
	{
		Trace = 0,
		Todo,
		Info,
		Warn,
		Error,
		Assert
	} LogSeverity;

	/// @brief stringfies log severity
	/// @param severity log's severity level
	/// @return the stringfied severity level
	const char* LogSeverity_cstr(LogSeverity severity);

	/// @brief logs data into the terminal (only if COSMOS_LOGGER_ENABLED is defined, does nothing otherwise)
	/// @param severity log's severity level
	/// @param file from witch file the error has origined from
	/// @param line witch line the error is
	/// @param message custom message about the error
	/// @param any variadict arguments, much like printf's
	void LogToTerminal(LogSeverity severity, const char* file, int line, const char* message, ...);

	/// @brief logs data into a file (only if COSMOS_LOGGER_ENABLED is defined, does nothing otherwise)
	/// @param severity log's severity level
	/// @param path the path on disk to write the error info to
	/// @param file from witch file the error has origined from
	/// @param line witch line the error is
	/// @param message custom message about the error
	/// @param any variadict arguments, much like printf's
	void LogToFile(LogSeverity severity, const char* path, const char* file, int line, const char* message, ...);

	class LoggerTracer
	{
	public:
		struct Message
		{
			LogSeverity severity;
			std::string message;
		};

	public:

		/// @brief returns the instance
		static LoggerTracer& GetInstance();

		/// @brief returns all the messages on the console tracer
		inline std::vector<Message>& GetMessagesRef() { return mConsoleMessages; }

		/// @brief cleans-up all message logs in the console tracer
		inline void Cleanup() { mConsoleMessages.clear(); }

	private:

		std::vector<Message> mConsoleMessages;
	};
}

#define COSMOS_LOG(severity, ...) Cosmos::LogToTerminal(severity, __FILE__, __LINE__, __VA_ARGS__);
#define COSMOS_LOG_FILE(severity, file, ...) Cosmos::LogToFile(severity, file, __FILE__, __LINE__, __VA_ARGS__);
#define COSMOS_ASSERT(condition, ...) do { if (!(condition)) { Cosmos::LogToTerminal(Cosmos::LogSeverity::Assert, __FILE__, __LINE__, __VA_ARGS__); } } while (0);

/// possible user-struggle here, if COSMOS_LOGGER_ENABLED is not defined and the user's code has an assert, the COSMOS_ASSERT will freeze the application without telling any
/// error message, so be aware when to enable/disable logger. This simple logger was made this way so it's easy to replace it with a more sophisticated one