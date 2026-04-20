#include "Descend/Public/FModInterfaceComponent.h"

#include "FMODBlueprintStatics.h"
#include "FMODEvent.h"

UFModInterfaceComponent::UFModInterfaceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bAllowConcurrentTick = true;
	
	FMODAudioComponent = CreateDefaultSubobject<UFMODAudioComponent>("Fmod Component Interface");
	GetFMODAudioComponent()->SetAutoActivate(false);	
}

void UFModInterfaceComponent::BeginPlay()
{
	Super::BeginPlay();
	
	AActor* Owner = GetOwner();
	if (IsValid(Owner))
	{
		GetFMODAudioComponent()->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		InitializeMap();
		CacheAllParameterNames();
	}
}

bool UFModInterfaceComponent::Validate() const
{
	if (!IsValid(GetFMODAudioComponent()) || !IsValid(GetFMODAudioComponent()->Event) )
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "[ERROR]: " + GetOwner()->GetName());
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "FMODComponentInterface IsValid: %i", IsValid(GetFMODAudioComponent()));
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "FMODComponentInterface->Event IsValid: %i", IsValid(GetFMODAudioComponent()->Event));
		
		return false;
	}
	return true;
}

void UFModInterfaceComponent::InitializeMap()
{
	if (!FMODAudioEventsMap.IsEmpty() && IsValid(GetFMODAudioComponent()))
	{
		TrySetFmodComponentEvent(FMODAudioEventsMap.begin().Value());
	}
}

bool UFModInterfaceComponent::AddNewEvent(TObjectPtr<UFMODEvent> EventToAdd)
{
	if (!IsValid(EventToAdd) || EventToAdd->GetName().IsEmpty())
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "[ERROR] UFModInterfaceComponent::AddNewEvent: Failed to Add New Event " + GetOwner()->GetName());
		return false;
	}
	
	FMODAudioEventsMap.Emplace(EventToAdd->GetName(), EventToAdd);
	return true; 
}

bool UFModInterfaceComponent::TrySetFmodComponentEvent(UFMODEvent* NewEvent) const
{
	if (IsValid(NewEvent))
	{
		GetFMODAudioComponent()->SetEvent(NewEvent);
		return true;
	}
	return false;
}

void UFModInterfaceComponent::CacheAllParameterNames()
{
	for (auto& Event : FMODAudioEventsMap)
	{
		TArray<FMOD_STUDIO_PARAMETER_DESCRIPTION> Descriptions;
		Descriptions.Empty();
		
		Event.Value->GetParameterDescriptions(Descriptions);
		for (const auto& Description : Descriptions)
		{
			if (Description.name == nullptr /*|| Description.name[0] == '\0'*/)
			{
				continue;
			}
			ParameterNames.Add(Description.name);
		}
	}
}

void UFModInterfaceComponent::PlayEvent()
{
	if (!Validate())
	{
		return;
	}
	GetFMODAudioComponent()->Play();
}

void UFModInterfaceComponent::StopEvent()
{
	if (!Validate())
	{
		return;
	}
	GetFMODAudioComponent()->Stop();
}

void UFModInterfaceComponent::SetParameter(const FName& ParameterName, const float ParameterValue)
{
	if (!ParameterName.IsValid() || ParameterName.IsNone())
	{
		return;
	}
	if (!ParameterNames.Contains(ParameterName))
	{
		return;
	}
	
	if (!Validate())
	{
		return;
	}
	
	GetFMODAudioComponent()->SetParameter(ParameterName, ParameterValue);
	
}

bool UFModInterfaceComponent::SetCurrentEvent(const FString& EventToChange)
{
	if (!FMODAudioEventsMap.Contains(EventToChange))
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "[ERROR]: UFMODInterfaceComponent::SetCurrentEvent");
		return false;
	}
	
	TObjectPtr<UFMODEvent> CurrentEvent = GetFModEvent();
	if (CurrentEvent.GetName().Equals(EventToChange))
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "[Yellow]: Trying to set to an event already set");
		return false;
	}
	
	UFMODEvent* NewCurrentEvent = *FMODAudioEventsMap.Find(EventToChange);
	return TrySetFmodComponentEvent(NewCurrentEvent);
	
}

#if WITH_EDITOR
void UFModInterfaceComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	const FProperty* Property = PropertyChangedEvent.Property;
	const FProperty* MemberProperty = PropertyChangedEvent.MemberProperty;
	
	if (!Property || !MemberProperty)
	{
		return;
	}
	
	if (MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UFModInterfaceComponent, FMODAudioEventsMap))
	{
		FixEventContainerMapKeys();
	}
}

void UFModInterfaceComponent::FixEventContainerMapKeys()
{
	TArray<FString> EventNamesToChange; 
	EventNamesToChange.Empty();
	
	for (auto& Event : FMODAudioEventsMap)
	{
		if (Event.Key.IsEmpty())
		{
			continue;
		}
		
		if (!IsValid(Event.Value))
		{
			continue;
		}
		
		if (Event.Key != Event.Value->GetName())
		{
			EventNamesToChange.Add(Event.Key);
		}
	}
	/*
	 * Event: TPair<"Evetno1", Evento1>
	 */
	for (auto& Name : EventNamesToChange)
	{
		TObjectPtr<UFMODEvent>* FmodEventToChange = FMODAudioEventsMap.Find(Name);
		if (!FmodEventToChange)
		{
			continue;
		}
		if (!IsValid(*FmodEventToChange))
		{
			continue;
		}
		UFMODEvent* NewEvent = FmodEventToChange->Get();
		FString EventNameToChange = NewEvent->GetName();
		
		FMODAudioEventsMap.Remove(Name);
		FMODAudioEventsMap.Emplace(EventNameToChange, NewEvent);
	}
}
#endif