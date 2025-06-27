// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_SoftBodyComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

// Sets default values for this component's properties
UCPP_SoftBodyComponent::UCPP_SoftBodyComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UCPP_SoftBodyComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeRope();
}

void UCPP_SoftBodyComponent::InitializeRope()
{
    Particles.Empty();
    Constraints.Empty();
    if (NumParticles < 2) return;

    // Création des particules
    float Segment = TotalLength / (NumParticles - 1);
    FVector Start = GetOwner()
        ? GetOwner()->GetActorLocation()
        : FVector::ZeroVector;

    for (int32 i = 0; i < NumParticles; ++i)
    {
        FParticle P;
        P.Position = Start + FVector(i * Segment, 0, 0);
        P.PreviousPosition = P.Position;
        P.InvMass = 1.f;
        P.Damping = Damping;
        Particles.Add(P);
    }

    // Ancrer les extrémités
    Particles[0].InvMass = 0.f;
    Particles.Last().InvMass = 0.f;

    // Création des contraintes distance
    for (int32 i = 0; i < NumParticles - 1; ++i)
    {
        FSoftBodyConstraint C;
        C.A = i; C.B = i + 1;
        C.RestLength = Segment;
        C.Stiffness = Stiffness;
        Constraints.Add(C);
    }

    bInitialized = true;
}



// Called every frame
void UCPP_SoftBodyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!bInitialized || Particles.Num() == 0) return;

    // 1) Intégration Verlet
    for (auto& P : Particles)
    {
        if (P.InvMass <= 0) continue; // points fixes

        FVector Temp = P.Position;
        FVector Velocity = (P.Position - P.PreviousPosition) * (1.f - P.Damping);
        P.Position += Velocity + Gravity * DeltaTime * DeltaTime;
        P.PreviousPosition = Temp;
    }

    // 2) Satisfaction des contraintes
    for (int32 it = 0; it < SolverIterations; ++it)
        SatisfyConstraints();

    // 3) Debug draw
    for (int32 i = 0; i < Constraints.Num(); ++i)
    {
        const auto& C = Constraints[i];
        const auto& A = Particles[C.A].Position;
        const auto& B = Particles[C.B].Position;
        DrawDebugLine(GetWorld(), A, B, FColor::Green, false, -1.f, 0, 2.f);
        DrawDebugPoint(GetWorld(), A, 5.f, FColor::Red, false);
    }
    // draw last point
    DrawDebugPoint(GetWorld(), Particles.Last().Position, 5.f, FColor::Red, false);
}

void UCPP_SoftBodyComponent::SatisfyConstraints()
{
    for (auto& C : Constraints)
    {
        FParticle& PA = Particles[C.A];
        FParticle& PB = Particles[C.B];
        FVector Delta = PB.Position - PA.Position;
        float CurrentLen = Delta.Size();
        if (CurrentLen <= KINDA_SMALL_NUMBER) continue;

        float Error = (CurrentLen - C.RestLength) / CurrentLen;
        FVector Correction = Delta * 0.5f * C.Stiffness * Error;

        if (PA.InvMass > 0) PA.Position += Correction * PA.InvMass;
        if (PB.InvMass > 0) PB.Position -= Correction * PB.InvMass;
    }
}

