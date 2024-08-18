#ifndef CMDPARSER_H_
#define CMDPARSER_H_

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

namespace sda {
    namespace utils {

        class CmdParser {
        public:
            class CmdSwitch {
            public:
                CmdSwitch(const string& key = "", const string& desc = "",
                    const string& default_value = "", bool istoggle = false)
                    : key(key), desc(desc), default_value(default_value),
                    value(default_value), istoggle(istoggle), isvalid(false) {}

                string key;
                string desc;
                string default_value;
                string value;
                bool istoggle;
                bool isvalid;
            };

        public:
            CmdParser(const string& appname = "application.exe") : m_appname(appname) {
                addSwitch("--help", "prints this help list", "", true);
            }

            ~CmdParser() {
                for (auto& entry : m_mapKeySwitch) {
                    delete entry.second;
                }
            }

            bool addSwitch(const string& key, const string& desc,
                const string& default_value = "", bool istoggle = false) {
                if (key.empty() || !starts_with(key, "--")) {
                    cerr << "Error: Invalid key format. Keys must start with '--'.\n";
                    return false;
                }

                if (m_mapKeySwitch.find(key) != m_mapKeySwitch.end()) {
                    cerr << "Error: Key '" << key << "' is already taken!\n";
                    return false;
                }

                CmdSwitch* cmd = new CmdSwitch(key, desc, default_value, istoggle);
                m_mapKeySwitch[key] = cmd;
                return true;
            }

            int parse(int argc, char* argv[]) {
                int ctOptions = 0;
                for (int i = 1; i < argc; ++i) {
                    string token = argv[i];

                    if (starts_with(token, "--")) {
                        auto it = m_mapKeySwitch.find(token);
                        if (it == m_mapKeySwitch.end()) {
                            cerr << "Unrecognized key passed " << token << std::endl;
                            printHelp();
                            return -1;
                        }

                        CmdSwitch* cmd = it->second;
                        cmd->isvalid = true;
                        if (cmd->istoggle) {
                            cmd->value = "true";
                        }
                        else if (i + 1 < argc) {
                            cmd->value = argv[++i];
                        }
                        else {
                            cmd->value = cmd->default_value;
                        }
                        ++ctOptions;
                    }
                    else if (starts_with(token, "-")) {
                        auto it = find_if(m_mapKeySwitch.begin(), m_mapKeySwitch.end(),
                            [&token](const auto& entry) {
                                return entry.second->key == token;
                            });
                        if (it == m_mapKeySwitch.end()) {
                            cerr << "Unrecognized shortcut key passed " << token << std::endl;
                            printHelp();
                            return -1;
                        }

                        CmdSwitch* cmd = it->second;
                        cmd->isvalid = true;
                        cmd->value = "true";
                        ++ctOptions;
                    }
                }
                return ctOptions;
            }

            string value(const char* key) const {
                string strKey(key);
                if (!starts_with(strKey, "--"))
                    strKey = "--" + strKey;

                auto it = m_mapKeySwitch.find(strKey);
                if (it != m_mapKeySwitch.end())
                    return it->second->value;
                return "";
            }

            bool value_to_bool(const char* key) const {
                return value(key) == "true";
            }

            int value_to_int(const char* key) const {
                string strVal = value(key);
                return strVal.empty() ? 0 : stoi(strVal);
            }

            double value_to_double(const char* key) const {
                string strVal = value(key);
                return strVal.empty() ? 0.0 : stod(strVal);
            }

            bool isValid(const char* key) const {
                string strKey(key);
                if (!starts_with(strKey, "--"))
                    strKey = "--" + strKey;

                auto it = m_mapKeySwitch.find(strKey);
                return it != m_mapKeySwitch.end() && it->second->isvalid;
            }

            void printHelp() const {
                cout << "Usage: " << m_appname << " [options]\n\n";
                for (const auto& entry : m_mapKeySwitch) {
                    const CmdSwitch* cmd = entry.second;
                    cout << cmd->key << " : " << cmd->desc;
                    if (cmd->istoggle)
                        cout << " [boolean flag]";
                    else
                        cout << " [default: " << cmd->default_value << "]";
                    cout << endl;
                }
            }

        private:
            bool starts_with(const string& src, const string& sub) const {
                return src.find(sub) == 0;
            }

            string m_appname;
            map<string, CmdSwitch*> m_mapKeySwitch;
        };

    }
}

#endif 
