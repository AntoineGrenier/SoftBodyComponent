// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPP_SoftBodyComponent.generated.h"

USTRUCT()
struct FParticle
{
	GENERATED_BODY()

	FVector Position;
	FVector PreviousPosition;
	float InvMass;
	float Damping;

	FParticle()
		: Position(FVector::ZeroVector)
		, PreviousPosition(FVector::ZeroVector)
		, InvMass(1.f)
		, Damping(0.01f)
	{
	}
};

USTRUCT()
struct FSoftBodyConstraint
{
	GENERATED_BODY()

	int32 A, B;
	float RestLength;
	float Stiffness;

	FSoftBodyConstraint() : A(0), B(0), RestLength(0.f), Stiffness(1.f) {}
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SOFTBODYCOMPONENT_API UCPP_SoftBodyComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCPP_SoftBodyComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Configurables
	
	//-NumParticles
	//	• Détermine le nombre de particules qui composent ta corde.
	//	• Plus tu mets de particules, plus la corde sera fine et “fluide”, mais plus le solveur sera coûteux.
	//	• SegmentLength = TotalLength / (NumParticles–1)fixe la distance au repos entre chaque particule.
	UPROPERTY(EditAnywhere, Category = "SoftBody|Rope")
	int32 NumParticles = 20;

	//-TotalLength
	//	• Longueur totale de la corde, en unités Unreal(cm).
	//	• Change directement la distance de repos(SegmentLength).
	//	• Utile pour adapter la corde à ton level sans toucher au code.
	UPROPERTY(EditAnywhere, Category = "SoftBody|Rope")
	float TotalLength = 400.f;

	//-Stiffness
	//	• Coefficient de raideur des contraintes(valeurs[0; 1]).
	//	• À 0 tu supprimes toute correction  : la corde devient molle comme un spaghetti.
	//	• À 1 tu appliques la correction totale pour que chaque segment conserve exactement sa longueur de repos.
	//	• Entre deux, tu compromises entre réalisme “élastique” et stabilité numérique.
	UPROPERTY(EditAnywhere, Category = "SoftBody|Simulation", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float Stiffness = 1.f;

	//-Damping
	//	• Amortissement visqueux appliqué aux vitesses(valeurs[0; 1]).
	//	• 0 = pas d’amortissement(la corde vibre longtemps),
	//	• 1 = amortissement maximal(le mouvement disparaît instantanément).
	//	• En pratique on choisit souvent 0.01–0.1 pour supprimer les hautes fréquences sans tuer la dynamique.
	UPROPERTY(EditAnywhere, Category = "SoftBody|Simulation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Damping = 0.01f;

	//-SolverIterations
	//	• Nombre de passes de PBD(Position Based Dynamics) par frame.
	//	• Chaque itération raffine la satisfaction des contraintes.
	//	• 1–2 itérations → corde très élastique, segments flottent.
	//	• 8–12 itérations → comportement quasi - rigide, mais CPU - bound.
	//	• Ajuste en fonction de ton budget perf et du niveau de fidélité voulu
	UPROPERTY(EditAnywhere, Category = "SoftBody|Simulation")
	int32 SolverIterations = 8;

	//-Gravity
	//	• Vecteur d’accélération global appliqué aux particules non fixées.
	//	• Par défaut(0, 0, –980) cm / s² simule la gravité terrestre.
	//	• Tu peux le modifier pour créer des effets de vent(X / Y), ou un espace faible en pesanteur.
	UPROPERTY(EditAnywhere, Category = "SoftBody|Simulation")
	FVector Gravity = FVector(0.f, 0.f, -980.f);

private:
	TArray<FParticle> Particles;
	TArray<FSoftBodyConstraint> Constraints;
	bool bInitialized = false;

	void InitializeRope();
	void SatisfyConstraints();
};
