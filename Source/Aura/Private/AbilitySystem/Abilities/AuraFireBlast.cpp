// Copyright Sovreighn Gaming

#include "AbilitySystem/Abilities/AuraFireBlast.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Actor/AuraFireball.h"

TArray<AAuraFireball*> UAuraFireBlast::SpawnFireballs()
{
	const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
	TArray<FRotator> Rotators = UAuraAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, 360.f, NumberOfFireballs);
	TArray<AAuraFireball*> Fireballs;
		
	for (const FRotator& Rotator : Rotators)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(Location);
		SpawnTransform.SetRotation(Rotator.Quaternion());
		AAuraFireball* Fireball = GetWorld()->SpawnActorDeferred<AAuraFireball>(
			FireballClass,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			CurrentActorInfo->PlayerController->GetPawn(),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		Fireball->DamageEffectParameters = MakeDamageEffectParamsFromClassDefaults();
		Fireball->ReturnToActor = GetAvatarActorFromActorInfo();

		Fireball->ExplosionDamageParameters = MakeDamageEffectParamsFromClassDefaults();
		Fireball->SetOwner(GetAvatarActorFromActorInfo());
		
		Fireballs.Add(Fireball);
		Fireball->FinishSpawning(SpawnTransform);
	}
	
	return Fireballs;
}

FString UAuraFireBlast::GetDescription(int32 InLevel)
{
	const float ScaledDamage = Damage.GetValueAtLevel(InLevel);
	const int32 RoundedDamage = FMath::RoundHalfFromZero(ScaledDamage);
	const float ManaCost = FMath::Abs(GetManaCost(InLevel));
	const float Cooldown = GetCooldown(InLevel);

	return FString::Printf(TEXT(
		// Title
		"<Title>FIRE BLAST</>\n\n"
		// Level
		"<Small>Level: </><Level>%d</>\n"
		// ManaCost
		"<Small>ManaCost: </><ManaCost>%.1f</>\n"
		// Cooldown
		"<Small>Cooldown: </><Cooldown>%.1f </><Small>Seconds</>\n\n"
		// Number of Fireballs
		"<Default>Launches </><Level>%d </>"
		"<Default>Fireballs in all directions, each returning, and exploding upon arrival, causing </>"
		// Damage
		"<Damage>%d</><Default> radial Fire damage, "
		"with a chance to burn.</>\n\n"),
		// Values
		InLevel,
		ManaCost,
		Cooldown,
		NumberOfFireballs,
		RoundedDamage);
}

FString UAuraFireBlast::GetNextLevelDescription(int32 InLevel)
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
		// Number of Fireballs
		"<Default>Launches </><Level>%d </>"
		"<Default>Fireballs in all directions, each returning, and exploding upon arrival, causing </>"
		// Damage
		"<Damage>%d</><Default> radial Fire damage, "
		"with a chance to burn.</>\n\n"),
		// Values
		InLevel,
		ManaCost,
		Cooldown,
		NumberOfFireballs,
		RoundedDamage);
}
