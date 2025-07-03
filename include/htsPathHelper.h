// Copyright(c) 2025-present, 徐翔.
// Distributed under the Apache License Version 2.0 (https://www.apache.org/licenses/LICENSE-2.0)

#ifndef _HTS_PATH_HELPER_
#define _HTS_PATH_HELPER_

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma comment(lib,"version.lib")
#include <io.h>
#include <Knownfolders.h>
#include <Shlobj.h>
#pragma comment( lib, "shell32.lib")
#include <Shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")
#endif

#include <filesystem>

namespace hts
{
	class PathHelper
	{
	public:
		static std::string GetDLLFileName(void* funInDll)
		{			
			#ifdef _WIN32
				char szDir[MAX_PATH];
				HMODULE hModule;
				GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)funInDll, &hModule);
				if (hModule != NULL)
				{
					GetModuleFileNameA(hModule, szDir, MAX_PATH);
					return std::string(szDir);
				}
				return std::string();
			#else			
   				Dl_info dl_info;
   				int rec = dladdr((void*)funInDll, &dl_info);
				if (rec)
				{
					return std::string(dl_info.dli_fname);
				}
				else
				{
					return std::string();
				}			
			#endif			
		}

		static std::string GetDLLFileName()
		{
			std::string (*pFunc)() = &PathHelper::GetDLLFileName;
			return GetDLLFileName((void*)pFunc);
		}

		static bool CreateFullDirectory(std::string dir)
		{
			std::filesystem::path path(dir);
			path.remove_filename();
			if (path.empty())
				return true;
			else if (std::filesystem::exists(path))
			{
				return true;
			}
			else
			{
				std::error_code _Ec;
				return std::filesystem::create_directories(path, _Ec);
			}
		}

		static std::string GetPathFromFilePath(const std::string& filePath)
		{	
			std::filesystem::path pathfileName(filePath.c_str());
			std::string str(pathfileName.parent_path().string());
			if (!str.empty())
			{
			#ifdef _WIN32
				str.append("\\");
			#else
				str.append('/');
			#endif // _WIN32
			}
			return str;
		}		

		static std::string GetNameFromFilePath(const std::string& filePath)
		{
			return std::filesystem::path(filePath.c_str()).filename().string();
		}

		static std::string GetFullPath(std::string basePath, std::string relativePath)
		{
			std::filesystem::path rPath(relativePath);
			if (rPath.is_relative())
			{
				std::filesystem::path base(basePath);
				base.append(relativePath);
				std::filesystem::path full = weakly_canonical(base);	// 去掉路径中的"." ".."
				return full.string();
			}
			else
			{				
				return relativePath;
			}						
		}		

		static std::string GetFullPath(std::string basePath, std::string relativePath, std::string altBase)
		{
			std::string path = GetFullPath(basePath, relativePath);
			if (!IsFileExist(path))
			{
				return GetFullPath(altBase, relativePath);
			}
			return path;
		}		

		static bool IsFileExist(std::string& filePath)
		{
			std::filesystem::path tpath(filePath);
			return(is_regular_file(tpath));
		}		

		static bool IsDirectory(std::string& directory)
		{
			std::filesystem::path tpath(directory);
			return(is_directory(tpath));
		}

		static std::string GetExeFileName()
		{
			#ifdef _WIN32
			char exeFileName[MAX_PATH];
			GetModuleFileNameA(NULL, exeFileName, MAX_PATH);
			return std::string(exeFileName);
			#else			
			char exeFileName[_POSIX_PATH_MAX];
			int len = readlink("/proc/self/exe", exeFileName, _POSIX_PATH_MAX);	
			if (len <= 0)
			{
				return std::string();
			}
			exeFileName[len] = 0;
			return std::string(exeFileName);
			#endif
		}

		static void GetAllFilesInDictionary(std::string paths, std::vector<std::string>& files)
		{			
			if (std::filesystem::exists(paths.c_str()))
			{
				for (auto& it : std::filesystem::recursive_directory_iterator(paths.c_str()))
				{					
					if (!is_directory(it.path()))
					{
						std::string tmp = it.path().string();
						files.push_back(tmp);
					}
				}
			}			
		}
	};
}
#endif // _HTS_PATH_HELPER_