#pragma once
#include "../FileSystem/File.h"

class CDlgCache : public CThread, public XFILE::IFileCallback
{
public:
  CDlgCache(DWORD dwDelay = 0);
  virtual ~CDlgCache();
  void Update();
  void SetMessage(const CStdString& strMessage);
  bool IsCanceled() const;
  void ShowProgressBar(bool bOnOff);
  void SetPercentage(int iPercentage);

  void Close(bool bForceClose = false);

  virtual void Process();
  virtual bool OnFileCallback(void* pContext, int ipercent, float curSpeed, float avgSpeed);

protected:

  void OpenDialog();

  DWORD m_dwTimeStamp;
  DWORD m_dwDelay;
  CGUIDialogProgress* m_pDlg;
  CStdString m_strLinePrev;  
  bool bSentCancel;
  bool m_bOpenTried;
};
