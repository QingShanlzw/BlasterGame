// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterMapGameMode.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

void ABlasterMapGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter,ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;
	if(AttackerPlayerState && AttackerPlayerState!=VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.0f);
	}
	if(VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	if(ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}
}
//生成玩家。先把死亡的玩家destroy，然后再用playercontroller和一个actor的位置生政一个player
void ABlasterMapGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}
