#include "Console.h"
#include "../Core/Log.h"

#include <imgui.h>
#include <sstream>

namespace Xi {

    Console::Console() {
        Clear();

        // Add default commands
        AddCommand("clear", [this](const std::vector<std::string>&) {
            Clear();
        });

        AddCommand("help", [this](const std::vector<std::string>&) {
            AddLog("Available commands:");
            for (const auto& cmd : m_Commands) {
                AddLog("  " + cmd.name);
            }
        });
    }

    Console::~Console() = default;

    void Console::Draw() {
        ImGui::SetNextWindowSize(ImVec2(520, 300), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Console")) {
            ImGui::End();
            return;
        }

        // Log window
        const float footerHeight = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footerHeight), false, ImGuiWindowFlags_HorizontalScrollbar);

        // Display log entries
        const auto& logEntries = Log::GetEntries();
        for (const auto& entry : logEntries) {
            ImVec4 color;
            switch (entry.level) {
                case LogLevel::Trace:   color = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); break;
                case LogLevel::Info:    color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); break;
                case LogLevel::Warning: color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); break;
                case LogLevel::Error:   color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f); break;
                default:                color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); break;
            }

            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextUnformatted(("[" + entry.timestamp + "] " + entry.message).c_str());
            ImGui::PopStyleColor();
        }

        // Custom logs
        for (const auto& log : m_Logs) {
            ImGui::TextUnformatted(log.c_str());
        }

        if (m_ScrollToBottom) {
            ImGui::SetScrollHereY(1.0f);
            m_ScrollToBottom = false;
        }

        ImGui::EndChild();

        // Command input
        ImGui::Separator();

        bool reclaimFocus = false;
        ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory;

        if (ImGui::InputText("##Input", m_InputBuffer, sizeof(m_InputBuffer), inputFlags,
            [](ImGuiInputTextCallbackData* data) -> int {
                Console* console = static_cast<Console*>(data->UserData);

                if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory) {
                    const int prevHistoryPos = console->m_HistoryPos;

                    if (data->EventKey == ImGuiKey_UpArrow) {
                        if (console->m_HistoryPos == -1) {
                            console->m_HistoryPos = static_cast<int>(console->m_History.size()) - 1;
                        } else if (console->m_HistoryPos > 0) {
                            console->m_HistoryPos--;
                        }
                    } else if (data->EventKey == ImGuiKey_DownArrow) {
                        if (console->m_HistoryPos != -1) {
                            if (++console->m_HistoryPos >= static_cast<int>(console->m_History.size())) {
                                console->m_HistoryPos = -1;
                            }
                        }
                    }

                    if (prevHistoryPos != console->m_HistoryPos) {
                        const char* historyStr = (console->m_HistoryPos >= 0) ?
                            console->m_History[console->m_HistoryPos].c_str() : "";
                        data->DeleteChars(0, data->BufTextLen);
                        data->InsertChars(0, historyStr);
                    }
                }

                return 0;
            }, this))
        {
            std::string cmd = m_InputBuffer;
            if (!cmd.empty()) {
                ExecuteCommand(cmd);
                m_History.push_back(cmd);
            }
            m_InputBuffer[0] = '\0';
            reclaimFocus = true;
        }

        ImGui::SetItemDefaultFocus();
        if (reclaimFocus) {
            ImGui::SetKeyboardFocusHere(-1);
        }

        ImGui::End();
    }

    void Console::Clear() {
        m_Logs.clear();
        m_ScrollToBottom = true;
    }

    void Console::AddLog(const std::string& message) {
        m_Logs.push_back(message);
        m_ScrollToBottom = true;
    }

    void Console::AddCommand(const std::string& name, CommandCallback callback) {
        m_Commands.push_back({name, callback});
    }

    void Console::ExecuteCommand(const std::string& commandLine) {
        AddLog("> " + commandLine);

        std::vector<std::string> args;
        std::istringstream iss(commandLine);
        std::string token;
        while (iss >> token) {
            args.push_back(token);
        }

        if (args.empty()) return;

        const std::string& cmdName = args[0];
        args.erase(args.begin());

        for (const auto& cmd : m_Commands) {
            if (cmd.name == cmdName) {
                cmd.callback(args);
                m_ScrollToBottom = true;
                return;
            }
        }

        AddLog("Unknown command: " + cmdName);
        m_ScrollToBottom = true;
    }

}
