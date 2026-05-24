// Copyright Sovreighn Gaming

#include "AbilitySystem/Abilities/Electrocute.h"

FString UElectrocute::GetDescription(int32 InLevel)
{
	const float ScaledDamage = Damage.GetValueAtLevel(InLevel);
	const int32 RoundedDamage = FMath::RoundHalfFromZero(ScaledDamage);
	const float ManaCost = FMath::Abs(GetManaCost(InLevel));
	const float Cooldown = GetCooldown(InLevel);

	if (InLevel == 1)
	{
		return FString::Printf(TEXT(
			// Title
			"<Title>ELECTROCUTE</>\n\n"
			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f </><Small>Seconds</>\n\n"
			// Damage
			"<Default>Emits a beam of Lightning, "
			"connecting with the Target, repeatedly causing </>"
			"<Damage>%d</><Default> Lightning damage, "
			"with a chance to Stun.</>\n\n"),
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
			"<Title>ELECTROCUTE</>\n\n"
			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f </><Small>Seconds</>\n\n"
			// Additional NumberOfShockedTargets
			"<Default>Emits a beam of Lightning, "
			"propagating to </><Level>%d</> additional Targets nearby, causing "
			// Damage
			"<Damage>%d</><Default> Lightning damage, "
			"with a chance to Stun.</>\n\n"),
			// Values
			InLevel,
			ManaCost,
			Cooldown,
			FMath::Min(InLevel - 1,MaxNumberOfShockTargets),
			RoundedDamage);
	}
}

FString UElectrocute::GetNextLevelDescription(int32 InLevel)
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
			// Additional NumberOfShockedTargets
			"<Default>Emits a beam of Lightning, "
			"propagating to </><Level>%d</> additional Targets nearby, causing "
			// Damage
			"<Damage>%d</><Default> Lightning damage, "
			"with a chance to Stun.</>\n\n"),
			// Values
			InLevel,
			ManaCost,
			Cooldown,
			FMath::Min(InLevel - 1,MaxNumberOfShockTargets),
			RoundedDamage);
}
