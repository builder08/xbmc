#pragma once
#include "idirectory.h"

class CURL;
class TiXmlElement;

using namespace DIRECTORY;
namespace DIRECTORY
{
class CShoutcastDirectory :
      public IDirectory
{
public:
  CShoutcastDirectory(void);
  virtual ~CShoutcastDirectory(void);
  virtual bool GetDirectory(const CStdString& strPath, CFileItemList &items);
  bool ParseGenres(TiXmlElement *root, CFileItemList &items, CURL &url);
  bool ParseStations(TiXmlElement *root, CFileItemList &items, CURL &url);
protected:
};

}
