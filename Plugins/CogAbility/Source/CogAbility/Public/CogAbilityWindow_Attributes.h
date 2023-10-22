#pragma once

#include "CoreMinimal.h"
#include "CogWindow.h"
#include "CogAbilityWindow_Attributes.generated.h"

class UAbilitySystemComponent;
class UCogAbilityDataAsset;
struct FGameplayAttribute;
struct FModifierSpec;
struct FGameplayModifierInfo;

UCLASS(Config = Cog)
class COGABILITY_API UCogAbilityWindow_Attributes : public UCogWindow
{
    GENERATED_BODY()

public:

    UCogAbilityWindow_Attributes();

protected:

    virtual void ResetConfig() override;

    virtual void RenderHelp() override;

    virtual void RenderContent() override;

    virtual void DrawAttributeInfo(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayAttribute& Attribute);

    virtual ImVec4 GetEffectModifierColor(const FModifierSpec& ModSpec, const FGameplayModifierInfo& ModInfo, float BaseValue) const;

    virtual ImVec4 GetAttributeColor(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayAttribute& Attribute) const;

private:

    UPROPERTY(Config)
    bool bSortByName = true;

    UPROPERTY(Config)
    bool bGroupByAttributeSet = false;

    UPROPERTY(Config)
    bool bGroupByCategory = false;

    UPROPERTY(Config)
    bool bShowOnlyModified = false;

    ImGuiTextFilter Filter;

    UPROPERTY()
    TObjectPtr<const UCogAbilityDataAsset> Asset = nullptr;
};
