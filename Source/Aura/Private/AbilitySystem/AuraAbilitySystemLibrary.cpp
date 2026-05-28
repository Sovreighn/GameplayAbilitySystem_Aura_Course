// Copyright Sovreighn Gaming

#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AuraAbilityTypes.h"
#include "AuraGameplayTags.h"
#include "Game/AuraGameModeBase.h"
#include "Interaction/CombatInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Player/AuraPlayerState.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "UI/HUD/AuraHUD.h"

// Widget Controller
bool UAuraAbilitySystemLibrary::MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWCParams, AAuraHUD*& OutAuraHUD)
{
	if(APlayerController* PlayerController = WorldContextObject->GetWorld()->GetFirstPlayerController())
	{
		OutAuraHUD = Cast<AAuraHUD>(PlayerController->GetHUD());
		if (OutAuraHUD)
		{
			AAuraPlayerState* PlayerState = PlayerController->GetPlayerState<AAuraPlayerState>();
			UAbilitySystemComponent* AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();
			UAttributeSet* AttributeSet = PlayerState->GetAttributeSet();
			
			OutWCParams.PlayerState = PlayerState;
			OutWCParams.AbilitySystemComponent = AbilitySystemComponent;
			OutWCParams.AttributeSet = AttributeSet;
			return true;
		}
	}

	return false;
}

UOverlayWidgetController* UAuraAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	AAuraHUD* AuraHUD = nullptr;
	
	if (MakeWidgetControllerParams(WorldContextObject,WCParams, AuraHUD))
	{
		return AuraHUD->GetOverlayWidgetController(WCParams);
	}

	return  nullptr;
}

UAttributeMenuWidgetController* UAuraAbilitySystemLibrary::GetAttributeMenuWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	AAuraHUD* AuraHUD = nullptr;
	
	if (MakeWidgetControllerParams(WorldContextObject,WCParams, AuraHUD))
	{
		return AuraHUD->GetAttributeMenuWidgetController(WCParams);
	}

	return  nullptr;
}

USpellMenuWidgetController* UAuraAbilitySystemLibrary::GetSpellMenuWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	AAuraHUD* AuraHUD = nullptr;
	
	if (MakeWidgetControllerParams(WorldContextObject,WCParams, AuraHUD))
	{
		return AuraHUD->GetSpellMenuWidgetController(WCParams);
	}

	return  nullptr;
}

// Ability System Class Defaults
void UAuraAbilitySystemLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC)
{
	AActor* AvatarActor = ASC->GetAvatarActor();

	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	FCharacterClassDefaultInfo ClassDefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);

	FGameplayEffectContextHandle PrimaryAttributesContextHandle = ASC->MakeEffectContext();
	PrimaryAttributesContextHandle.AddSourceObject(AvatarActor);

	FGameplayEffectContextHandle SecondaryAttributesContextHandle = ASC->MakeEffectContext();
	SecondaryAttributesContextHandle.AddSourceObject(AvatarActor);

	FGameplayEffectContextHandle VitalAttributesContextHandle = ASC->MakeEffectContext();
	VitalAttributesContextHandle.AddSourceObject(AvatarActor);
	
	const FGameplayEffectSpecHandle PrimaryAttributeSpecHandle = ASC->MakeOutgoingSpec(ClassDefaultInfo.PrimaryAttributes, Level, PrimaryAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*PrimaryAttributeSpecHandle.Data.Get());

	const FGameplayEffectSpecHandle SecondaryAttributeSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttributes, Level, SecondaryAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*SecondaryAttributeSpecHandle.Data.Get());

	const FGameplayEffectSpecHandle VitalAttributeSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->VitalAttributes, Level, VitalAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*VitalAttributeSpecHandle.Data.Get());
}

void UAuraAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);

	if (CharacterClassInfo == nullptr) return;
	
	for (TSubclassOf<UGameplayAbility> AbilityClass : CharacterClassInfo->CommonAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		ASC->GiveAbility(AbilitySpec);
	}

	const FCharacterClassDefaultInfo& DefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);

	for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultInfo.StartupAbilities)
	{
		if (ASC->GetAvatarActor()->Implements<UCombatInterface>())
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, ICombatInterface::Execute_GetPlayerLevel(ASC->GetAvatarActor()));
			ASC->GiveAbility(AbilitySpec);
		}
	}
}

// Gameplay Effect Context Getters
bool UAuraAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		return AuraEffectContext->IsBlockedHit();
	}
	
	return false;
}

bool UAuraAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		return AuraEffectContext->IsCriticalHit();
	}
	
	return false;
}

