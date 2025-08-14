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
- [x] [Network](./Network/)
  - [x] [HttpRequest](./Network/HttpRequest.hpp) (`HINTERNET`)
  - [x] [HttpResponse](./Network/HttpResponse.hpp) (`HINTERNET`)
- [x] [System](./System/)
  - [x] [Service](./System/Service.hpp) (`SC_HANDLE`)
  - [x] [Registry](./System/Registry.hpp) (`HKEY`)
  - [x] [Hooks](./System/Hooks.hpp) (`HHOOK`)
- [x] [Utility](./Utility/)
  - [x] StringUtil (a set of string encodings&conversions utility)
    - [x] [converts](./Utility/StringUtil/converts.hpp)
    - [x] [encodings](./Utility/StringUtil/encodings.hpp)
    - [x] [operations](./Utility/StringUtil/operations.hpp)
  - [x] [RAII Helper Class](./Utility/RAII.hpp)
  - [x] [Memory Utility](./Utility/Memory.hpp)

## License
[MIT](./LICENSE)
