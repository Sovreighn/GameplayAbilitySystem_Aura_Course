// Copyright Sovreighn Gaming

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"
#include "AuraFireBlast.generated.h"

class AAuraFireball;

UCLASS()
class AURA_API UAuraFireBlast : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	TArray<AAuraFireball*> SpawnFireballs();
	
	virtual FString GetDescription(int32 InLevel) override;
	virtual FString GetNextLevelDescription(int32 InLevel) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Fireblast")
	int32 NumberOfFireballs = 12;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Fireblast")
	TSubclassOf<AAuraFireball> FireballClass;	
	
};
