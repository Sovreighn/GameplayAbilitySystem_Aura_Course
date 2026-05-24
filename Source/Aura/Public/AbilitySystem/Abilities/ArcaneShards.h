// Copyright Sovreighn Gaming

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"
#include "ArcaneShards.generated.h"

UCLASS()
class AURA_API UArcaneShards : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	int32 MaxNumberOfShards = 10;
	
	virtual FString GetDescription(int32 InLevel) override;
	virtual FString GetNextLevelDescription(int32 InLevel) override;
	
};
