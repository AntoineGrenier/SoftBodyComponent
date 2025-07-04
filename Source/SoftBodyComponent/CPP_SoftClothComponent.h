// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Materials/MaterialInterface.h"
#include "CPP_SoftClothComponent.generated.h"

USTRUCT()
struct FClothParticle
{
	GENERATED_BODY()

	FVector Position;
	FVector PreviousPosition;
	float InvMass;
	float Damping;

	FClothParticle()
		: Position(FVector::ZeroVector)
		, PreviousPosition(FVector::ZeroVector)
		, InvMass(1.f)
		, Damping(0.01f)
	{
	}
};

USTRUCT()
struct FClothConstraint
{
	GENERATED_BODY()

	int32 A, B;
	float RestLength;
	float Stiffness;

	FClothConstraint()
		: A(0), B(0), RestLength(0.f), Stiffness(1.f)
	{
	}
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOFTBODYCOMPONENT_API UCPP_SoftClothComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCPP_SoftClothComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// grille
	UPROPERTY(EditAnywhere, Category = "Cloth|Grid")
	int32 NumX = 20;

	UPROPERTY(EditAnywhere, Category = "Cloth|Grid")
	int32 NumY = 20;

	UPROPERTY(EditAnywhere, Category = "Cloth|Grid")
	float Spacing = 50.f;

	// PBD
	UPROPERTY(EditAnywhere, Category = "Cloth|Simulation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StiffnessStructural = 1.f;

	UPROPERTY(EditAnywhere, Category = "Cloth|Simulation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StiffnessShear = 1.f;

	UPROPERTY(EditAnywhere, Category = "Cloth|Simulation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StiffnessBend = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Cloth|Simulation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Damping = 0.01f;

	UPROPERTY(EditAnywhere, Category = "Cloth|Simulation")
	int32 SolverIterations = 8;

	UPROPERTY(EditAnywhere, Category = "Cloth|Simulation")
	FVector Gravity = FVector(0.f, 0.f, -980.f);

	UPROPERTY(EditAnywhere, Category = "Cloth|Rendering")
	UMaterialInterface* ClothMaterial = nullptr;

	// Blueprint callable pour réinitialiser
	UFUNCTION(BlueprintCallable, Category = "Cloth")
	void ResetCloth();

private:
	bool bInitialized = false;
	TArray<FClothParticle> Particles;
	TArray<FClothConstraint> Constraints;

	void InitializeCloth();
	void SatisfyConstraints();

};
