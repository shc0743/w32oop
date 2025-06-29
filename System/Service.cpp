#include "./Service.hpp"
using namespace w32oop;
using namespace w32oop::system;


Service ServiceManager::get(wstring name, DWORD access) {
	return Service(OpenServiceW(scm, name.c_str(), access));
}

Service w32oop::system::ServiceManager::create(wstring name, wstring binPath, 
	DWORD startup_type, wstring displayName, wstring description, DWORD type) {
	if (displayName.empty()) displayName = name;
	DWORD err = 0;
	Service svc(CreateServiceW(scm, name.c_str(), displayName.c_str(),
		SERVICE_ALL_ACCESS, type, startup_type, SERVICE_ERROR_NORMAL, binPath.c_str(),
		NULL, NULL, NULL, NULL, NULL));

	if (!description.empty()) {
		SERVICE_DESCRIPTIONW a{};
		auto ptr = new WCHAR[(description.length() + 1)];
		wcscpy_s(ptr, description.length() + 1, description.c_str());
		a.lpDescription = ptr;
		ChangeServiceConfig2W(svc, SERVICE_CONFIG_DESCRIPTION, &a);
        delete[] ptr;
	}

	return svc;
}

bool w32oop::system::Service::remove() {
	return DeleteService(hService);
}

bool w32oop::system::Service::start() {
	return StartServiceW(hService, 0, NULL);
}

bool w32oop::system::Service::control(DWORD dwControl) {
	SERVICE_STATUS status{};
	return ControlService(hService, dwControl, &status);
}

DWORD w32oop::system::Service::status() {
	SERVICE_STATUS_PROCESS status{};
	DWORD dwBytesNeeded{};

	// Check the status in case the service is not stopped.
	if (!QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO,
		(LPBYTE)&status, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			throw exceptions::service_operation_failed_exception("The operation failed for an operation-specific reason");
		throw exceptions::service_operation_failed_exception("Unexpected: The buffer should not be too small, but it was");
	}
	return status.dwCurrentState;
}
