#pragma once
#include "GUIWindowVideoBase.h"

class CGUIWindowVideoGenre : public CGUIWindowVideoBase
{
public:
  CGUIWindowVideoGenre(void);
  virtual ~CGUIWindowVideoGenre(void);
  virtual bool OnMessage(CGUIMessage& message);  

protected:
  virtual void SaveViewMode();
  virtual void LoadViewMode();
  virtual int SortMethod();
  virtual bool SortAscending();

  virtual void FormatItemLabels();
  virtual void SortItems(CFileItemList& items);
  virtual void Update(const CStdString &strDirectory);
  virtual void OnClick(int iItem);
  virtual void OnInfo(int iItem);
  virtual void OnDeleteItem(int iItem) {return;};
};
