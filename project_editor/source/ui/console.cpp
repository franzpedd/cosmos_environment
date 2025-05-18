#include "ui/console.h"

namespace Cosmos::Editor
{
	Console::Console()
		: Widget("Console", true)
	{
	}

	void Console::OnUpdate()
	{
		if (mVisible) {

			ImGui::Begin(ICON_FA_TERMINAL " Console", nullptr, ImGuiWindowFlags_HorizontalScrollbar);

			auto& messages = LoggerTracer::GetInstance().GetMessagesRef();
			for (size_t i = 0; i < messages.size() ; i++) {
				ImVec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

				switch (messages[i].severity) {
					case LogSeverity::Trace: {
						color = ImVec4 { 0.0f, 0.5f, 0.6f, 1.0f };
						ImGui::TextColored(color, ICON_FA_INFO_CIRCLE " %s", messages[i].message.c_str());
						break;
					}

					case LogSeverity::Todo: {
						color = ImVec4{ 0.0f, 1.0f, 0.0f, 1.0f };
						ImGui::TextColored(color, ICON_FA_HEART " %s", messages[i].message.c_str());
						break;
					}

					case LogSeverity::Info: {
						color = ImVec4{ 0.0f, 0.86f, 1.0f, 1.0f };
						ImGui::TextColored(color, ICON_FA_INFO_CIRCLE " %s", messages[i].message.c_str());
						break;
					}

					case LogSeverity::Warn: {
						color = ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f };
						ImGui::TextColored(color, ICON_FA_QUESTION_CIRCLE " %s", messages[i].message.c_str());
						break;
					}

					case LogSeverity::Error: {
						color = ImVec4{ 1.0f, 0.65f, 0.0f, 1.0f };
						ImGui::TextColored(color, ICON_FA_QUESTION_CIRCLE " %s", messages[i].message.c_str());
						break;
					}
				}
			}

			ImGui::End();
		}
	}
}