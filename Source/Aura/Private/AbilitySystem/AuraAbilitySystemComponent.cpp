// Copyright Sovreighn Gaming

#include "AbilitySystem/AuraAbilitySystemComponent.h"

#include <filesystem>

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Aura/AuraLogChannels.h"
#include "Interaction/PlayerInterface.h"

void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::ClientEffectApplied);
}

void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(AuraAbility->StartupInputTag);
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(FAuraGameplayTags::Get().Abilities_Status_Equipped);
			GiveAbility(AbilitySpec);
		}
	}
	
	bStartupAbilitiesGiven = true;
	AbilitiesGivenDelegate.Broadcast();
}

void UAuraAbilitySystemComponent::AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupPassiveAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	FScopedAbilityListLock AbilityScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			
			if (!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) { return; }

	FScopedAbilityListLock AbilityScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag) && AbilitySpec.IsActive())
		{
			// First mark the input as released on the spec (important!)
			AbilitySpecInputReleased(AbilitySpec);

			// Only invoke the replicated event if the ability is still active
			// It might have ended immediately after releasing input
			if (AbilitySpec.IsActive() && !AbilitySpec.GetAbilityInstances().IsEmpty())
			{
				const FGameplayAbilityActivationInfo& ActivationInfo = AbilitySpec.GetAbilityInstances().Last()->GetCurrentActivationInfoRef();
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle,ActivationInfo.GetActivationPredictionKey());
			}
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	FScopedAbilityListLock AbilityScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			
			if (AbilitySpec.IsActive())
			{
				const FGameplayAbilityActivationInfo& ActivationInfo = AbilitySpec.GetAbilityInstances().Last()->GetCurrentActivationInfoRef();
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilitySpec.Handle,ActivationInfo.GetActivationPredictionKey());
			}
		}
	}
}

void UAuraAbilitySystemComponent::ForEachAbility(const FForEachAbility& Delegate)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!Delegate.ExecuteIfBound(AbilitySpec))
		{
			UE_LOG(LogAura, Error, TEXT("Failed to execute delegate in %hs"), __FUNCTION__);
		}
	}
}

FGameplayTag UAuraAbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.Ability)
	{
		for (FGameplayTag Tag : AbilitySpec.Ability->GetAssetTags())
		{
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities"))))
			{
				return Tag;
			}
		}
	}
	
	return FGameplayTag();
	
}

FGameplayTag UAuraAbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag Tag : AbilitySpec.GetDynamicSpecSourceTags())
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("InputTag"))))
		{
			return Tag;
		}
	}
	
	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag StatusTag : AbilitySpec.GetDynamicSpecSourceTags())
	{
		if (StatusTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status"))))
		{
			return StatusTag;
		}
	}

	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetStatusFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetStatusFromSpec(*Spec);
	}

	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetSlotFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetInputTagFromSpec(*Spec);
	}

	return FGameplayTag();
}

FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetSpecFromAbilityTag(const FGameplayTag& AbilityTag)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		for (FGameplayTag Tag : AbilitySpec.Ability->GetAssetTags())
		{
			if (Tag.MatchesTag(AbilityTag))
			{
				return &AbilitySpec;
			}
		}
	}

	return nullptr;
}

FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetSpecWithSlot(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(Slot))
		{
			return &AbilitySpec;
		}
	}

	return nullptr;
}

void UAuraAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		if (IPlayerInterface::Execute_GetAttributePoints(GetAvatarActor()) > 0)
		{
			ServerUpgradeAttribute(AttributeTag);
		}
	}
}

void UAuraAbilitySystemComponent::UpdateAbilityStatuses(int32 Level)
{
	UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	for (const FAuraAbilityInfo& Info : AbilityInfo->AbilityInformation)
	{
		if (!Info.AbilityTag.IsValid()) continue;
		if (Level < Info.LevelRequirement) continue;
		if (GetSpecFromAbilityTag(Info.AbilityTag) == nullptr)
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Info.Ability, 1);
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(FAuraGameplayTags::Get().Abilities_Status_Eligible);
			GiveAbility(AbilitySpec);
			MarkAbilitySpecDirty(AbilitySpec);
			ClientUpdateAbilityStatuses(Info.AbilityTag, FAuraGameplayTags::Get().Abilities_Status_Eligible, 1);
		}
	}
}

bool UAuraAbilitySystemComponent::GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription, FString& OutNextLevelDescription)
{
	if (const FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		if (UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec->Ability))
		{
			OutDescription = AuraAbility->GetDescription(AbilitySpec->Level);
			OutNextLevelDescription = AuraAbility->GetNextLevelDescription(AbilitySpec->Level + 1);
			return true;
		}
	}
	
	const UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());

	if (!AbilityTag.IsValid() || AbilityTag.MatchesTagExact(FAuraGameplayTags::Get().Abilities_None))
	{
		OutDescription = FString();
	}
	else
	{
		OutDescription = UAuraGameplayAbility::GetLockedDescription(AbilityInfo->FindAbilityInfoForTag(AbilityTag).LevelRequirement);
	}
	
	OutNextLevelDescription = FString();
	return false;
}

bool UAuraAbilitySystemComponent::SlotIsEmpty(const FGameplayTag& Slot)
{
	FScopedAbilityListLock AbilityScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilityHasSlot(AbilitySpec, Slot))
		{
			return false; // The Slot is populated
		}
	}

	return true; // Yes the Slot is empty
}

bool UAuraAbilitySystemComponent::AbilityHasSlot(const FGameplayAbilitySpec& Spec, const FGameplayTag& Slot)
{
	return Spec.GetDynamicSpecSourceTags().HasTagExact(Slot);
}

