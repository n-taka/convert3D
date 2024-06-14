/****************************** Module Header ******************************\
Module Name:  FileContextMenuExt.cpp
Project:      CppShellExtContextMenuHandler
Copyright (c) Microsoft Corporation.

The code sample demonstrates creating a Shell context menu handler with C++.

A context menu handler is a shell extension handler that adds commands to an
existing context menu. Context menu handlers are associated with a particular
file class and are called any time a context menu is displayed for a member
of the class. While you can add items to a file class context menu with the
registry, the items will be the same for all members of the class. By
implementing and registering such a handler, you can dynamically add items to
an object's context menu, customized for the particular object.

The example context menu handler adds the menu item "Display File Name (C++)"
to the context menu when you right-click a .cpp file in the Windows Explorer.
Clicking the menu item brings up a message box that displays the full path
of the .cpp file.

This source is subject to the Microsoft Public License.
See http://www.microsoft.com/opensource/licenses.mspx#Ms-PL.
All other rights reserved.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#include "FileContextMenuExt.h"
#include "resource.h"
#include <strsafe.h>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include <string>
#include <filesystem>
#include <igl/read_triangle_mesh.h>
#include <igl/write_triangle_mesh.h>

extern HINSTANCE g_hInst;
extern long g_cDllRef;

#define IDM_DISPLAY             0  // The command's identifier offset

FileContextMenuExt::FileContextMenuExt(void) : m_cRef(1),
m_pszMenuText(L"&convert3D"),
m_pszVerb("convert3D"),
m_pwszVerb(L"convert3D"),
m_pszVerbCanonicalName("convert3D"),
m_pwszVerbCanonicalName(L"convert3D"),
m_pszVerbHelpText("convert 3D models into another format"),
m_pwszVerbHelpText(L"convert 3D models into another format")
{
	InterlockedIncrement(&g_cDllRef);

	// Load the bitmap for the menu item. 
	// If you want the menu item bitmap to be transparent, the color depth of 
	// the bitmap must not be greater than 8bpp.
	//m_hMenuBmp = LoadImage(g_hInst, MAKEINTRESOURCE(IDB_OK), 
	//    IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADTRANSPARENT);
}

FileContextMenuExt::~FileContextMenuExt(void)
{
	if (m_hMenuBmp)
	{
		DeleteObject(m_hMenuBmp);
		m_hMenuBmp = NULL;
	}

	InterlockedDecrement(&g_cDllRef);
}


void FileContextMenuExt::OnVerbConvertFiles(const std::wstring &format)
{
	for (const auto& selectedFile : this->m_szSelectedFiles)
	{
		std::filesystem::path inFilePath(selectedFile);
		const std::filesystem::path currentDir(inFilePath.parent_path());
		const std::wstring inFileNameWithExt(inFilePath.filename().wstring());
		const std::wstring inFileNameWithoutExt(inFilePath.stem().wstring());
		std::wstring inFileNameExt(inFilePath.extension().wstring());
		std::transform(inFileNameExt.begin(), inFileNameExt.end(), inFileNameExt.begin(), ::tolower);

		if (inFileNameExt == L".msh" || inFileNameExt == L".mesh" || inFileNameExt == L".obj" || inFileNameExt == L".off" || inFileNameExt == L".stl" || inFileNameExt == L".ply" || inFileNameExt == L".wrl")
		{
			// get fresh filename
			std::filesystem::path outFilePath;
			std::wstring outFileName;
			int count = 0;
			do {
				outFileName = inFileNameWithoutExt;
				if (count > 0)
				{
					outFileName += (L" (" + std::to_wstring(count) + L")");
				}
				outFileName += (L"." + format);
				outFilePath = currentDir;
				outFilePath.append(outFileName);
				++count;
			} while (std::filesystem::exists(outFilePath));

			// here, we safely use outFilePath

			Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> V;
			Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic> F;
			igl::read_triangle_mesh(inFilePath.string(), V, F);
			igl::write_triangle_mesh(outFilePath.string(), V, F, igl::FileEncoding::Binary);
		}
	}
}


#pragma region IUnknown

// Query to the interface the component supported.
IFACEMETHODIMP FileContextMenuExt::QueryInterface(REFIID riid, void** ppv)
{
	static const QITAB qit[] =
	{
		QITABENT(FileContextMenuExt, IContextMenu),
		QITABENT(FileContextMenuExt, IShellExtInit),
		{ 0 },
	};
	return QISearch(this, qit, riid, ppv);
}

// Increase the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) FileContextMenuExt::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

// Decrease the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) FileContextMenuExt::Release()
{
	ULONG cRef = InterlockedDecrement(&m_cRef);
	if (0 == cRef)
	{
		delete this;
	}

	return cRef;
}

#pragma endregion


#pragma region IShellExtInit

// Initialize the context menu handler.
IFACEMETHODIMP FileContextMenuExt::Initialize(
	LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hKeyProgID)
{
	if (NULL == pDataObj)
	{
		return E_INVALIDARG;
	}

	HRESULT hr = E_FAIL;

	FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stm;

	// The pDataObj pointer contains the objects being acted upon. In this 
	// example, we get an HDROP handle for enumerating the selected files and 
	// folders.
	if (SUCCEEDED(pDataObj->GetData(&fe, &stm)))
	{
		// Get an HDROP handle.
		HDROP hDrop = static_cast<HDROP>(GlobalLock(stm.hGlobal));
		if (hDrop != NULL)
		{
			// Determine how many files are involved in this operation. This 
			// code sample displays the custom context menu item when only 
			// one file is selected. 
			m_szSelectedFiles.clear();
			UINT nFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
			for (UINT uFile = 0; uFile < nFiles; ++uFile)
			{
				// Get the path of the file.
				wchar_t selectedFile[MAX_PATH];
				if (0 != DragQueryFile(hDrop, uFile, selectedFile, ARRAYSIZE(selectedFile)))
				{
					m_szSelectedFiles.push_back(std::wstring(selectedFile));
				}
			}
			hr = (m_szSelectedFiles.size() > 0) ? S_OK : E_FAIL;

			GlobalUnlock(stm.hGlobal);
		}

		ReleaseStgMedium(&stm);
	}

	// If any value other than S_OK is returned from the method, the context 
	// menu item is not displayed.
	return hr;
}

#pragma endregion


#pragma region IContextMenu

//
//   FUNCTION: FileContextMenuExt::QueryContextMenu
//
//   PURPOSE: The Shell calls IContextMenu::QueryContextMenu to allow the 
//            context menu handler to add its menu items to the menu. It 
//            passes in the HMENU handle in the hmenu parameter. The 
//            indexMenu parameter is set to the index to be used for the 
//            first menu item that is to be added.
//
IFACEMETHODIMP FileContextMenuExt::QueryContextMenu(
	HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	// If uFlags include CMF_DEFAULTONLY then we should not do anything.
	if (CMF_DEFAULTONLY & uFlags)
	{
		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
	}

	// create submenu for our extension
	HMENU hSubmenu = CreatePopupMenu();
	UINT uID = idCmdFirst + IDM_DISPLAY;

	if (!InsertMenu(hSubmenu, 0, MF_BYPOSITION, uID++, L"OBJ"))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if (!InsertMenu(hSubmenu, 1, MF_BYPOSITION, uID++, L"PLY"))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if (!InsertMenu(hSubmenu, 2, MF_BYPOSITION, uID++, L"STL"))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if (!InsertMenu(hSubmenu, 3, MF_BYPOSITION, uID++, L"WRL"))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if (!InsertMenu(hSubmenu, 4, MF_BYPOSITION, uID++, L"OFF"))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if (!InsertMenu(hSubmenu, 5, MF_BYPOSITION, uID++, L"MESH"))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	// Use either InsertMenu or InsertMenuItem to add menu items.
	// Learn how to add sub-menu from:
	// http://www.codeproject.com/KB/shell/ctxextsubmenu.aspx
	MENUITEMINFO mii = { sizeof(mii) };
	mii.fMask = MIIM_SUBMENU | MIIM_STRING | MIIM_ID;
	mii.wID = uID++;
	mii.hSubMenu = hSubmenu;
	mii.dwTypeData = m_pszMenuText;
	//mii.hbmpItem = static_cast<HBITMAP>(m_hMenuBmp);
	if (!InsertMenuItem(hMenu, indexMenu, TRUE, &mii))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	// Add a separator.
	MENUITEMINFO sep = { sizeof(sep) };
	sep.fMask = MIIM_TYPE;
	sep.fType = MFT_SEPARATOR;
	if (!InsertMenuItem(hMenu, indexMenu + 1, TRUE, &sep))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	// Return an HRESULT value with the severity set to SEVERITY_SUCCESS. 
	// Set the code value to the offset of the largest command identifier 
	// that was assigned, plus one (1).
	return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(uID - idCmdFirst));
}


//
//   FUNCTION: FileContextMenuExt::InvokeCommand
//
//   PURPOSE: This method is called when a user clicks a menu item to tell 
//            the handler to run the associated command. The lpcmi parameter 
//            points to a structure that contains the needed information.
//
IFACEMETHODIMP FileContextMenuExt::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
	// If lpVerb really points to a string, ignore this function call and bail out.
	if (0 != HIWORD(pici->lpVerb))
		return E_INVALIDARG;

	// Get the command index.
	switch (LOWORD(pici->lpVerb))
	{
	case 0:
	{
		OnVerbConvertFiles(L"obj");
		return S_OK;
	}
	break;

	case 1:
	{
		OnVerbConvertFiles(L"ply");
		return S_OK;
	}
	break;

	case 2:
	{
		OnVerbConvertFiles(L"stl");
		return S_OK;
	}
	break;

	case 3:
	{
		OnVerbConvertFiles(L"wrl");
		return S_OK;
	}
	break;

	case 4:
	{
		OnVerbConvertFiles(L"off");
		return S_OK;
	}
	break;

	case 5:
	{
		OnVerbConvertFiles(L"mesh");
		return S_OK;
	}
	break;

	default:
		return E_INVALIDARG;
		break;
	}
}


//
//   FUNCTION: CFileContextMenuExt::GetCommandString
//
//   PURPOSE: If a user highlights one of the items added by a context menu 
//            handler, the handler's IContextMenu::GetCommandString method is 
//            called to request a Help text string that will be displayed on 
//            the Windows Explorer status bar. This method can also be called 
//            to request the verb string that is assigned to a command. 
//            Either ANSI or Unicode verb strings can be requested. This 
//            example only implements support for the Unicode values of 
//            uFlags, because only those have been used in Windows Explorer 
//            since Windows 2000.
//
IFACEMETHODIMP FileContextMenuExt::GetCommandString(UINT_PTR idCommand,
	UINT uFlags, UINT* pwReserved, LPSTR pszName, UINT cchMax)
{
	HRESULT hr = E_INVALIDARG;

	if (idCommand == IDM_DISPLAY)
	{
		switch (uFlags)
		{
		case GCS_HELPTEXTW:
			// Only useful for pre-Vista versions of Windows that have a 
			// Status bar.
			hr = StringCchCopy(reinterpret_cast<PWSTR>(pszName), cchMax,
				m_pwszVerbHelpText);
			break;

		case GCS_VERBW:
			// GCS_VERBW is an optional feature that enables a caller to 
			// discover the canonical name for the verb passed in through 
			// idCommand.
			hr = StringCchCopy(reinterpret_cast<PWSTR>(pszName), cchMax,
				m_pwszVerbCanonicalName);
			break;

		default:
			hr = S_OK;
		}
	}

	// If the command (idCommand) is not supported by this context menu 
	// extension handler, return E_INVALIDARG.

	return hr;
}

#pragma endregion