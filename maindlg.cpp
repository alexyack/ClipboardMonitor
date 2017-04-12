// maindlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "maindlg.h"

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CRegKey reg;
	DWORD cbSize;

	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);

	m_bFirst = TRUE;

	m_hWndNextChain = SetClipboardViewer();

	m_nCurrentIndex = 0;
	m_strFormat = TEXT("Capture_%03d.bmp");
	cbSize = 512;

	if(ERROR_SUCCESS == reg.Open(HKEY_CURRENT_USER, TEXT("Software\\Alexyack\\Clipboard Monitor")))
	{
		reg.QueryDWORDValue(TEXT("Current Index"), m_nCurrentIndex);
		reg.QueryStringValue(TEXT("Format String"), m_strFormat.GetBufferSetLength(cbSize), &cbSize); 
		m_strFormat.ReleaseBuffer();
		reg.Close();
	}

	DoDataExchange();

	return TRUE;
}

LRESULT CMainDlg::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CSimpleDialog<IDD_ABOUTBOX> dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CloseDialog(IDOK);
	return 0;
}

void CMainDlg::CloseDialog(int nVal)
{
	CRegKey reg;

	ChangeClipboardChain(m_hWndNextChain);

	if(ERROR_SUCCESS == reg.Create(HKEY_CURRENT_USER, TEXT("Software\\Alexyack\\Clipboard Monitor")))
	{
		reg.SetDWORDValue(TEXT("Current Index"), m_nCurrentIndex);
		reg.SetStringValue(TEXT("Format String"), m_strFormat); 
		reg.Close();
	}

	DestroyWindow();
	::PostQuitMessage(nVal);
}

LRESULT CMainDlg::OnChangeClipChain(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	HWND hWndToRemove = HWND(wParam);
	HWND hWndNextChain = HWND(lParam);

	if(hWndToRemove == m_hWndNextChain)
		m_hWndNextChain = hWndNextChain;
	else
		::SendMessage(m_hWndNextChain, uMsg, wParam, lParam);
	
	return 0;
}

LRESULT CMainDlg::OnDrawClipboard(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CBitmap bmp;
	CImage image;
	WTL::CString strname;

	::SendMessage(m_hWndNextChain, uMsg, wParam, lParam);

	if(m_bFirst)
	{
		m_bFirst = FALSE;
		return 0;
	}

	if(OpenClipboard())
	{
		bmp = HBITMAP(::GetClipboardData(CF_BITMAP));
		if(bmp != NULL)
		{
			Beep(1200, 10);

			DoDataExchange(TRUE);

			strname.Format(m_strFormat, m_nCurrentIndex);

			image.Attach(bmp);
			image.Save(strname);

			m_nCurrentIndex++;

			DoDataExchange();
		}

		CloseClipboard();
	}

	return 0;
}
