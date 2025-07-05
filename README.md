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
- [ ] [Concurrence](./Concurrence/)
  - [ ] [Process](./Concurrence/Process.hpp) (`hProcess`)
  - [ ] [Thread](./Concurrence/Thread.hpp) (`hThread`)
  - [ ] [EventObject](./Event/EventObject.hpp) (`hEvent`)
- [ ] [Network](./Network/)
  - [ ] [WinINetTask](./Network/WinINetTask.hpp) (`HINTERNET`)
  - [ ] [DownloadTask](./Network/DownloadTask.hpp) (`HINTERNET`)
- [ ] [System](./System/)
  - [x] [Service](./System/Service.hpp) (`SC_HANDLE`)
  - [ ] [Registry](./System/Registry.hpp) (`HKEY`)
- [ ] [Utility](./Utility/)
  - [ ] StringUtil (a set of string encodings&conversions utility)
    - [ ] [converts](./Utility/StringUtil/converts.hpp)
    - [ ] [encodings](./Utility/StringUtil/encodings.hpp)
  - [x] [RAII Helper Class](./Utility/RAII.hpp)

## License
[MIT](./LICENSE)
