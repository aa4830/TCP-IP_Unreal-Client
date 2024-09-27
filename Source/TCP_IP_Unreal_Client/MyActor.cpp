#include "MyActor.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Sockets.h"
#include "SocketSubsystem.h"

// ���� ���� �ʱ�ȭ
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
    ReceiveData();  // �� �����Ӹ��� ������ ���� Ȯ��
}

// ������ �����ϴ� �Լ�
void AMyActor::ConnectToServer()
{
    FString ServerAddress = TEXT("127.0.0.1"); // ���� IP
    int32 Port = 10990; // ��Ʈ ��ȣ
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
    // JSON �Ľ�
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

    // JSON �Ľ� ���� ���� Ȯ��
    if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
    {
        // "MyStruct" ��ü ��������
        TSharedPtr<FJsonObject> MyStruct = JsonObject->GetObjectField(TEXT("MyStruct"));
        if (MyStruct.IsValid())  // MyStruct ��ȿ�� Ȯ��
        {
            // State ���� ������ ParsedData�� ����
            ParsedData.State = MyStruct->GetStringField(TEXT("State"));
            // Temperature ���� �� ��������
            ParsedData.TemperatureValue = MyStruct->GetNumberField(TEXT("Temperature"));

            // State�� Temperature ���� �α� ���
            LogParsedData();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("MyStruct is invalid or missing"));
        }
    }
    else
    {
        // JSON �Ľ� ���� �� �� ���� ���� ����
        UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON: %s"), *JsonString);
    }
}

// �ֱ������� �����κ��� �����͸� �����ϴ� �Լ�
void AMyActor::ReceiveData()
{
    if (Socket && Socket->GetConnectionState() == SCS_Connected)
    {
        uint32 DataSize;
        if (Socket->HasPendingData(DataSize))
        {
            TArray<uint8> ReceivedData;
            ReceivedData.SetNumUninitialized(DataSize);

            int32 BytesRead = 0;
            if (Socket->Recv(ReceivedData.GetData(), ReceivedData.Num(), BytesRead))
            {
                // ���ŵ� �����͸� ���ڿ��� ��ȯ
                FString ReceivedString = FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(ReceivedData.GetData())));

                // ������ ���ڿ� ����
                if (ReceivedString.EndsWith(TEXT("\n"))) {
                    ReceivedString = ReceivedString.Left(ReceivedString.Len() - 1);
                }

                // �߰� ���� (��: Ư�� Ư������ ����)
                ReceivedString.RemoveFromEnd(TEXT("\r")); // ĳ���� ���� ����

                // ������ ���ڿ��� JSON���� ó��
                ReceiveJsonData(ReceivedString);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to read from socket"));
            }
        }
    }
}

void AMyActor::LogParsedData()
{
    // State�� Temperature ���� �α� ���
    UE_LOG(LogTemp, Log, TEXT("State: %s"), *ParsedData.State);
    UE_LOG(LogTemp, Log, TEXT("Temperature: %.1f"), ParsedData.TemperatureValue);
}
