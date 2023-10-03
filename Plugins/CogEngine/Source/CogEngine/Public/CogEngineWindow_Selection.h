#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CogWindow.h"
#include "CogEngineWindow_Selection.generated.h"

UCLASS()
class COGENGINE_API UCogEngineWindow_Selection : public UCogWindow
{
    GENERATED_BODY()

public:
    UCogEngineWindow_Selection();

    virtual void RenderTick(float DeltaTime);

    virtual void PreRender(ImGuiWindowFlags& WindowFlags) override;

    virtual void RenderContent() override;

    virtual void DrawMainMenuWidget(bool Draw, float& Width) override;

    bool DrawSelectionCombo();

    void DrawActorContextMenu(AActor* Actor);

    void ActivateSelectionMode();

    void HackWaitInputRelease();

    bool GetIsSelecting() const { return bSelectionModeActive; }

    void SetCurrentActorSubClass(TSubclassOf<AActor> Value) { SelectedSubClass = Value; }

    TSubclassOf<AActor> GetCurrentActorSubClass() const { return SelectedSubClass; }

    const TArray<TSubclassOf<AActor>>& GetActorSubClasses() const { return SubClasses; }

    void SetActorSubClasses(const TArray<TSubclassOf<AActor>>& Value) { SubClasses = Value; }

    ETraceTypeQuery GetTraceType() const { return TraceType; }

    void SetTraceType(ETraceTypeQuery Value) { TraceType = Value; }

private:

    void TickSelectionMode();

    void ToggleSelectionMode();

    void DeactivateSelectionMode();

    void DrawActorFrame(const AActor* Actor);

    //void DrawActorDebug(const AActor* Actor);

    bool ComputeBoundingBoxScreenPosition(const APlayerController* PlayerController, const FVector& Origin, const FVector& Extent, FVector2D& Min, FVector2D& Max);

    FVector LastSelectedActorLocation = FVector::ZeroVector;

    bool bSelectionModeActive = false;

    bool bImGuiHadInputBeforeEnteringSelectionMode = false;

    int32 WaitInputReleased = 0;

    TSubclassOf<AActor> SelectedSubClass;

    TArray<TSubclassOf<AActor>> SubClasses;

    ETraceTypeQuery TraceType = TraceTypeQuery1;
};