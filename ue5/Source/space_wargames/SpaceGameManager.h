// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"

#include "jeu.hh"


#include "SpaceGameManager.generated.h"

UCLASS(BlueprintType, Blueprintable)
class SPACE_WARGAMES_API ASpaceGameManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpaceGameManager();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WarGame|Config")
	int32 BoardSize = 10;

	// Références aux Blueprints de tes tuiles (Espace, Planète, etc.)
	UPROPERTY(EditAnywhere, Category="WarGame|Visuals")
	TSubclassOf<AActor> ClasseTuileGenerique;
	UPROPERTY(EditAnywhere, Category = "WarGame|Visuals")
	TMap<FString, UStaticMesh*> BibliothequeVisuelle;

	UFUNCTION(BlueprintCallable, Category="WarGame|Actions")
	void InitialiserEtGenererJeu();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	std::unique_ptr<board> MonBoard;
	WorldFactory World;
	std::map<std::string, Ressource*> Ressources;

};
