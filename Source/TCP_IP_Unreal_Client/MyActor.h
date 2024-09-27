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

    // ParsedData�� MyActor.cpp���� ���ǵ�
    static FMyStruct ParsedData;  // ��� ���Ͽ��� static���� ����

    // Ÿ�̸� �ڵ鷯
    FTimerHandle TimerHandle;

};
