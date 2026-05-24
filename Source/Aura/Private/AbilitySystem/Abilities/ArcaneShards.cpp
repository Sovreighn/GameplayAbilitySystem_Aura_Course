// Copyright Sovreighn Gaming

#include "AbilitySystem/Abilities/ArcaneShards.h"

FString UArcaneShards::GetDescription(int32 InLevel)
{
	const float ScaledDamage = Damage.GetValueAtLevel(InLevel);
	const int32 RoundedDamage = FMath::RoundHalfFromZero(ScaledDamage);
	const float ManaCost = FMath::Abs(GetManaCost(InLevel));
	const float Cooldown = GetCooldown(InLevel);

	if (InLevel == 1)
	{
		return FString::Printf(TEXT(
			// Title
			"<Title>ARCANE SHARDS</>\n\n"
			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f </><Small>Seconds</>\n\n"
			// Damage
			"<Default>Summon a shard of Arcane energy, "
			"causing radial Arcane damage of </>"
			"<Damage>%d</><Default> at the shard origin, "
			"with a chance to Knockback enemies.</>\n\n"),
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
			"<Title>ARCANE SHARDS</>\n\n"
			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f </><Small>Seconds</>\n\n"
			// Additional NumberOfShards
			"<Default>Summon </><Level>%d</> shards of Arcane energy, "
			"causing radial Arcane damage of "
			// Damage
			"<Damage>%d</><Default> at the shards origins, "
			"with a chance to Knockback enemies.</>\n\n"),
			// Values
			InLevel,
			ManaCost,
			Cooldown,
			FMath::Min(InLevel,MaxNumberOfShards),
			RoundedDamage);
	}
}

FString UArcaneShards::GetNextLevelDescription(int32 InLevel)
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
			// Additional NumberOfShards
			"<Default>Summon </><Level>%d</> shards of Arcane energy, "
			"causing radial Arcane damage of "
			// Damage
			"<Damage>%d</><Default> at the shards origins, "
			"with a chance to Knockback enemies.</>\n\n"),
			// Values
			InLevel,
			ManaCost,
			Cooldown,
			FMath::Min(InLevel,MaxNumberOfShards),
			RoundedDamage);
}
