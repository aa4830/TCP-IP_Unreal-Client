#include "MyActor.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Sockets.h"
#include "SocketSubsystem.h"

FMyStruct AMyActor::ParsedData;

// ������: ���� �ʱ�ȭ
AMyActor::AMyActor()
{
    PrimaryActorTick.bCanEverTick = true;
    Socket = nullptr;
}

// ���� ���� �� ������ ���� �õ�
void AMyActor::BeginPlay()
{
    Super::BeginPlay();
    ConnectToServer();
}

// �� �����Ӹ��� �����κ��� ������ ������ Ȯ��
void AMyActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // ������ ����� �������� Ȯ��
    if (Socket && Socket->GetConnectionState() == SCS_Connected)
    {
        uint32 DataSize;
        // ��� ���� �����Ͱ� �ִ��� Ȯ��
        if (Socket->HasPendingData(DataSize))
        {
            // �����͸� �о��
            TArray<uint8> ReceivedData;
            ReceivedData.SetNumUninitialized(DataSize);

            int32 BytesRead = 0;
            Socket->Recv(ReceivedData.GetData(), ReceivedData.Num(), BytesRead);

            // ������ �����͸� ���ڿ��� ��ȯ
            FString ReceivedString = FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(ReceivedData.GetData())));

            // ������ �����͸� JSON���� ó��
            ReceiveJsonData(ReceivedString);
        }
    }
}

// ������ �����ϴ� �Լ�
void AMyActor::ConnectToServer()
{
    FString ServerAddress = TEXT("127.0.0.1"); // ���� IP
    int32 Port = 10890; // ��Ʈ ��ȣ
    ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

    // ���� �ּ� ����
    TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
    bool bIsValid;
    Addr->SetIp(*ServerAddress, bIsValid);
    Addr->SetPort(Port);

    // IP �ּ� ��ȿ�� Ȯ��
    if (!bIsValid)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid IP address"));
        return;
    }

    // ���� ����
    Socket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("DefaultSocket"), false);
    if (Socket == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create socket"));
        return;
    }

    // ������ ���� �õ�
    bool bConnected = Socket->Connect(*Addr);
    if (bConnected)
    {
        UE_LOG(LogTemp, Log, TEXT("Successfully connected to server"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to connect to server"));
    }
}

// ������ JSON �����͸� ó���ϴ� �Լ�
void AMyActor::ReceiveJsonData(const FString& JsonString)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

    // JSON �Ľ� ���� ���� Ȯ��
    if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
    {
        // State ���� ������ ParsedData�� ����
        ParsedData.State = JsonObject->GetStringField(TEXT("State"));

        // Temperature �迭 ��������
        TArray<TSharedPtr<FJsonValue>> TemperatureValues = JsonObject->GetArrayField(TEXT("Temperature"));

        ParsedData.Temperature.Empty();
        for (const TSharedPtr<FJsonValue>& Value : TemperatureValues)
        {
            ParsedData.Temperature.Add(Value->AsNumber());  // ���� ���·� ��ȯ�Ͽ� �߰�
        }

        // �α� ���
        UE_LOG(LogTemp, Log, TEXT("State: %s"), *ParsedData.State);
        for (float Temp : ParsedData.Temperature)  // �µ��� ���� float Ÿ������ ó��
        {
            UE_LOG(LogTemp, Log, TEXT("Temperature: %.1f"), Temp); // �Ҽ��� ���
        }
    }
}
