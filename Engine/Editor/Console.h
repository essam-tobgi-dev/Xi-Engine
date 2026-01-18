#pragma once

#include <string>
#include <vector>
#include <functional>

namespace Xi {

    class Console {
    public:
        using CommandCallback = std::function<void(const std::vector<std::string>&)>;

        Console();
        ~Console();

        void Draw();
        void Clear();

        void AddLog(const std::string& message);
        void AddCommand(const std::string& name, CommandCallback callback);
        void ExecuteCommand(const std::string& commandLine);

    private:
        struct Command {
            std::string name;
            CommandCallback callback;
        };

        std::vector<std::string> m_Logs;
        std::vector<Command> m_Commands;
        char m_InputBuffer[256] = {};
        bool m_ScrollToBottom = false;
        std::vector<std::string> m_History;
        int m_HistoryPos = -1;
    };

}
