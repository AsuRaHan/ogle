// src/log/Logger.h
#pragma once

#include <string>

namespace ogle {

	enum class LogLevel {
		Debug,
		Info,
		Warning,
		Error,
		Success
	};

	class Logger {
	public:
		static void Debug(const std::string& message);
		static void Info(const std::string& message);
		static void Warning(const std::string& message);
		static void Error(const std::string& message);
		static void Success(const std::string& message);

		// Универсальный метод
		static void Log(LogLevel level, const std::string& message);
	};

} // namespace ogle