bool UAuraAbilitySystemLibrary::IsSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		return AuraEffectContext->IsSuccessfulDebuff();
	}
	
	return false;
}

bool UAuraAbilitySystemLibrary::IsRadialDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		return AuraEffectContext->IsRadialDamage();
	}
	
	return false;
}

UCharacterClassInfo* UAuraAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	const AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AuraGameMode == nullptr) { return nullptr; }

	return AuraGameMode->CharacterClassInfo;
}

UAbilityInfo* UAuraAbilitySystemLibrary::GetAbilityInfo(const UObject* WorldContextObject)
{
	const AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AuraGameMode == nullptr) { return nullptr; }

	return AuraGameMode->AbilityInfo;
}

float UAuraAbilitySystemLibrary::GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		return AuraEffectContext->GetDebuffDamage();
	}
	
	return 0.f;
}

float UAuraAbilitySystemLibrary::GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		return AuraEffectContext->GetDebuffDuration();
	}
	
	return 0.f;
}

float UAuraAbilitySystemLibrary::GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		return AuraEffectContext->GetDebuffFrequency();
	}
	
	return 0.f;
}

float UAuraAbilitySystemLibrary::GetRadialDamageInnerRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		return AuraEffectContext->GetRadialDamageInnerRadius();
	}
	
	return 0.f;
}

float UAuraAbilitySystemLibrary::GetRadialDamageOuterRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		return AuraEffectContext->GetRadialDamageOuterRadius();
	}
	
	return 0.f;
}

FGameplayTag UAuraAbilitySystemLibrary::GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		if (AuraEffectContext->GetDamageType().IsValid())
		{
			return *AuraEffectContext->GetDamageType();
		}
	}
	
	return FGameplayTag();
}

FVector UAuraAbilitySystemLibrary::GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		return AuraEffectContext->GetDeathImpulse();
	}
	
	return FVector::ZeroVector;
}

FVector UAuraAbilitySystemLibrary::GetKnockbackForce(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		return AuraEffectContext->GetKnockbackForce();
	}
	
	return FVector::ZeroVector;
}

FVector UAuraAbilitySystemLibrary::GetRadialDamageOrigin(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		return AuraEffectContext->GetRadialDamageOrigin();
	}
	
	return FVector::ZeroVector;
}

// Gameplay Effect Context Setters
void UAuraAbilitySystemLibrary::SetIsBlockedHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		AuraEffectContext->SetIsBlockedHit(bInIsBlockedHit);
	}
}

void UAuraAbilitySystemLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		AuraEffectContext->SetIsCriticalHit(bInIsCriticalHit);
	}
}

void UAuraAbilitySystemLibrary::SetIsSuccessfulDebuff(FGameplayEffectContextHandle& EffectContextHandle, bool bInSuccessfulDebuff)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		AuraEffectContext->SetIsSuccessfulDebuff(bInSuccessfulDebuff);
	}
}

void UAuraAbilitySystemLibrary::SetIsRadialDamage(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsRadialDamage)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		AuraEffectContext->SetIsRadialDamage(bInIsRadialDamage);
	}
}

void UAuraAbilitySystemLibrary::SetDebuffDamage(FGameplayEffectContextHandle& EffectContextHandle, float InDamage)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		AuraEffectContext->SetDebuffDamage(InDamage);
	}
}

void UAuraAbilitySystemLibrary::SetDebuffDuration(FGameplayEffectContextHandle& EffectContextHandle, float InDuration)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		AuraEffectContext->SetDebuffDuration(InDuration);
	}
}

void UAuraAbilitySystemLibrary::SetDebuffFrequency(FGameplayEffectContextHandle& EffectContextHandle, float InFrequency)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		AuraEffectContext->SetDebuffFrequency(InFrequency);
	}
}

void UAuraAbilitySystemLibrary::SetRadialDamageInnerRadius(FGameplayEffectContextHandle& EffectContextHandle, float InInnerRadius)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		AuraEffectContext->SetRadialDamageInnerRadius(InInnerRadius);
	}
}

void UAuraAbilitySystemLibrary::SetRadialDamageOuterRadius(FGameplayEffectContextHandle& EffectContextHandle, float InOuterRadius)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		AuraEffectContext->SetRadialDamageOuterRadius(InOuterRadius);
	}
}

void UAuraAbilitySystemLibrary::SetDamageType(FGameplayEffectContextHandle& EffectContextHandle, const FGameplayTag& InDamageType)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		const TSharedPtr<FGameplayTag> DamageType = MakeShared<FGameplayTag>(InDamageType);
		AuraEffectContext->SetDamageType(DamageType);
	}
}

