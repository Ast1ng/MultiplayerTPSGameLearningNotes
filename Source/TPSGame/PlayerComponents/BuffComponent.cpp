﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "TPSGame/Character/PlayerCharacter.h"


UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	
}


// Called every frame
void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

