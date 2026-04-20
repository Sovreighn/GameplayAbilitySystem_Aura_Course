// Copyright Sovreighn Gaming

#include "AbilitySystem/Abilities/AuraFireBolt.h"

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