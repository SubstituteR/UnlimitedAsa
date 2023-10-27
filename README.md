# UnlimitedAsa
Run as many ASA Dedicated Servers as you want on your computer.

# How to Use?
Build and inject the library `crack.dll` into the ASA Dedicated Server at start up.

You must shut down all currently running servers for this library to have any effect.

# Legal
I am not a lawyer, but this should still satisfy the EULA for the dedicated server.

Specifically, 
> Reverse Engineering / manipulating the Application is not allowed (e.g. circumventing the restrictions).

This library does not manipulate the executable code of the Ark Ascended Server. It simply instead modifies third party code that the Server links against. (The Windows System Libraries.)

Additionally, this library does not technically circumvent the restrictions imposed, but rather prevents them from ever occuring.

# Technical

<details>
  <summary>Reading this part could constitute a violation of the EULA.</summary>
  
  Inside the `UShooterEngine::Init` function we can see the following
  ```cpp
  if ( !GIsOfficialServer )
  {
    v6 = UGeneralProjectSettings::StaticClass();
    v7 = v6;
    if ( !v6->ClassDefaultObject )
      UClass::InternalCreateDefaultObjectWrapper(v6);
    p_OuterPrivate = (FGuid *)&v7->ClassDefaultObject[3].OuterPrivate;
    result.Data.AllocatorInstance.Data = 0i64;
    *(_QWORD *)&result.Data.ArrayNum = 0i64;
    FGuid::AppendString(p_OuterPrivate, &result, Digits);
    v9 = FWindowsPlatformProcess::UserTempDir();
    FString::ConcatCF((FString *)&Commandline.Data.ArrayNum, v9, &result);
    if ( result.Data.AllocatorInstance.Data )
      FMemory::Free(result.Data.AllocatorInstance.Data);
    v10 = IFileManager::Get();
    v11 = &InExpression;
    if ( LODWORD(Proc.Handle) )
      v11 = *(const FStructuredArchiveSlot **)&Commandline.Data.ArrayNum;
    if ( !v10->CreateFileWriter(v10, (const wchar_t *)v11, 0) )
    {
      GlobalLogSingleton = GetGlobalLogSingleton();
      FOutputDevice::Log(
        GlobalLogSingleton,
        L"ArkAscendedServer.exe already running. You can only run one dedicated server at a time. Shut down the running se"
         "rver if you wish to run a different one.");
      FWindowsPlatformApplicationMisc::RequestMinimize();
      FWindowsPlatformMisc::RequestExit(0);
    }
  ```
  What is happening here is that the game is loading a GUID from a blueprint, and then attempting to create/open a file located in the user's temporary files directory (`%temp%`).

  When it does this, it creates/opens the file with the `dwShareMode` parameter set to `0`.

  Upon looking at the documentation of `CreateFileW`, we can see a `dwShareMode` of `0` prevents all shared access to this file.
  https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew

  This means that when the second server boots, it is unable to get a valid handle to the file. 
  When this happens, the server logs the message that you are already running the server, and then closes the server.

  This patch resolves this by changing the `dwShareMode` parameter to `FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE` for this file when the server attempts to create/open it.
</details>
