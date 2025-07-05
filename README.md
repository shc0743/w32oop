# w32oop
C++ Win32 Object-Oriented Programming Framework

### Note: This project is still under development.

## Development Progress

- [x] [Core](./Core/)
  - [x] [w32def](./Core/w32def.hpp)
- [x] [GUI](./GUI/)
  - [x] [Window](./GUI/Window.hpp) (`HWND`)
- [ ] [IO](./IO/)
  - [ ] [File](./IO/File.hpp) (`hFile`)
  - [ ] [Pipe](./IO/Pipe.hpp) (`hPipe`)
- [ ] [Concurrency](./Concurrency/)
  - [ ] [Process](./Concurrency/Process.hpp) (`hProcess`)
  - [ ] [Thread](./Concurrency/Thread.hpp) (`hThread`)
  - [x] [EventObject](./Concurrency/EventObject.hpp) (`hEvent`)
- [ ] [Network](./Network/)
  - [ ] [WinINetTask](./Network/WinINetTask.hpp) (`HINTERNET`)
  - [ ] [DownloadTask](./Network/DownloadTask.hpp) (`HINTERNET`)
- [ ] [System](./System/)
  - [x] [Service](./System/Service.hpp) (`SC_HANDLE`)
  - [ ] [Registry](./System/Registry.hpp) (`HKEY`)
  - [x] [Hooks](./System/Hooks.hpp) (`HHOOK`)
- [ ] [Utility](./Utility/)
  - [ ] StringUtil (a set of string encodings&conversions utility)
    - [ ] [converts](./Utility/StringUtil/converts.hpp)
    - [ ] [encodings](./Utility/StringUtil/encodings.hpp)
  - [x] [RAII Helper Class](./Utility/RAII.hpp)
  - [x] [Memory Utility](./Utility/Memory.hpp)

## License
[MIT](./LICENSE)
