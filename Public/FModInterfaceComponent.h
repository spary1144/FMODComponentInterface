// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FMODAudioComponent.h"
#include "FMODSnapshot.h"
#include "Components/ActorComponent.h"
#include "FModInterfaceComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DESCEND_API UFModInterfaceComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category ="Sound", meta = (AllowPrivateAccess = "true"))
	
	UPROPERTY()
	UFMODAudioComponent* FMODAudioComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category ="Sound", meta = (AllowPrivateAccess = "true"))
	TMap<FString, TObjectPtr<UFMODEvent>> FMODAudioEventsMap;
	
	TArray<FString> ParameterNames;
	
	bool Validate() const;
	
	FORCEINLINE void SetFmodComponentEvent(UFMODEvent* NewEvent) const { GetFMODAudioComponent()->Event = NewEvent; }
	
	void InitializeMap();
	
	bool TrySetFmodComponentEvent(UFMODEvent* NewEvent) const;
	
	void CacheAllParameterNames();
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent ) override;
	void FixEventContainerMapKeys();
#endif
	
public:
	
	UFModInterfaceComponent();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Sound")
	void PlayEvent();
	
	UFUNCTION(BlueprintCallable, Category = "Sound")
	void StopEvent();
	
	UFUNCTION(BlueprintCallable, Category = "Sound")
	void SetParameter(const FName& ParameterName, const float ParameterValue);
	
	UFUNCTION(BlueprintCallable, Category = "Sound")
	bool SetCurrentEvent(const FString& EventToChange);
	
	FORCEINLINE UFMODAudioComponent* GetFMODAudioComponent() const { return FMODAudioComponent; }
	
	FORCEINLINE TObjectPtr<UFMODEvent> GetFModEvent() const { return GetFMODAudioComponent()->Event;}
	
	bool AddNewEvent(TObjectPtr<UFMODEvent> EventToAdd);

protected:
	virtual void BeginPlay() override;
		
};