void UAuraAbilitySystemLibrary::SetDeathImpulse(FGameplayEffectContextHandle& EffectContextHandle, const FVector& InImpulse)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		AuraEffectContext->SetDeathImpulse(InImpulse);
	}
}

void UAuraAbilitySystemLibrary::SetKnockbackForce(FGameplayEffectContextHandle& EffectContextHandle, const FVector& InForce)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		AuraEffectContext->SetKnockbackForce(InForce);
	}
}

void UAuraAbilitySystemLibrary::SetRadialDamageOrigin(FGameplayEffectContextHandle& EffectContextHandle, const FVector& InRadialDamageOrigin)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = GetAuraContext(EffectContextHandle))
	{
		AuraEffectContext->SetRadialDamageOrigin(InRadialDamageOrigin);
	}
}

// Gameplay Mechanics
void UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(const UObject* WorldContextObject,
                                                           TArray<AActor*>& OutOverlappingActors,
                                                           const TArray<AActor*>& ActorsToIgnore,
                                                           float Radius,
                                                           const FVector& SphereOrigin)
{
	FCollisionQueryParams SphereParams;
	SphereParams.AddIgnoredActors(ActorsToIgnore);

	TArray<FOverlapResult> Overlaps;
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		World->OverlapMultiByObjectType(Overlaps, SphereOrigin, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(Radius), SphereParams);
		for (FOverlapResult& Overlap : Overlaps)
		{
			if (Overlap.GetActor()->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsDead(Overlap.GetActor()))
			{
				OutOverlappingActors.AddUnique(ICombatInterface::Execute_GetAvatar(Overlap.GetActor()));
			}
		}
	}
}

void UAuraAbilitySystemLibrary::GetClosestTargets(int32 MaxTargets, const TArray<AActor*>& Actors, TArray<AActor*>& OutClosestTargets, const FVector& Origin)
{
	if (Actors.Num() <= MaxTargets)
	{
		OutClosestTargets = Actors;
		return;
	}

	TArray<AActor*> ActorsToCheck = Actors;
	int32 NumTargetsFound = 0;

	while (NumTargetsFound < MaxTargets)
	{
		if (ActorsToCheck.Num() == 0) { break; }
		
		double ClosestDistance = TNumericLimits<double>::Max();
		AActor* ClosestActor;
		for (AActor* PotentialTargetActor : ActorsToCheck)
		{
			const double Distance = (PotentialTargetActor->GetActorLocation() - Origin).Length();
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestActor = PotentialTargetActor;
			}
		}

		ActorsToCheck.Remove(ClosestActor);
		OutClosestTargets.AddUnique(ClosestActor);
		++NumTargetsFound;
	}
}

bool UAuraAbilitySystemLibrary::IsNotFriend(AActor* FirstActor, AActor* SecondActor)
{
	const bool bBothArePlayers = FirstActor->ActorHasTag(FName("Player")) && SecondActor->ActorHasTag(FName("Player"));
	const bool bBothAreEnemies = FirstActor->ActorHasTag(FName("Enemy")) && SecondActor->ActorHasTag(FName("Enemy"));
	const bool Friends = bBothArePlayers || bBothAreEnemies;
	return !Friends;
}

FGameplayEffectContextHandle UAuraAbilitySystemLibrary::ApplyDamageEffect(const FDamageEffectParameters& DamageEffectParameters)
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
	const AActor* SourceAvatarActor = DamageEffectParameters.SourceAbilitySystemComponent->GetAvatarActor();

	FGameplayEffectContextHandle EffectContextHandle = DamageEffectParameters.SourceAbilitySystemComponent->MakeEffectContext();
	EffectContextHandle.AddSourceObject(SourceAvatarActor);
	SetDeathImpulse(EffectContextHandle, DamageEffectParameters.DeathImpulse);
	SetKnockbackForce(EffectContextHandle, DamageEffectParameters.KnockbackForce);
	SetIsRadialDamage(EffectContextHandle, DamageEffectParameters.bIsRadialDamage);
	SetRadialDamageInnerRadius(EffectContextHandle, DamageEffectParameters.RadialDamageInnerRadius);
	SetRadialDamageOuterRadius(EffectContextHandle, DamageEffectParameters.RadialDamageOuterRadius);
	SetRadialDamageOrigin(EffectContextHandle, DamageEffectParameters.RadialDamageOrigin);
	const FGameplayEffectSpecHandle EffectSpecHandle = DamageEffectParameters.SourceAbilitySystemComponent->MakeOutgoingSpec(DamageEffectParameters.DamageGameplayEffectClass, DamageEffectParameters.AbilityLevel, EffectContextHandle);

	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, DamageEffectParameters.DamageType, DamageEffectParameters.BaseDamage);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, GameplayTags.Debuff_Chance, DamageEffectParameters.DebuffChance);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, GameplayTags.Debuff_Damage, DamageEffectParameters.DebuffDamage);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, GameplayTags.Debuff_Duration, DamageEffectParameters.DebuffDuration);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, GameplayTags.Debuff_Frequency, DamageEffectParameters.DebuffFrequency);

	DamageEffectParameters.TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data);
	
	return EffectContextHandle;
}

