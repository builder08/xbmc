
#include "../stdafx.h"
#include "RarDirectory.h"
#include "../utils/log.h"
#include "../Util.h"

namespace DIRECTORY
{
  CRarDirectory::CRarDirectory()
  {
  }

  CRarDirectory::~CRarDirectory()
  {
  }

  bool CRarDirectory::GetDirectory(const CStdString& strPath, CFileItemList& items)
  {
    // the RAR code depends on things having a "\" at the end of the path
    CStdString strSlashPath = strPath;
    if (!CUtil::HasSlashAtEnd(strSlashPath))
      strSlashPath += "\\";
    CURL url(strSlashPath);
    if (g_RarManager.GetFilesInRar(items,url.GetHostName(),true,url.GetFileName()))
    {
      // fill in paths
      for( int iEntry=0;iEntry<items.Size();++iEntry)
      {
        if (items[iEntry]->IsParentFolder())
          continue;
        if ((IsAllowed(items[iEntry]->m_strPath)) || (items[iEntry]->m_bIsFolder))
        {
          items[iEntry]->m_strPath = strSlashPath + items[iEntry]->m_strPath;
        }
        else
        {
          items.Remove(iEntry);
          iEntry--; //do not confuse loop
        }
      } 
      return( true);
    }
    else
      return( false );
  }

  bool CRarDirectory::ContainsFiles(const CStdString& strPath)
  {
    CFileItemList items;
    if (g_RarManager.GetFilesInRar(items,strPath))
    {
      if (items.Size() > 1)
        return true;
      
      return false;
    }
    
    return false;
  }
}