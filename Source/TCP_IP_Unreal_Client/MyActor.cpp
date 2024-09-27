#include "MyActor.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Sockets.h"
#include "SocketSubsystem.h"

// 정적 변수 초기화
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
    ReceiveData();  // 매 프레임마다 데이터 수신 확인
}

// 서버에 연결하는 함수
void AMyActor::ConnectToServer()
{
    FString ServerAddress = TEXT("127.0.0.1"); // 서버 IP
    int32 Port = 10990; // 포트 번호
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
    // JSON 파싱
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

    // JSON 파싱 성공 여부 확인
    if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
    {
        // "MyStruct" 객체 가져오기
        TSharedPtr<FJsonObject> MyStruct = JsonObject->GetObjectField(TEXT("MyStruct"));
        if (MyStruct.IsValid())  // MyStruct 유효성 확인
        {
            // State 값을 가져와 ParsedData에 저장
            ParsedData.State = MyStruct->GetStringField(TEXT("State"));
            // Temperature 단일 값 가져오기
            ParsedData.TemperatureValue = MyStruct->GetNumberField(TEXT("Temperature"));

            // State와 Temperature 값만 로그 출력
            LogParsedData();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("MyStruct is invalid or missing"));
        }
    }
    else
    {
        // JSON 파싱 실패 시 더 많은 정보 제공
        UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON: %s"), *JsonString);
    }
}

// 주기적으로 서버로부터 데이터를 수신하는 함수
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
                // 수신된 데이터를 문자열로 변환
                FString ReceivedString = FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(ReceivedData.GetData())));

                // 수신한 문자열 정리
                if (ReceivedString.EndsWith(TEXT("\n"))) {
                    ReceivedString = ReceivedString.Left(ReceivedString.Len() - 1);
                }

                // 추가 정리 (예: 특정 특수문자 제거)
                ReceivedString.RemoveFromEnd(TEXT("\r")); // 캐리지 리턴 제거

                // 수신한 문자열을 JSON으로 처리
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
    // State와 Temperature 값만 로그 출력
    UE_LOG(LogTemp, Log, TEXT("State: %s"), *ParsedData.State);
    UE_LOG(LogTemp, Log, TEXT("Temperature: %.1f"), ParsedData.TemperatureValue);
}
