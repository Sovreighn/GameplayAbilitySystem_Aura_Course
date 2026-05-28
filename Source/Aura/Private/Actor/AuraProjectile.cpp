// Copyright Sovreighn Gaming

#include "Actor/AuraProjectile.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Aura/Aura.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AAuraProjectile::AAuraProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	SetRootComponent(Sphere);
	Sphere->SetCollisionObjectType(ECC_Projectile);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->InitialSpeed = 550.f;
	ProjectileMovement->MaxSpeed = 550.0f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

void AAuraProjectile::BeginPlay()
{
	Super::BeginPlay();

	// Prevent Collisions between this projectile and its Instigator
	Sphere->IgnoreActorWhenMoving(GetInstigator(), true);

	if (ACharacter* InstigatorCharacter = Cast<ACharacter>(GetInstigator()))
	{
		// Collision Detection was on Capsule
		InstigatorCharacter->GetCapsuleComponent()->IgnoreActorWhenMoving(this, true);
		// Collision Detection was on Mesh
		InstigatorCharacter->GetMesh()->IgnoreActorWhenMoving(this, true);
	}
	
	SetReplicateMovement(true);
	SetLifeSpan(Lifespan);
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AAuraProjectile::OnSphereOverlap);
	LoopingSoundComponent = UGameplayStatics::SpawnSoundAttached(LoopingSound, GetRootComponent());
}

bool AAuraProjectile::IsValidOverlap(AActor* OtherActor)
{
	if (DamageEffectParameters.SourceAbilitySystemComponent == nullptr) { return false; }
	AActor* SourceAvatarActor = DamageEffectParameters.SourceAbilitySystemComponent->GetAvatarActor();
	if (SourceAvatarActor == OtherActor) { return false; }
	if (OtherActor == GetInstigator()) { return false; }
	if (!UAuraAbilitySystemLibrary::IsNotFriend(SourceAvatarActor, OtherActor)) { return false; }
	return true;
}

void AAuraProjectile::OnHit()
{
	UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());

	if (LoopingSoundComponent)
	{
		LoopingSoundComponent->Stop();
		LoopingSoundComponent->DestroyComponent();
	}
	
	bHit = true;
}

void AAuraProjectile::Destroyed()
{
	if (LoopingSoundComponent)
	{
		LoopingSoundComponent->Stop();
		LoopingSoundComponent->DestroyComponent();
	}
	
	if (!bHit && !HasAuthority()) { OnHit(); }
	Super::Destroyed();
}

void AAuraProjectile::OnSphereOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!IsValidOverlap(OtherActor)) { return; }
	if (!bHit) { OnHit(); }
	
	if (HasAuthority())
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			const FVector DeathImpulse = GetActorForwardVector() * DamageEffectParameters.DeathImpulseMagnitude;
			DamageEffectParameters.DeathImpulse = DeathImpulse;

			const bool bKnockback = FMath::RandRange(1,100) < DamageEffectParameters.KnockbackChance;
			if (bKnockback)
			{
				FRotator Rotation = GetActorRotation();
				Rotation.Roll = 15.f;
				const FVector KnockbackDirection = Rotation.Vector();
				const FVector KnockbackForce = KnockbackDirection * DamageEffectParameters.KnockbackForceMagnitude;
				DamageEffectParameters.KnockbackForce = KnockbackForce;
			}
			
			DamageEffectParameters.TargetAbilitySystemComponent = TargetASC;
			UAuraAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParameters);
		}
		
		Destroy();
	}
	
	else { bHit = true; }
}



