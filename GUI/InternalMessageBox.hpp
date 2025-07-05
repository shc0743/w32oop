extern "C" {
    int WINAPI MessageBoxTimeoutA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption,
        UINT uType, WORD wLanguageId, DWORD dwMilliseconds);
    int WINAPI MessageBoxTimeoutW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption,
        UINT uType, WORD wLanguageId, DWORD dwMilliseconds);
};