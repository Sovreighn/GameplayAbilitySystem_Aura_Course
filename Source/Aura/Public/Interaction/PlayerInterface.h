// Copyright Sovreighn Gaming

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerInterface.generated.h"

UINTERFACE(MinimalAPI)
class UPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

class AURA_API IPlayerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void AddToXp(int32 InXP);

	UFUNCTION(BlueprintNativeEvent)
	void AddToPlayerLevel(int32 InPlayerLevel);

	UFUNCTION(BlueprintNativeEvent)
	void AddToAttributePoints(int32 InAttributePoints);

	UFUNCTION(BlueprintNativeEvent)
	void AddToSpellPoints(int32 InSpellPoints);

	UFUNCTION(BlueprintNativeEvent)
	int32 FindLevelForXp(int32 InXP) const;
	
	UFUNCTION(BlueprintNativeEvent)
	int32 GetXp() const;

	UFUNCTION(BlueprintNativeEvent)
	int32 GetAttributePointsReward(int32 Level);

	UFUNCTION(BlueprintNativeEvent)
	int32 GetSpellPointsReward(int32 Level);
	
	UFUNCTION(BlueprintNativeEvent)
	void LevelUp();
	
};
