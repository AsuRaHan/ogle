// src/log/Logger.cpp
#include "Logger.h"
#include <windows.h>
#include <iostream>

namespace ogle {

	namespace {

		// Цвета консоли (Windows)
		WORD GetColor(LogLevel level) {
			switch (level) {
			case LogLevel::Debug:   return FOREGROUND_BLUE | FOREGROUND_INTENSITY;         // cyan
			case LogLevel::Info:    return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; // белый
			case LogLevel::Warning: return FOREGROUND_RED | FOREGROUND_GREEN;              // жёлтый
			case LogLevel::Error:   return FOREGROUND_RED | FOREGROUND_INTENSITY;          // ярко-красный
			case LogLevel::Success: return FOREGROUND_GREEN | FOREGROUND_INTENSITY;        // ярко-зелёный
			default:                return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
			}
		}

		void SetConsoleColor(WORD color) {
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, color);
		}

		void ResetConsoleColor() {
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}

		std::string LevelToString(LogLevel level) {
			switch (level) {
			case LogLevel::Debug:   return "[DEBUG]";
			case LogLevel::Info:    return "[INFO] ";
			case LogLevel::Warning: return "[WARN] ";
			case LogLevel::Error:   return "[ERROR]";
			case LogLevel::Success: return "[OK]   ";
			default:                return "[UNKNOWN]";
			}
		}

	} // anonymous namespace

	void Logger::Log(LogLevel level, const std::string& message) {
		SetConsoleColor(GetColor(level));
		std::cout << LevelToString(level) << " " << message << std::endl;
		ResetConsoleColor();
	}

	void Logger::Debug(const std::string& message) { Log(LogLevel::Debug, message); }
	void Logger::Info(const std::string& message) { Log(LogLevel::Info, message); }
	void Logger::Warning(const std::string& message) { Log(LogLevel::Warning, message); }
	void Logger::Error(const std::string& message) { Log(LogLevel::Error, message); }
	void Logger::Success(const std::string& message) { Log(LogLevel::Success, message); }

} // namespace ogle