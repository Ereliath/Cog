#include "CogAbilityWindow_Tags.h"

#include "AbilitySystemComponent.h"
#include "CogAbilityHelper.h"
#include "CogWindowWidgets.h"

//--------------------------------------------------------------------------------------------------------------------------
static void DrawTagInfo(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTag& Tag)
{
    if (ImGui::BeginTable("Tag", 2, ImGuiTableFlags_Borders))
    {
        const ImVec4 TextColor(1.0f, 1.0f, 1.0f, 0.5f);

        ImGui::TableSetupColumn("Property");
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

        //------------------------
        // Name
        //------------------------
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(TextColor, "Name");
        ImGui::TableNextColumn();
        ImGui::Text("%s", TCHAR_TO_ANSI(*Tag.GetTagName().ToString()));

        //------------------------
        // Base Value
        //------------------------
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(TextColor, "Count");
        ImGui::TableNextColumn();
        const int32 TagCount = AbilitySystemComponent.GetTagCount(Tag);
        ImGui::TextColored(TagCount > 1 ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%d", TagCount);

        //------------------------
        // Abilities
        //------------------------
        //ImGui::TableNextRow();
        //ImGui::TableNextColumn();
        //ImGui::TextColored(TextColor, "Abilities");
        //ImGui::TableNextColumn();
        //for (FGameplayAbilitySpec AbilitySpec : AbilitySystemComponent.GetActivatableAbilities())
        //{
        //    UGameplayAbility* Ability = AbilitySpec.GetPrimaryInstance() != nullptr ? AbilitySpec.GetPrimaryInstance() : AbilitySpec.Ability;
        //    if (Ability == nullptr)
        //    {
        //        continue;
        //    }

        //    if (Ability->ActivationOwnedTags  GetActivationOwnedTags().HasTagExact(Tag))
        //    {
        //        ImGui::Text("%s", TCHAR_TO_ANSI(*FCogAbilityHelper::CleanupName(GetNameSafe(Ability))));
        //    }
        //}

        //------------------------
        // Effects
        //------------------------
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(TextColor, "Effects");
        ImGui::TableNextColumn();
        FGameplayEffectQuery Query;
        for (const FActiveGameplayEffectHandle& ActiveHandle : AbilitySystemComponent.GetActiveEffects(Query))
        {
            const FActiveGameplayEffect* ActiveEffect = AbilitySystemComponent.GetActiveGameplayEffect(ActiveHandle);
            if (ActiveEffect == nullptr)
            {
                continue;
            }

            FGameplayTagContainer Container;
            ActiveEffect->Spec.GetAllGrantedTags(Container);

            if (Container.HasTagExact(Tag))
            {
                ImGui::Text("%s", TCHAR_TO_ANSI(*FCogAbilityHelper::CleanupName(GetNameSafe(ActiveEffect->Spec.Def))));
            }
        }

        ImGui::EndTable();
    }
}


//--------------------------------------------------------------------------------------------------------------------------
static void DrawTags(const FString& Title, UAbilitySystemComponent& AbilitySystemComponent, FGameplayTagContainer& TagContainer)
{
    if (ImGui::BeginTable(TCHAR_TO_ANSI(*Title), 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoBordersInBodyUntilResize | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_BordersOuterV))
    {
        ImGui::TableSetupColumn("Count");
        ImGui::TableSetupColumn(TCHAR_TO_ANSI(*Title));
        ImGui::TableHeadersRow();

        TArray<FGameplayTag> Tags;
        TagContainer.GetGameplayTagArray(Tags);

        static int Selected = -1;
        int index = 0;

        for (const FGameplayTag& Tag : Tags)
        {
            ImGui::TableNextRow();

            ImGui::PushID(index);

            //------------------------
            // Count
            //------------------------
            ImGui::TableNextColumn();
            const int32 TagCount = AbilitySystemComponent.GetTagCount(Tag);
            ImGui::TextColored(TagCount > 1 ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%d", TagCount);

            //------------------------
            // Name
            //------------------------
            ImGui::TableNextColumn();
            if (ImGui::Selectable(TCHAR_TO_ANSI(*Tag.GetTagName().ToString()), Selected == index, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_AllowDoubleClick))
            {
                Selected = index;
            }

            //------------------------
            // Popup
            //------------------------
            if (ImGui::IsItemHovered())
            {
                FCogWindowWidgets::BeginTableTooltip();
                DrawTagInfo(AbilitySystemComponent, Tag);
                FCogWindowWidgets::EndTableTooltip();
            }


            ImGui::PopID();
            index++;
        }

        ImGui::EndTable();
    }
}

//--------------------------------------------------------------------------------------------------------------------------

UCogAbilityWindow_Tags::UCogAbilityWindow_Tags()
{
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogAbilityWindow_Tags::PreRender(ImGuiWindowFlags& WindowFlags)
{
    WindowFlags = ImGuiWindowFlags_MenuBar;
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogAbilityWindow_Tags::RenderContent()
{
    Super::RenderContent();

    UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetSelection(), true);
    if (AbilitySystemComponent == nullptr)
    {
        return;
    }

    FGameplayTagContainer OwnedTags, BlockedTags;
    AbilitySystemComponent->GetOwnedGameplayTags(OwnedTags);
    AbilitySystemComponent->GetBlockedAbilityTags(BlockedTags);

    DrawTags("Owned Tags", *AbilitySystemComponent, OwnedTags);
}