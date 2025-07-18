﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

/**
 * 
 */
UCLASS()
class TPSGAME_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;

	void SetDisplayText(FString TextToDisplay);//设置文本块文本

	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(APawn* InPawn);//展示玩家NetWork Role类型

protected:
	virtual void NativeDestruct() override;
};
   