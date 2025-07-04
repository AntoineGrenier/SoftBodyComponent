// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_SoftClothComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"

// Sets default values for this component's properties
UCPP_SoftClothComponent::UCPP_SoftClothComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	InitializeCloth();
}


// Called when the game starts
void UCPP_SoftClothComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCPP_SoftClothComponent::ResetCloth()
{
	InitializeCloth();
}



// Called every frame
void UCPP_SoftClothComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!bInitialized) return;

    // 1) Verlet integration
    for (auto& P : Particles)
    {
        if (P.InvMass <= 0) continue;
        FVector Temp = P.Position;
        FVector Vel = (P.Position - P.PreviousPosition) * (1.f - P.Damping);
        P.Position += Vel + Gravity * DeltaTime * DeltaTime;
        P.PreviousPosition = Temp;
    }

    // 2) PBD constraint solve
    for (int i = 0; i < SolverIterations; ++i)
        SatisfyConstraints();

    // 3) Debug draw
    for (auto& C : Constraints)
    {
        const FVector& A = Particles[C.A].Position;
        const FVector& B = Particles[C.B].Position;
        DrawDebugLine(GetWorld(), A, B, FColor::Green, false, -1.f, 0, 1.f);
    }
}

void UCPP_SoftClothComponent::InitializeCloth()
{
    Particles.Empty();
    Constraints.Empty();
    if (NumX < 2 || NumY < 2) return;

    FVector Origin = GetOwner()
        ? GetOwner()->GetActorLocation()
        : FVector::ZeroVector;

    // 1) Création des particules
    for (int y = 0; y < NumY; ++y)
    {
        for (int x = 0; x < NumX; ++x)
        {
            FClothParticle P;
            P.Position = Origin + FVector(x * Spacing, y * Spacing, 0);
            P.PreviousPosition = P.Position;
            P.InvMass = 1.f;
            P.Damping = Damping;
            // ancrage haut du tissu
            if (y == NumY - 1)
                P.InvMass = 0.f;
            Particles.Add(P);
        }
    }

    // 2) Création des contraintes
    auto Idx = [&](int ix, int iy) { return iy * NumX + ix; };

    for (int y = 0; y < NumY; ++y)
    {
        for (int x = 0; x < NumX; ++x)
        {
            int32 i = Idx(x, y);

            // structurel : droite, bas
            if (x + 1 < NumX)
            {
                FClothConstraint C;
                C.A = i; C.B = Idx(x + 1, y);
                C.RestLength = Spacing;
                C.Stiffness = StiffnessStructural;
                Constraints.Add(C);
            }
            if (y + 1 < NumY)
            {
                FClothConstraint C;
                C.A = i; C.B = Idx(x, y + 1);
                C.RestLength = Spacing;
                C.Stiffness = StiffnessStructural;
                Constraints.Add(C);
            }

            // shear : diagonales
            if (x + 1 < NumX && y + 1 < NumY)
            {
                FClothConstraint C1, C2;
                C1.A = i;         C1.B = Idx(x + 1, y + 1);
                C2.A = Idx(x + 1, y); C2.B = Idx(x, y + 1);
                float diag = Spacing * FMath::Sqrt(2.f);
                C1.RestLength = C2.RestLength = diag;
                C1.Stiffness = C2.Stiffness = StiffnessShear;
                Constraints.Add(C1);
                Constraints.Add(C2);
            }

            // bend : deux voisins en droite et bas
            if (x + 2 < NumX)
            {
                FClothConstraint C;
                C.A = i; C.B = Idx(x + 2, y);
                C.RestLength = Spacing * 2.f;
                C.Stiffness = StiffnessBend;
                Constraints.Add(C);
            }
            if (y + 2 < NumY)
            {
                FClothConstraint C;
                C.A = i; C.B = Idx(x, y + 2);
                C.RestLength = Spacing * 2.f;
                C.Stiffness = StiffnessBend;
                Constraints.Add(C);
            }
        }
    }

    bInitialized = true;
}

void UCPP_SoftClothComponent::SatisfyConstraints()
{
    for (auto& C : Constraints)
    {
        FClothParticle& PA = Particles[C.A];
        FClothParticle& PB = Particles[C.B];
        FVector Delta = PB.Position - PA.Position;
        float Len = Delta.Size();
        if (Len < KINDA_SMALL_NUMBER) continue;

        float Error = (Len - C.RestLength) / Len;
        FVector Corr = Delta * 0.5f * C.Stiffness * Error;
        if (PA.InvMass > 0) PA.Position += Corr * PA.InvMass;
        if (PB.InvMass > 0) PB.Position -= Corr * PB.InvMass;
    }
}

