#include "MyActor.h"
#include "Sockets.h"
#include "SocketSubsystem.h"

AMyActor::AMyActor()
{
    PrimaryActorTick.bCanEverTick = true;
    Socket = nullptr;
}

void AMyActor::BeginPlay()
{
    Super::BeginPlay();
    ConnectToServer();
}

void AMyActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (Socket && Socket->GetConnectionState() == SCS_Connected)
    {
        FString Command;
        if (ReceiveData(Command))
        {
            HandleServerCommand(Command);
        }
    }
}

void AMyActor::ConnectToServer()
{
    FString ServerAddress = TEXT("192.168.0.108");
    int32 Port = 10880;
    ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

    TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
    bool bIsValid;
    Addr->SetIp(*ServerAddress, bIsValid);
    Addr->SetPort(Port);
    if (!bIsValid)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid IP address"));
        return;
    }

    Socket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("DefaultSocket"), false);
    if (Socket == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create socket"));
        return;
    }

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

bool AMyActor::ReceiveData(FString& OutData)
{
    if (!Socket)
    {
        UE_LOG(LogTemp, Error, TEXT("Socket is not connected"));
        return false;
    }

    TArray<uint8> ReceivedData;
    uint32 Size;

    if (Socket->HasPendingData(Size))
    {
        ReceivedData.SetNumUninitialized(Size);

        int32 Read = 0;
        Socket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read);

        OutData = FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(ReceivedData.GetData())));
        UE_LOG(LogTemp, Log, TEXT("Received data: %s"), *OutData);
        return true;
    }

    return false;
}

void AMyActor::HandleServerCommand(const FString& Command)
{
    if (Command == "Rotate")
    {
        FRotator NewRotation = GetActorRotation();
        NewRotation.Yaw += 10.0f;
        SetActorRotation(NewRotation);
    }
}