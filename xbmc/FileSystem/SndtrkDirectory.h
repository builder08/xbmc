#pragma once

#include "directory.h"
#include <vector>


class CSoundtrack
{
public:

    //VOID    GetSoundtrackName( WCHAR* strName ) { wcscpy( strName, m_strName ); }
    //UINT    GetSongCount() { return m_uSongCount; }

    WCHAR       strName[42];
    UINT        uSongCount;
    UINT    uSoundtrackId;
};
__MSL_FIX_ITERATORS__(CSoundtrack);
typedef	vector<CSoundtrack> SOUNDTRACK;
typedef	vector<CSoundtrack>::iterator ISOUNDTRACK;


using namespace DIRECTORY;
namespace DIRECTORY
{

  class CSndtrkDirectory :
    public CDirectory
  {
  public:
    CSndtrkDirectory(void);
    virtual ~CSndtrkDirectory(void);
    virtual bool  GetDirectory(const CStdString& strPath,VECFILEITEMS &items);
	bool IsAlone(const CStdString& strPath);
  };
};
