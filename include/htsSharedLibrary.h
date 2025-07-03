// Copyright(c) 2025-present, 徐翔.
// Distributed under the Apache License Version 2.0 (https://www.apache.org/licenses/LICENSE-2.0)

#ifndef _HTS_SHARED_LIBRARY_
#define _HTS_SHARED_LIBRARY_

#ifdef _WIN32
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <filesystem>

namespace hts {
	class SharedLibrary {
	public:
#ifdef _WIN32
		typedef HMODULE native_handle_t;
#else
		typedef void* native_handle_t;
#endif
		SharedLibrary() noexcept
			: m_handle(nullptr)
		{}

		~SharedLibrary() noexcept {
			Unload();
		}

		SharedLibrary(SharedLibrary&& sl) noexcept
			: m_handle(sl.m_handle)
		{
			sl.m_handle = nullptr;
		}

		SharedLibrary& operator=(SharedLibrary& sl) noexcept {
			Swap(sl);
			return *this;
		}

		bool Load(std::filesystem::path filename) {
			Unload();

			// If do not has extension, add it
			if (!filename.has_extension()) {
				filename.replace_extension(Suffix());
			}

#ifdef _WIN32
			DWORD native_mode = LOAD_LIBRARY_SEARCH_DEFAULT_DIRS;
			m_handle = ::LoadLibraryExW(filename.c_str(), 0, native_mode);
#else	
			m_handle = dlopen(filename.c_str(), RTLD_LAZY);
#endif
			return (m_handle != nullptr);
		}

		bool IsLoaded() const noexcept {
			return (m_handle != nullptr);
		}

		void Unload() noexcept {
			if (m_handle != nullptr) {
#ifdef _WIN32
				FreeLibrary(m_handle);
#else
				dlclose(m_handle);
#endif
				m_handle = nullptr;
			}
		}

		void Swap(SharedLibrary& rhs) noexcept {
			std::swap(m_handle, rhs.m_handle);
		}

		std::filesystem::path GetPath() const {
#ifdef _WIN32
			wchar_t szDir[MAX_PATH];
			if (GetModuleFileNameW(m_handle, szDir, MAX_PATH))
			{
				return szDir;
			}
			else
				return "";
#else
			Dl_info dl_info;
			if (dladdr(m_handle, &dl_info)) {
				return dl_info.dli_fname;
			}
			else {
				return "";
			}
#endif
		}

		static std::filesystem::path Suffix() {
#ifdef _WIN32
			return L".dll";
#else
			return ".so";
#endif
		}

		void* GetSymbolAddress(const char* symbo_name) const noexcept {
			if (m_handle) {
#ifdef _WIN32
				return GetProcAddress(m_handle, symbo_name);
#else
				return dlsym(m_handle, symbo_name);
#endif
			}
			return nullptr;
		}

		native_handle_t native() const noexcept {
			return m_handle;
		}

	private:
		native_handle_t m_handle;
	};
}

#endif // _HTS_SHARED_LIBRARY_