#include "MyActor.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Sockets.h"
#include "SocketSubsystem.h"

FMyStruct AMyActor::ParsedData;

// 생성자: 소켓 초기화
AMyActor::AMyActor()
{
    PrimaryActorTick.bCanEverTick = true;
    Socket = nullptr;
}

// 게임 시작 시 서버에 연결 시도
void AMyActor::BeginPlay()
{
    Super::BeginPlay();
    ConnectToServer();
}

// 매 프레임마다 서버로부터 데이터 수신을 확인
void AMyActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 소켓이 연결된 상태인지 확인
    if (Socket && Socket->GetConnectionState() == SCS_Connected)
    {
        uint32 DataSize;
        // 대기 중인 데이터가 있는지 확인
        if (Socket->HasPendingData(DataSize))
        {
            // 데이터를 읽어옴
            TArray<uint8> ReceivedData;
            ReceivedData.SetNumUninitialized(DataSize);

            int32 BytesRead = 0;
            Socket->Recv(ReceivedData.GetData(), ReceivedData.Num(), BytesRead);

            // 수신한 데이터를 문자열로 변환
            FString ReceivedString = FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(ReceivedData.GetData())));

            // 수신한 데이터를 JSON으로 처리
            ReceiveJsonData(ReceivedString);
        }
    }
}

// 서버에 연결하는 함수
void AMyActor::ConnectToServer()
{
    FString ServerAddress = TEXT("127.0.0.1"); // 서버 IP
    int32 Port = 10890; // 포트 번호
    ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

    // 서버 주소 설정
    TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
    bool bIsValid;
    Addr->SetIp(*ServerAddress, bIsValid);
    Addr->SetPort(Port);

    // IP 주소 유효성 확인
    if (!bIsValid)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid IP address"));
        return;
    }

    // 소켓 생성
    Socket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("DefaultSocket"), false);
    if (Socket == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create socket"));
        return;
    }

    // 서버에 연결 시도
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

// 수신한 JSON 데이터를 처리하는 함수
void AMyActor::ReceiveJsonData(const FString& JsonString)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

    // JSON 파싱 성공 여부 확인
    if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
    {
        // State 값을 가져와 ParsedData에 저장
        ParsedData.State = JsonObject->GetStringField(TEXT("State"));

        // Temperature 배열 가져오기
        TArray<TSharedPtr<FJsonValue>> TemperatureValues = JsonObject->GetArrayField(TEXT("Temperature"));

        ParsedData.Temperature.Empty();
        for (const TSharedPtr<FJsonValue>& Value : TemperatureValues)
        {
            ParsedData.Temperature.Add(Value->AsNumber());  // 숫자 형태로 변환하여 추가
        }

        // 로그 출력
        UE_LOG(LogTemp, Log, TEXT("State: %s"), *ParsedData.State);
        for (float Temp : ParsedData.Temperature)  // 온도는 보통 float 타입으로 처리
        {
            UE_LOG(LogTemp, Log, TEXT("Temperature: %.1f"), Temp); // 소수점 출력
        }
    }
}
