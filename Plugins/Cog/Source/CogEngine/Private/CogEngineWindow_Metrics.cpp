#include "CogEngineWindow_Metrics.h"

#include "CogDebugMetric.h"
#include "CogImguiHelper.h"
#include "CogWindowWidgets.h"
#include "imgui.h"

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Metrics::Initialize()
{
    Super::Initialize();

    bHasMenu = true;

    Config = GetConfig<UCogEngineConfig_Metrics>();

    FCogDebugMetric::MaxDurationSetting = Config->MaxDurationSetting;
    FCogDebugMetric::RestartDelaySetting = Config->RestartDelaySetting;
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Metrics::RenderHelp()
{
    ImGui::Text(
        "This window gather events generated by the selected actor to compute how much output it produces or receives per second. "
        "This is typically useful to compute the damage dealt per second, the damage received per second, etc. "
    );
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Metrics::ResetConfig()
{
    Super::ResetConfig();

    Config->Reset();
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Metrics::PreSaveConfig()
{
    Super::PreSaveConfig();

    Config->MaxDurationSetting = FCogDebugMetric::MaxDurationSetting;
    Config->RestartDelaySetting = FCogDebugMetric::RestartDelaySetting;
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Metrics::RenderTick(float DeltaTime)
{
    Super::RenderTick(DeltaTime);
    FCogDebugMetric::IsVisible = GetIsVisible();

    FCogDebugMetric::Tick(DeltaTime);
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Metrics::RenderContent()
{
    Super::RenderContent();

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Options"))
        {
            bool bSettingModified = false;

            FCogWindowWidgets::PushStyleCompact();
            ImGui::DragFloat("Auto Restart Delay", &FCogDebugMetric::RestartDelaySetting, 0.1f, 0.0f, FLT_MAX, "%0.1f");
            FCogWindowWidgets::PopStyleCompact();

            FCogWindowWidgets::PushStyleCompact();
            ImGui::DragFloat("Max Time", &FCogDebugMetric::MaxDurationSetting, 0.1f, 0.0f, FLT_MAX, "%0.1f");
            FCogWindowWidgets::PopStyleCompact();

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    if (GetWorld()->GetNetMode() == ENetMode::NM_Client)
    {
        ImGui::Text("Currently not available on client");
        return;
    }

    if (FCogDebugMetric::Metrics.IsEmpty())
    {
        ImGui::Text("No metric received yet");
        return;
    }

    int32 Index = 0;
    for (auto& Entry : FCogDebugMetric::Metrics)
    {
        FName MetricName = Entry.Key;
        FCogDebugMetricEntry& Metric = Entry.Value;

        if (ImGui::CollapsingHeader(TCHAR_TO_ANSI(*MetricName.ToString()), ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::PushID(Index);
            DrawMetric(Metric);
            ImGui::PopID();
        }

        Index++;
    }
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Metrics::DrawMetric(FCogDebugMetricEntry& Metric)
{
    FCogWindowWidgets::PushBackColor(ImVec4(0.8f, 0.8f, 0.8f, 1.0f));

    if (ImGui::BeginTable("MetricTable", 4, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoBordersInBodyUntilResize | ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("");
        ImGui::TableSetupColumn("Mitigated");
        ImGui::TableSetupColumn("Unmitigated");
        ImGui::TableSetupColumn("Mitigation %");
        ImGui::TableHeadersRow();

        DrawMetricRow("Per Second", Metric.Mitigated.PerSecond, Metric.Unmitigated.PerSecond, ImVec4(1.0f, 1.0, 0.0f, 1.0f));
        DrawMetricRow("Total", Metric.Mitigated.Total, Metric.Unmitigated.Total, ImVec4(1.0f, 1.0, 1.0f, 1.0f));
        DrawMetricRow("Last", Metric.Mitigated.Last, Metric.Unmitigated.Last, ImVec4(1.0f, 1.0, 1.0f, 1.0f));
        DrawMetricRow("Min", Metric.Mitigated.Min, Metric.Unmitigated.Min, ImVec4(1.0f, 1.0, 1.0f, 1.0f));
        DrawMetricRow("Min", Metric.Mitigated.Max, Metric.Unmitigated.Max, ImVec4(1.0f, 1.0, 1.0f, 1.0f));

        ImGui::EndTable();
    }

    ImGui::Text("Crits");
    ImGui::SameLine(FCogWindowWidgets::GetFontWidth() * 20);
    FCogWindowWidgets::ProgressBarCentered(Metric.Count == 0 ? 0.0f : Metric.Crits / (float)Metric.Count, ImVec2(-1, 0), TCHAR_TO_ANSI(*FString::Printf(TEXT("%d / %d"), Metric.Crits, Metric.Count)));

    if (FCogDebugMetric::MaxDurationSetting > 0.0f)
    {
        ImGui::Text("Timer");
        ImGui::SameLine(FCogWindowWidgets::GetFontWidth() * 20);
        FCogWindowWidgets::ProgressBarCentered(Metric.Timer / (float)FCogDebugMetric::MaxDurationSetting, ImVec2(-1, 0), TCHAR_TO_ANSI(*FString::Printf(TEXT("%0.1f / %0.1f"), Metric.Timer, FCogDebugMetric::MaxDurationSetting)));
    }
    else
    {
        ImGui::Text("Timer");
        ImGui::SameLine(FCogWindowWidgets::GetFontWidth() * 20);
        ImGui::Text("%0.1f", Metric.Timer);
    }

    ImGui::Spacing();

    FCogWindowWidgets::PopBackColor();

    if (ImGui::Button("Restart"))
    {
        Metric.Reset();
    }

    ImGui::Spacing();
    ImGui::Spacing();
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Metrics::DrawMetricRow(const char* RowTitle, float MitigatedValue, float UnmitigatedValue, const ImVec4& Color)
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Selectable(RowTitle, false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_AllowDoubleClick);
    ImGui::TableNextColumn();
    ImGui::TextColored(Color, "%.1f", MitigatedValue);
    ImGui::TableNextColumn();
    ImGui::Text("%.1f", UnmitigatedValue);
    ImGui::TableNextColumn();
    ImGui::Text("%.0f%%", UnmitigatedValue <= 0 ? 0.0 : 100.0f * (1.0f - (MitigatedValue / UnmitigatedValue)));
}

