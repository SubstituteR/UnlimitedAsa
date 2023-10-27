#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <detours.h>
#include <filesystem>
#include <string>

#include <MoarPtr/moar_ptr.h>

moar::function_ptr<HANDLE(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE), moar::types::stdcall_t> create_file(reinterpret_cast<void*>(CreateFileW));

std::wstring get_temp()
{
    wchar_t path[MAX_PATH + 1] = { 0 };
    wchar_t full_path[MAX_PATH + 1] = { 0 };
    GetTempPathW(MAX_PATH + 1, path);
    GetLongPathNameW(path, full_path, MAX_PATH + 1);
    return { full_path };
}

HANDLE __stdcall create_file_hook(const LPCWSTR file_name, const DWORD desired_access, DWORD share_mode, const LPSECURITY_ATTRIBUTES security_attributes, const DWORD creation_disposition, const DWORD flags_and_attributes, const HANDLE template_file)
{
    const auto file = std::wstring(file_name);

    if(const auto temp_path = get_temp(); share_mode == 0 && file.find(temp_path) != std::string::npos)
        share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    
    return create_file.original(file_name, desired_access, share_mode, security_attributes, creation_disposition, flags_and_attributes, template_file);
}

void patch()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(create_file.mut(), reinterpret_cast<void*>(create_file_hook));
    DetourTransactionCommit();
}


