#ifndef CMDPARSER_H_
#define CMDPARSER_H_

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cctype>
#include <algorithm>
#include <memory>


inline bool is_file(const std::string& name) {
	std::ifstream f(name.c_str());
	return f.good();
}

inline bool is_number(const std::string& s) {
	return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

inline bool starts_with(const std::string& src, const std::string& sub) {
	return src.find(sub) == 0;
}

class CmdParser {
public:

	class CmdSwitch {
	public:
		CmdSwitch() = default;
		CmdSwitch(const std::string& key, const std::string& shortcut, const std::string& desc, const std::string& default_value, bool istoggle)
			: key(key), shortcut(shortcut), desc(desc), default_value(default_value), value(default_value), istoggle(istoggle), isvalid(false) {
			if (istoggle) {
				this->default_value = "false";
				this->value = this->default_value;
				this->isvalid = true;
			}
		}

		CmdSwitch(const CmdSwitch& other) = default;
		CmdSwitch& operator=(const CmdSwitch& other) = default;

		std::string key;
		std::string shortcut;
		std::string default_value;
		std::string value;
		std::string desc;
		bool istoggle = false;
		bool isvalid = false;
	};

	CmdParser() : m_appname("application.exe") {
		addSwitch("--help", "-h", "prints this help list", "", true);
	}

	~CmdParser() = default;

	bool addSwitch(const CmdSwitch& s) {
		if (s.desc.empty()) {
			std::cerr << "Error: No description provided for switch!\n";
			return false;
		}

		if (!isValidKeyFormat(s.key)) {
			std::cerr << "Error: Invalid key format. Keys must start with '--' and have a length of at least 3.\n";
			return false;
		}

		if (m_mapKeySwitch.find(s.key) != m_mapKeySwitch.end()) {
			std::cerr << "Error: Key '" << s.key << "' is already taken!\n";
			return false;
		}

		CmdSwitch cmd = s;
		if (cmd.shortcut.empty()) {
			cmd.shortcut = generateShortcut(cmd.key);
			std::cout << "Automatic shortcut assigned: '" << cmd.shortcut << "' for '" << cmd.key << "'\n";
		}

		auto pcmd = std::make_unique<CmdSwitch>(cmd);
		m_mapShortcutKeys[cmd.shortcut] = cmd.key;
		m_mapKeySwitch[cmd.key] = std::move(pcmd);

		return true;
	}

	bool addSwitch(const std::string& name, const std::string& shortcut,
		const std::string& desc, const std::string& default_value = "",
		bool istoggle = false) {
		return addSwitch(CmdSwitch(name, shortcut, desc, default_value, istoggle));
	}

	bool setDefaultKey(const std::string& key) {
		std::string strKey = key;
		if (!starts_with(strKey, "--")) {
			strKey = "--" + strKey;
		}

		if (m_mapKeySwitch.find(strKey) != m_mapKeySwitch.end()) {
			CmdSwitch* pcmd = m_mapKeySwitch[m_strDefaultKey].get();
			if (pcmd && pcmd->istoggle) {
				std::cerr << "Boolean command line options cannot be used as default keys\n";
				return false;
			}

			m_strDefaultKey = strKey;
			return true;
		}
		return false;
	}

	int parse(int argc, char* argv[]) {
		int i = 0;
		int ctOptions = 0;
		while (i < argc) {
			std::string token = argv[i];
			std::string key;

			bool iskey = false;
			bool isNextTokenKey = false;

			if (i + 1 < argc) {
				std::string peeknext = argv[i + 1];
				isNextTokenKey = isTokenKey(peeknext);
			}

			if (isTokenKey(token)) {
				key = getTokenKey(token);
				if (key.empty()) {
					printHelp();
					return -1;
				}
				iskey = true;
			}
			else if (!isNextTokenKey && !m_strDefaultKey.empty() && i == argc - 2) {
				key = m_strDefaultKey;
				iskey = true;
			}

			if (iskey) {
				++ctOptions;

				if (key == "--help") {
					printHelp();
					return 1;
				}

				CmdSwitch* pcmd = m_mapKeySwitch[key].get();
				if (pcmd->istoggle) {
					pcmd->value = "true";
					pcmd->isvalid = true;
				}
				else {
					++i;
					pcmd->value = argv[i];
					pcmd->isvalid = true;
				}
			}

			++i;
		}

		if (argc > 0) {
			m_appname = argv[0];
		}

		return ctOptions;
	}

	std::string value(const std::string& key) const {
		std::string strKey = key;
		if (!starts_with(strKey, "--")) {
			strKey = "--" + strKey;
		}

		auto it = m_mapKeySwitch.find(strKey);
		if (it != m_mapKeySwitch.end()) {
			return it->second->value;
		}

		std::cerr << "The input key " << strKey << " is not recognized!\n";
		return "";
	}

	bool value_to_bool(const std::string& key) const {
		return value(key) == "true";
	}

	int value_to_int(const std::string& key) const {
		std::string strVal = value(key);
		return (strVal.empty() || !is_number(strVal)) ? -1 : std::stoi(strVal);
	}

	double value_to_double(const std::string& key) const {
		std::string strVal = value(key);
		return strVal.empty() ? -1 : std::stod(strVal);
	}

	bool isValid(const std::string& key) const {
		std::string strKey = key;
		if (!starts_with(strKey, "--")) {
			strKey = "--" + strKey;
		}

		auto it = m_mapKeySwitch.find(strKey);
		if (it != m_mapKeySwitch.end()) {
			return it->second->isvalid;
		}

		std::cerr << "The input key " << strKey << " is not recognized!\n";
		return false;
	}

	void printHelp() const {
		std::cout << "===========================================================\n";
		std::string strAllShortcuts;
		for (const auto& switchPtr : m_mapKeySwitch) {
			if (!switchPtr.second->shortcut.empty()) {
				strAllShortcuts += switchPtr.second->shortcut;
			}
		}

		std::cout << "Usage: " << m_appname << " -[" << strAllShortcuts << "]\n\n";

		for (const auto& switchPtr : m_mapKeySwitch) {
			if (!switchPtr.second->default_value.empty()) {
				std::cout << "\t" << switchPtr.second->key << ", " << switchPtr.second->shortcut
					<< "\t\t" << switchPtr.second->desc << "\t Default: ["
					<< switchPtr.second->default_value << "]\n";
			}
			else {
				std::cout << "\t" << switchPtr.second->key << ", " << switchPtr.second->shortcut
					<< "\t\t" << switchPtr.second->desc << '\n';
			}
		}
	}

private:
	bool isValidKeyFormat(const std::string& key) const {
		return starts_with(key, "--") && key.length() >= 3;
	}

	std::string generateShortcut(const std::string& key) const {
		std::string temp = "-" + key[2];
		int i = 3;
		while (m_mapShortcutKeys.find(temp) != m_mapShortcutKeys.end() && i < key.size()) {
			temp = "-" + key[i];
			++i;
		}
		return temp;
	}

	bool isTokenKey(const std::string& token) const {
		return starts_with(token, "-") || starts_with(token, "--");
	}

	std::string getTokenKey(const std::string& token) const {
		if (starts_with(token, "--")) {
			if (m_mapKeySwitch.find(token) != m_mapKeySwitch.end()) {
				return token;
			}
		}
		else if (starts_with(token, "-")) {
			auto it = m_mapShortcutKeys.find(token);
			if (it != m_mapShortcutKeys.end()) {
				return it->second;
			}
		}

		std::cerr << "Unrecognized key passed " << token << "\n";
		return "";
	}

	std::map<std::string, std::unique_ptr<CmdSwitch>> m_mapKeySwitch;
	std::map<std::string, std::string> m_mapShortcutKeys;
	std::string m_strDefaultKey;
	std::string m_appname;
};

#endif