bool UAuraAbilitySystemComponent::AbilityHasAnySlot(const FGameplayAbilitySpec& Spec)
{
	return Spec.GetDynamicSpecSourceTags().HasTag(FGameplayTag::RequestGameplayTag(FName("InputTag")));
}

bool UAuraAbilitySystemComponent::IsPassiveAbility(const FGameplayAbilitySpec& Spec) const
{
	const UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	const FGameplayTag AbilityTag = GetAbilityTagFromSpec(Spec);
	const FAuraAbilityInfo& Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	const FGameplayTag AbilityType = Info.AbilityType;
	return AbilityType.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Type_Passive);
}

void UAuraAbilitySystemComponent::ClearSlot(FGameplayAbilitySpec* Spec)
{
	const FGameplayTag Slot = GetInputTagFromSpec(*Spec);
	Spec->GetDynamicSpecSourceTags().RemoveTag(Slot);
}

void UAuraAbilitySystemComponent::ClearAbilitiesOfSlot(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (AbilityHasSlot(Spec, Slot))
		{
			ClearSlot(&Spec);
		}
	}
}

void UAuraAbilitySystemComponent::AssignSlotToAbility(FGameplayAbilitySpec& Spec, const FGameplayTag& Slot)
{
	ClearSlot(&Spec);
	Spec.GetDynamicSpecSourceTags().AddTag(Slot);
}

void UAuraAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	if (!bStartupAbilitiesGiven)
	{
		bStartupAbilitiesGiven = true;
		AbilitiesGivenDelegate.Broadcast();
	}
}

// Multicast RPC's
void UAuraAbilitySystemComponent::MulticastActivatePassiveEffect_Implementation(const FGameplayTag& AbilityTag, bool bActivate)
{
	ActivatePassiveEffectDelegate.Broadcast(AbilityTag, bActivate);
}

// Server RPC's
void UAuraAbilitySystemComponent::ServerEquipAbility_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& Slot)
{
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
		const FGameplayTag& PreviousSlot = GetInputTagFromSpec(*AbilitySpec);
		const FGameplayTag& Status = GetStatusFromSpec(*AbilitySpec);
		const bool bStatusValid = Status == GameplayTags.Abilities_Status_Equipped || Status == GameplayTags.Abilities_Status_Unlocked;

		if (bStatusValid)
		{
			if (!SlotIsEmpty(Slot)) // There is an Ability in this Slot already. Deactivate and Clear its Slot.
			{
				FGameplayAbilitySpec* SpecWithSlot = GetSpecWithSlot(Slot);
				if (SpecWithSlot)
				{
					// Is that Ability the as this Ability? If so, we can return early.
					if (AbilityTag.MatchesTagExact(GetAbilityTagFromSpec(*SpecWithSlot)))
					{
						ClientEquipAbility(AbilityTag, GameplayTags.Abilities_Status_Equipped, Slot, PreviousSlot);
						return;
					}
					
					if (IsPassiveAbility(*SpecWithSlot))
					{
						MulticastActivatePassiveEffect(GetAbilityTagFromSpec(*SpecWithSlot), false);
						DeactivatePassiveAbilityDelegate.Broadcast(GetAbilityTagFromSpec(*SpecWithSlot));
					}

					ClearSlot(SpecWithSlot);
				}
			}

			if (!AbilityHasAnySlot(*AbilitySpec)) // Ability doesn't have a Slot, it is not Active.
			{
				if (IsPassiveAbility(*AbilitySpec))
				{
					TryActivateAbility(AbilitySpec->Handle);
					MulticastActivatePassiveEffect(AbilityTag, true);
				}
			}

			AssignSlotToAbility(*AbilitySpec, Slot);
			MarkAbilitySpecDirty(*AbilitySpec);
		}

		ClientEquipAbility(AbilityTag, GameplayTags.Abilities_Status_Equipped, Slot, PreviousSlot);
	}
}

void UAuraAbilitySystemComponent::ServerUpgradeAttribute_Implementation(const FGameplayTag& AttributeTag)
{
	FGameplayEventData Payload;
	Payload.EventTag = AttributeTag;
	Payload.EventMagnitude = 1.f;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);

	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		IPlayerInterface::Execute_AddToAttributePoints(GetAvatarActor(), -1);
	}
}

void UAuraAbilitySystemComponent::ServerSpendSpellPoint_Implementation(const FGameplayTag& AbilityTag)
{
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		if (GetAvatarActor()->Implements<UPlayerInterface>())
		{
			// Spend a Spell Point
			IPlayerInterface::Execute_AddToSpellPoints(GetAvatarActor(), -1);
		}
		
		const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
		FGameplayTag Status = GetStatusFromSpec(*AbilitySpec);

		if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
		{
			AbilitySpec->GetDynamicSpecSourceTags().RemoveTag(GameplayTags.Abilities_Status_Eligible);
			AbilitySpec->GetDynamicSpecSourceTags().AddTag(GameplayTags.Abilities_Status_Unlocked);
			Status = GameplayTags.Abilities_Status_Unlocked;
		}
		else if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Equipped) || Status.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
		{
			AbilitySpec->Level += 1;
		}
		
		ClientUpdateAbilityStatuses(AbilityTag, Status, AbilitySpec->Level);
		MarkAbilitySpecDirty(*AbilitySpec);
	}
}

// Client RPC's
void UAuraAbilitySystemComponent::ClientUpdateAbilityStatuses_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel)
{
	AbilityStatusChangedDelegate.Broadcast(AbilityTag, StatusTag, AbilityLevel);
}

void UAuraAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle) const
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	EffectAssetTags.Broadcast(TagContainer);
}

void UAuraAbilitySystemComponent::ClientEquipAbility_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot)
{
	AbilityEquippedDelegate.Broadcast(AbilityTag, Status, Slot, PreviousSlot);
}
