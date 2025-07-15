// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionSubsystem.generated.h"


//
//声明用于 Menu类绑定回调 的 自定义委托
//
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessful);
/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSION_API UMultiplayerSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UMultiplayerSessionSubsystem();

	//
	//	处理会话功能，供菜单类调用
	//

	void CreateSession(int32 NumPublicConnections, FString MatchType);
	void FindSession(int32 MaxSearchResults);
	void JoinSession(const FOnlineSessionSearchResult& SearchResult);
	void DestroySession();
	void StartSession();

	//
	//用于菜单类绑定的自定义委托的回调函数
	//

	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete; //创建会话完成
	FMultiplayerOnFindSessionComplete MultiplayerOnFindSessionsComplete;	//查找会话完成
	FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;		//加入会话完成
	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;//销毁会话完成
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;	//开始会话完成

protected:
	//
	//用于委托的内部回调函数，将加入到在线回话接口委托列表
	//这些不需要在类外被调用
	//

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);	//创建会话完成的回调函数
	void OnFindSessionsComplete(bool bWasSuccessful);						//查找会话完成的回调函数
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);	//加入会话完成的回调函数
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);	//销毁会话完成的回调函数
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);	//开始会话完成的回调函数

private:
	IOnlineSessionPtr SessionInterface;						//在线会话接口指针
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;	//上次创建的会话设置
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;		//上次查找的会话搜索结果

	//
	// 用于加入到在线会话接口列表
	//在OnlineSessionSubsystem内部绑定这些回调函数
	//
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;

	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;

	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;

	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;

	bool bCreateSessionOnDestroy{ false };
	int32 LastNumPublicConnections;
	FString LastMatchType;
};
