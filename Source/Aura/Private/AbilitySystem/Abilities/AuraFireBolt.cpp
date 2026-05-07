// Copyright Sovreighn Gaming

#include "AbilitySystem/Abilities/AuraFireBolt.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Interaction/CombatInterface.h"
#include "Actor/AuraProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"

FString UAuraFireBolt::GetDescription(int32 InLevel)
{
	const float ScaledDamage = Damage.GetValueAtLevel(InLevel);
	const int32 RoundedDamage = FMath::RoundHalfFromZero(ScaledDamage);
	const float ManaCost = FMath::Abs(GetManaCost(InLevel));
	const float Cooldown = GetCooldown(InLevel);

	if (InLevel == 1)
	{
		return FString::Printf(TEXT(
			// Title
			"<Title>FIRE BOLT</>\n\n"
			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f </><Small>Seconds</>\n\n"
			// Damage
			"<Default>Launches a bolt of fire, "
			"exploding on impact, dealing </>"
			"<Damage>%d</><Default> fire damage, "
			"with a chance to burn.</>\n\n"),
			// Values
			InLevel,
			ManaCost,
			Cooldown,
			RoundedDamage);
	}
	else
	{
		return FString::Printf(TEXT(
			// Title
			"<Title>FIRE BOLT</>\n\n"
			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f </><Small>Seconds</>\n\n"
			// NumberOfProjectiles
			"<Default>Launches </><Level>%d</> bolts of fire, "
			"exploding on impact, dealing "
			// Damage
			"<Damage>%d</><Default> fire damage, "
			"with a chance to burn.</>\n\n"),
			// Values
			InLevel,
			ManaCost,
			Cooldown,
			FMath::Min(InLevel,NumberOfProjectiles),
			RoundedDamage);
	}
}

FString UAuraFireBolt::GetNextLevelDescription(int32 InLevel)
{
	const float ScaledDamage = Damage.GetValueAtLevel(InLevel);
	const int32 RoundedDamage = FMath::RoundHalfFromZero(ScaledDamage);
	const float ManaCost = FMath::Abs(GetManaCost(InLevel));
	const float Cooldown = GetCooldown(InLevel);

	return FString::Printf(TEXT(
		// Title
		"<Title>NEXT LEVEL</>\n\n"
		// Level
		"<Small>Level: </><Level>%d</>\n"
		// ManaCost
		"<Small>ManaCost: </><ManaCost>%.1f</>\n"
		// Cooldown
		"<Small>Cooldown: </><Cooldown>%.1f </><Small>Seconds</>\n\n"
		// NumberOfProjectiles
		"<Default>Launches </><Level>%d</> bolts of fire, "
		"exploding on impact, dealing "
		// Damage
		"<Damage>%d</><Default> fire damage, "
		"with a chance to burn.</>\n\n"),
		// Values
		InLevel,
		ManaCost,
		Cooldown,
		FMath::Min(InLevel,NumberOfProjectiles),
		RoundedDamage);
}

void UAuraFireBolt::SpawnProjectiles(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverrideAmount, AActor* HomingTarget)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;
	
	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo(), SocketTag);
	FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
	if (bOverridePitch) { Rotation.Pitch = PitchOverrideAmount; }

	const FVector Forward = Rotation.Vector();
	const int32 EffectiveNumberOfProjectiles = FMath::Min(NumberOfProjectiles, GetAbilityLevel());
	TArray<FRotator> Rotations = UAuraAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, ProjectileSpread, EffectiveNumberOfProjectiles);
	
	for (const FRotator& Rot : Rotations)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);
		SpawnTransform.SetRotation(Rot.Quaternion());

		AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
		ProjectileClass,
		SpawnTransform,
		GetOwningActorFromActorInfo(),
		Cast<APawn>(GetAvatarActorFromActorInfo()),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	
		Projectile->DamageEffectParameters = MakeDamageEffectParamsFromClassDefaults();

		if (HomingTarget && HomingTarget->Implements<UCombatInterface>())
		{
			Projectile->ProjectileMovement->HomingTargetComponent = HomingTarget->GetRootComponent();
		}
		else
		{
			Projectile->HomingTargetSceneComponent = NewObject<USceneComponent>(USceneComponent::StaticClass());
			Projectile->HomingTargetSceneComponent->SetWorldLocation(ProjectileTargetLocation);
			// To mark the NEW USceneComponent for GarbageCollection
			Projectile->ProjectileMovement->HomingTargetComponent = Projectile->HomingTargetSceneComponent;
		}
		
		Projectile->ProjectileMovement->HomingAccelerationMagnitude = FMath::FRandRange(HomingAccelerationMinMagnitude, HomingAccelerationMaxMagnitude);
		Projectile->ProjectileMovement->bIsHomingProjectile = bLaunchHomingProjectiles;
		Projectile->FinishSpawning(SpawnTransform);
	}
}
