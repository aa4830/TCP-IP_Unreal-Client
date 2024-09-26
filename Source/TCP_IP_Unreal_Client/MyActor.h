#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyActor.generated.h"

USTRUCT()
struct FMyStruct
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<int32> Temperature;

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

    FSocket* Socket;

    // ParsedData는 MyActor.cpp에서 정의됨
    static FMyStruct ParsedData;  // 헤더 파일에는 static으로 선언
};
