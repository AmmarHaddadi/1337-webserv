#pragma once

#include <iostream>
#include <string>

// HINT read readme #Logging

class ActiveLogger {
	std::string m_topic;

  public:
	enum Level { DEBUG, INFO, WARN, ERROR };
	Level m_minLevel;

	ActiveLogger(const std::string &topic, Level level) : m_topic(topic), m_minLevel(level) {}

	void debug(const std::string &msg) const {
		if (m_minLevel <= DEBUG)
			std::cout << "[DEBUG][" << m_topic << "] " << msg << "\n";
	}
	void info(const std::string &msg) const {
		if (m_minLevel <= INFO)
			std::cout << "\033[34m[INFO][" << m_topic << "]\033[0m " << msg << "\n";
	}
	void warn(const std::string &msg) const {
		if (m_minLevel <= WARN)
			std::cout << "\033[33m[WARN][" << m_topic << "]\033[0m " << msg << '\n';
	}
	void error(const std::string &msg) const {
		std::cerr << "\033[31m[ERROR][" << m_topic << "]\033[0m " << msg << '\n';
	}
};

// for optimiziations (this is a server after all)
class QuietLogger {
	std::string m_topic;

  public:
	enum Level { DEBUG, INFO, WARN, ERROR };
	QuietLogger(const std::string &topic, Level /*lvl*/) : m_topic(topic) {}
	void debug(const std::string & /*msg*/) const {}
	void info(const std::string & /*msg*/) const {}
	void warn(const std::string & /*msg*/) const {}
	void error(const std::string &msg) const {
		// Errors stay enabled all time
		std::cerr << "\033[31m[ERROR][" << m_topic << "]\033[0m " << msg << '\n';
	}
};

// Declare loggers here

#ifdef LOG_CORE
typedef ActiveLogger CoreLogger;
#else
typedef QuietLogger CoreLogger;
#endif
