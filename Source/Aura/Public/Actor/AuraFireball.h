// Copyright Sovreighn Gaming

#pragma once

#include "CoreMinimal.h"
#include "Actor/AuraProjectile.h"
#include "AuraFireball.generated.h"

UCLASS()
class AURA_API AAuraFireball : public AAuraProjectile
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> ReturnToActor;

	UPROPERTY(BlueprintReadWrite)
	FDamageEffectParameters ExplosionDamageParameters;
	
	UFUNCTION(BlueprintImplementableEvent)
	void StartOutgoingTimeline();

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void BeginPlay() override;
	virtual void OnHit();
	
};