TArray<FRotator> UAuraAbilitySystemLibrary::EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, float Spread, int32 NumberOfRotators)
{
	TArray<FRotator> Rotators;
	
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);
	if (NumberOfRotators > 1)
	{
		const float DeltaSpread = Spread / (NumberOfRotators - 1);
		for (int32 i = 0; i < NumberOfRotators; i++)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
			Rotators.Add(Direction.Rotation());
		}
	}
	else
	{
		Rotators.Add(Forward.Rotation());
	}

	return Rotators;
}

TArray<FVector> UAuraAbilitySystemLibrary::EvenlyRotatedVectors(const FVector& Forward, const FVector& Axis, float Spread, int32 NumberOfVectors)
{
	TArray<FVector> Vectors;
	
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);
	if (NumberOfVectors > 1)
	{
		const float DeltaSpread = Spread / (NumberOfVectors - 1);
		for (int32 i = 0; i < NumberOfVectors; i++)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
			Vectors.Add(Direction);
		}
	}
	else
	{
		Vectors.Add(Forward);
	}

	return Vectors;
}

int32 UAuraAbilitySystemLibrary::GetXPRewardForClassAndLevel(const UObject* WorldContextObject, ECharacterClass CharacterClass, int32 CharacterLevel)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr) return 0;

	const FCharacterClassDefaultInfo Info = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	const float XPReward = Info.XPReward.GetValueAtLevel(CharacterLevel);

	return static_cast<int32>(XPReward);
}

// Damage Effect Parameters
void UAuraAbilitySystemLibrary::SetIsRadialDamageEffectParam(FDamageEffectParameters& DamageEffectParameters, bool bIsRadial, float InnerRadius, float OuterRadius, FVector Origin)
{
	DamageEffectParameters.bIsRadialDamage = bIsRadial;
	DamageEffectParameters.RadialDamageInnerRadius = InnerRadius;
	DamageEffectParameters.RadialDamageOuterRadius = OuterRadius;
	DamageEffectParameters.RadialDamageOrigin = Origin;
}

void UAuraAbilitySystemLibrary::SetKnockbackDirection(FDamageEffectParameters& DamageEffectParameters, FVector KnockbackDirection, float Magnitude)
{
	KnockbackDirection.Normalize();
	if (Magnitude == 0.f)
	{
		DamageEffectParameters.KnockbackForce = KnockbackDirection * DamageEffectParameters.KnockbackForceMagnitude;
	}
	else
	{
		DamageEffectParameters.KnockbackForce = KnockbackDirection * Magnitude;
	}
}

void UAuraAbilitySystemLibrary::SetDeathImpulseDirection(FDamageEffectParameters& DamageEffectParameters, FVector DeathImpulseDirection, float Magnitude)
{
	DeathImpulseDirection.Normalize();
	if (Magnitude == 0.f)
	{
		DamageEffectParameters.DeathImpulse = DeathImpulseDirection * DamageEffectParameters.DeathImpulseMagnitude;
	}
	else
	{
		DamageEffectParameters.DeathImpulse = DeathImpulseDirection * Magnitude;
	}
}

void UAuraAbilitySystemLibrary::SetTargetEffectParametersASC(FDamageEffectParameters& DamageEffectParameters, UAbilitySystemComponent* InASC)
{
	DamageEffectParameters.TargetAbilitySystemComponent = InASC;
}

// Private helpers
FAuraGameplayEffectContext* UAuraAbilitySystemLibrary::GetAuraContext(FGameplayEffectContextHandle& Handle)
{
	return static_cast<FAuraGameplayEffectContext*>(Handle.Get());
}

const FAuraGameplayEffectContext* UAuraAbilitySystemLibrary::GetAuraContext(const FGameplayEffectContextHandle& Handle)
{
	return static_cast<const FAuraGameplayEffectContext*>(Handle.Get());
}
