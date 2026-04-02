// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceGameManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASpaceGameManager::ASpaceGameManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ASpaceGameManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASpaceGameManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASpaceGameManager::InitialiserEtGenererJeu()
{
    Ressources.clear();

    try {
        TxtRessourceReader resReader;
        resReader.load("configs/config_ressources.txt", Ressources);

        TxtWorldReader worldReader;
        worldReader.chargerConfig("configs/config_espace.txt", Ressources, World);

        TuileData limite;
        limite.nom = "Limite"; limite.symbole = '#'; limite.cout = -1;
        limite.constructible = false; limite.gen = { 0, 0 };
        limite.mouv = { false, false, false };
        World.ajouterAuCatalogue('#', limite);

        MonBoard = std::make_unique<board>(BoardSize, World);

        for (int i = 0; i < BoardSize; ++i) {
            for (int j = 0; j < BoardSize; ++j) {
                const hexa* tileData = MonBoard->getCell(i, j);

                // On récupère le NOM du type (ex: "Planete") pour être générique
                FString TypeNom = FString(tileData->getType().c_str());

                FVector SpawnLocation(i * 200.f, j * 200.f, 0.f);

                if (BibliothequeVisuelle.Contains(TypeNom)) {
                    // On fait apparaître la tuile générique
                    AActor* NouvelleTuile = GetWorld()->SpawnActor<AActor>(ClasseTuileGenerique, SpawnLocation, FRotator::ZeroRotator);

                }
            }
        }
    }
    catch (const std::exception& e) {
        UE_LOG(LogTemp, Error, TEXT("Erreur lors de l'initialisation du jeu : %s"), UTF8_TO_TCHAR(e.what()));
    }
}

