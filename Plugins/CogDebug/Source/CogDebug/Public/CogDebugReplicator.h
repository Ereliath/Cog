#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CogDebugShape.h"
#include "CogDebugLogBlueprint.h"
#include "UObject/Class.h"
#include "UObject/ObjectMacros.h"
#include "CogDebugReplicator.generated.h"

class ACogDebugReplicator;
class APlayerController;

//--------------------------------------------------------------------------------------------------------------------------

USTRUCT()
struct FCogServerCategoryData
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    FName LogCategoryName;

    UPROPERTY()
    ECogLogVerbosity Verbosity = ECogLogVerbosity::Fatal;
};

//--------------------------------------------------------------------------------------------------------------------------

USTRUCT()
struct FCogReplicatorNetPack
{
    GENERATED_USTRUCT_BODY()

    ACogDebugReplicator* Owner = nullptr;

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);

private:

    TArray<FCogDebugShape> SavedShapes;
};

//--------------------------------------------------------------------------------------------------------------------------
template<>
struct TStructOpsTypeTraits<FCogReplicatorNetPack> : public TStructOpsTypeTraitsBase2<FCogReplicatorNetPack>
{
    enum
    {
        WithNetDeltaSerializer = true,
    };
};

//--------------------------------------------------------------------------------------------------------------------------
UCLASS(NotBlueprintable, NotBlueprintType, notplaceable, noteditinlinenew, hidedropdown, Transient)
class COGDEBUG_API ACogDebugReplicator : public AActor
{
    GENERATED_UCLASS_BODY()

public:
    static void Create(APlayerController* Controller);

    virtual void BeginPlay() override;
    virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;

    APlayerController* GetPlayerController() const { return OwnerPlayerController.Get(); }

    bool IsLocal() const { return bIsLocal; }

    TArray<FCogDebugShape> ReplicatedShapes;

    UFUNCTION(Server, Reliable)
    void Server_RequestAllCategoriesVerbosity();

    UFUNCTION(Server, Reliable)
    void Server_SetCategoryVerbosity(FName LogCategoryName, ECogLogVerbosity Verbosity);

    UFUNCTION(NetMulticast, Reliable)
    void NetMulticast_SendCategoriesVerbosity(const TArray<FCogServerCategoryData>& Categories);
    
    UFUNCTION(Client, Reliable)
    void Client_SendCategoriesVerbosity(const TArray<FCogServerCategoryData>& Categories);

protected:
    friend FCogReplicatorNetPack;

    TObjectPtr<APlayerController> OwnerPlayerController;

    uint32 bHasAuthority : 1;
    uint32 bIsLocal : 1;

private:

    UPROPERTY(Replicated)
    FCogReplicatorNetPack ReplicatedData;
};