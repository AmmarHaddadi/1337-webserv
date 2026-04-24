#pragma once

#include <iostream>
#include <string>

// HINT read readme #Logging

class ActiveLogger {
	std::string topic;

  public:
	enum Level { DEBUG, INFO, WARN, ERROR };
	Level minLevel;

	ActiveLogger(std::string t, Level l) : topic(t), minLevel(l) {}

	void debug(const std::string &m) const {
		if (minLevel <= DEBUG)
			std::cout << "[DEBUG][" << topic << "] " << m << "\n";
	}
	void info(const std::string &m) const {
		if (minLevel <= INFO)
			std::cout << "\033[34m[INFO][" << topic << "]\033[0m " << m << "\n";
	}
	void warn(const std::string &m) const {
		if (minLevel <= WARN)
			std::cout << "\033[33m[WARN][" << topic << "]\033[0m " << m << std::endl;
	}
	void error(const std::string &m) const {
		std::cerr << "\033[31m[ERROR][" << topic << "]\033[0m " << m << std::endl;
	}
};

// for optimiziations (this is a server after all)
class QuietLogger {
	std::string m_topic;

  public:
	enum Level { DEBUG, INFO, WARN, ERROR };
	QuietLogger(std::string t, Level) : m_topic(t) {}
	void debug(const std::string &) const {}
	void info(const std::string &) const {}
	void warn(const std::string &) const {}
	void error(const std::string &m) const {
		// Errors stay enabled all time
		std::cerr << "\033[31m[ERROR][" << m_topic << "]\033[0m " << m << std::endl;
	}
};

// Declare loggers here

#ifdef LOG_CORE
typedef ActiveLogger CoreLogger;
#else
typedef QuietLogger CoreLogger;
#endif
