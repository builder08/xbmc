/*
 *      Copyright (C) 2005-2009 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "DAVDirectory.h"
#include "URL.h"
#include "Util.h"
#include "FileCurl.h"
#include "FileItem.h"
#include "utils/RegExp.h"
#include "AdvancedSettings.h"
#include "StringUtils.h"
#include "utils/CharsetConverter.h"
#include "XMLUtils.h"
#include "utils/strptime.h"

using namespace XFILE;
using namespace DIRECTORY;

CDAVDirectory::CDAVDirectory(void) {}
CDAVDirectory::~CDAVDirectory(void) {}

/*
 * Return true if pElement value is equal value without namespace.
 *
 * if pElement is <DAV:foo> and value is foo then ValueWithoutNamespace is true
 */ 
bool CDAVDirectory::ValueWithoutNamespace(const TiXmlNode *pNode, CStdString value)
{
  CStdStringArray result;
  const TiXmlElement *pElement;

  if (!pNode)
  {
    return false;
  }

  pElement = pNode->ToElement();

  if (!pElement)
  {
    return false;
  }
    
  StringUtils::SplitString(pElement->Value(), ":", result, 2);

  if (result.size() == 1 && result[0] == value)
  {
    return true;
  }
  else if (result.size() == 2 && result[1] == value)
  {
    return true;
  }
  else if (result.size() > 2)
  {
    CLog::Log(LOGERROR, "%s - Splitting %s failed, size(): %lu, value: %s", __FUNCTION__, pElement->Value(), (unsigned long int)result.size(), value.c_str());
  }
    
  return false;
}

/*
 * Search for <status> and return its content
 */
CStdString CDAVDirectory::GetStatusTag(const TiXmlElement *pElement)
{
  const TiXmlElement *pChild;

  for (pChild = pElement->FirstChild()->ToElement(); pChild != 0; pChild = pChild->NextSibling()->ToElement()) 
  {
    if (ValueWithoutNamespace(pChild, "status"))
    {
      return CStdString(pChild->GetText());
    }
  }

  return CStdString("");
}

/*
 * Parses a <response>
 *
 * <!ELEMENT response (href, ((href*, status)|(propstat+)), responsedescription?) >
 * <!ELEMENT propstat (prop, status, responsedescription?) >
 *
 */
bool CDAVDirectory::ParseResponse(const TiXmlElement *pElement, CFileItem &item)
{
  const TiXmlNode *pResponseChild;
  const TiXmlNode *pPropstatChild;
  const TiXmlNode *pPropChild;

  /* Iterate response children elements */
  for (pResponseChild = pElement->FirstChild(); pResponseChild != 0; pResponseChild = pResponseChild->NextSibling()) 
  {
    if (ValueWithoutNamespace(pResponseChild, "href"))
    {
      item.m_strPath = pResponseChild->ToElement()->GetText();
      CUtil::RemoveSlashAtEnd(item.m_strPath);
    }
    else if (ValueWithoutNamespace(pResponseChild, "propstat"))
    {
      if (GetStatusTag(pResponseChild->ToElement()) == "HTTP/1.1 200 OK")
      {
        /* Iterate propstat children elements */
        for (pPropstatChild = pResponseChild->FirstChild(); pPropstatChild != 0; pPropstatChild = pPropstatChild->NextSibling()) 
        {
          if (ValueWithoutNamespace(pPropstatChild, "prop"))
          {
            /* Iterate all properties available */
            for (pPropChild = pPropstatChild->FirstChild(); pPropChild != 0; pPropChild = pPropChild->NextSibling())
            {
              if (ValueWithoutNamespace(pPropChild, "getcontentlength"))
              {
                item.m_dwSize = strtoll(pPropChild->ToElement()->GetText(), NULL, 10);
              }
              else if (ValueWithoutNamespace(pPropChild, "getlastmodified"))
              {
                struct tm timeDate = {0};
                strptime(pPropChild->ToElement()->GetText(), "%a, %d %b %Y %T", &timeDate);
                item.m_dateTime = mktime(&timeDate);
              }
              else if (ValueWithoutNamespace(pPropChild, "displayname"))
              {
                item.SetLabel(pPropChild->ToElement()->GetText());
              }
              else if (!item.m_dateTime.IsValid() && ValueWithoutNamespace(pPropChild, "creationdate"))
              {
                struct tm timeDate = {0};
                strptime(pPropChild->ToElement()->GetText(), "%Y-%m-%dT%T", &timeDate);
                item.m_dateTime = mktime(&timeDate);
              }
              else if (ValueWithoutNamespace(pPropChild, "resourcetype"))
              {
                if (ValueWithoutNamespace(pPropChild->FirstChild(), "collection"))
                {
                  item.m_bIsFolder = true;
                }
              }
            }
          }
        }
      }
    }
  }
  
  return true;
}

bool CDAVDirectory::GetDirectory(const CStdString& strPath, CFileItemList &items)
{
  CFileCurl dav;
  CURL url(strPath);
  CStdString strResponse;
  CStdString strRequest = "PROPFIND";
  TiXmlDocument davResponse;
  TiXmlNode *pChild;

  dav.SetCustomRequest(strRequest);
  dav.SetContentType("text/xml; charset=\"utf-8\"");
  dav.SetRequestHeader("depth", 1);
  dav.SetPostData(
    "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
    " <D:propfind xmlns:D=\"DAV:\">"
    "   <D:prop>"
    "     <D:resourcetype/>"
    "     <D:getcontentlength/>"
    "     <D:getlastmodified/>"
    "     <D:creationdate/>"
    "     <D:displayname/>"
    "    </D:prop>"
    "  </D:propfind>");

  if (!dav.Open(url))
  {
    CLog::Log(LOGERROR, "%s - Unable to get dav directory (%s)", __FUNCTION__, strPath.c_str());
    return false;
  }

  dav.ReadData(strResponse);
  davResponse.Parse(strResponse.c_str());

  if (!strResponse)
  {
    CLog::Log(LOGERROR, "%s - Failed to get any response", __FUNCTION__);
  } 

  // Iterate over all responses
  for ( pChild = davResponse.RootElement()->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) 
  {
    if (ValueWithoutNamespace(pChild, "response"))
    {
      CFileItemPtr pItem(new CFileItem());

      if (ParseResponse(pChild->ToElement(), *pItem))
      {
        CURL url2(strPath);
        CURL url3(pItem->m_strPath);

        CUtil::AddFileToFolder(url2.GetWithoutFilename(), url3.GetFileName(), pItem->m_strPath);

        if (pItem->GetLabel().IsEmpty())
        {
          CStdString name(pItem->m_strPath);
          CUtil::RemoveSlashAtEnd(name);
          CUtil::URLDecode(name);
          pItem->SetLabel(CUtil::GetFileName(name));
        }

        if (pItem->m_bIsFolder)
          CUtil::AddSlashAtEnd(pItem->m_strPath);

        // Add back protocol options
        if (!url2.GetProtocolOptions().IsEmpty())
          pItem->m_strPath = pItem->m_strPath + "|" + url2.GetProtocolOptions();

        if (!pItem->m_strPath.Equals(strPath))
          items.Add(pItem);
        }
      }
    }

  dav.Close();

  return true;
}
