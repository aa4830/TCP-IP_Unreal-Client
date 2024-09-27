#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyActor.generated.h"

USTRUCT()
struct FMyStruct
{
    GENERATED_BODY()

    UPROPERTY()
    double TemperatureValue;

    UPROPERTY()
    FString State;
};

UCLASS()
class TCP_IP_UNREAL_CLIENT_API AMyActor : public AActor
{
    GENERATED_BODY()

public:
    AMyActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

private:
    void ConnectToServer();
    void ReceiveJsonData(const FString& JsonString);
    void ReceiveData();
    void LogParsedData();

    FSocket* Socket;

    // ParsedData는 MyActor.cpp에서 정의됨
    static FMyStruct ParsedData;  // 헤더 파일에는 static으로 선언

    // 타이머 핸들러
    FTimerHandle TimerHandle;

};
