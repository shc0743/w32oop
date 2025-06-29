import ctypes
k32 = ctypes.WinDLL('kernel32.dll')
pid = int(input("Enter PID: "))
hProcess = k32.OpenProcess(((0x000F0000) | (0x00100000) | 0xFFFF), 0, pid) # PROCESS_ALL_ACCESS
print('hProcess =', hProcess)

input("Will trigger STATUS_ACCESS_VIOLATION, Enter to continue...")
hThread = k32.CreateRemoteThread(hProcess, 0, 0, 0x0, 0, 0, 0) # 故意在目标进程触发异常（0xC0000005）
print('hThread =', hThread)
k32.CloseHandle(hThread)
k32.CloseHandle(hProcess)

input("Will trigger a WM_QUERYENDSESSION, Enter to continue...")
result = ctypes.WinDLL('user32').SendMessageW(ctypes.WinDLL('user32').FindWindowW(0, "My Notepad"), 0x11, 0, 0) # WM_QUERYENDSESSION
print('result =', result)

input("Enter to exit...")
