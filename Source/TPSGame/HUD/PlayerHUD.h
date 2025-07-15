// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:
	class UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;
	
	FLinearColor CrosshairsColor;
	//准星扩散量
	float CrosshairSpread;

};


UCLASS()
class TPSGAME_API APlayerHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	virtual void DrawHUD() override;

	/*
	*	-------- 玩家覆盖层 --------
	*/

	UPROPERTY(EditAnywhere, Category = "玩家统计信息")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	//将玩家覆盖层添加到视口
	void AddCharacterOverlay();

	// ---------------------------


	/*
	*	------- 热身阶段HUD --------
	*/

	UPROPERTY(EditAnywhere, Category = "热身阶段HUD")
	TSubclassOf<class UUserWidget> AnnouncementClass;

	UPROPERTY()
	class UAnnouncement* Announcement;

	//将热身阶段HUD添加到视口
	void AddAnnouncement();	

	// ---------------------------

protected:
	virtual void BeginPlay() override;
	

private:
	FHUDPackage HUDPackage;	//存储准星HUD

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);	//绘制准星

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }	//设置准星HUD
};
