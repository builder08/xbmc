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

#ifdef HAS_DS_PLAYER

#include "dsconfig.h"
#include "Utils/log.h"
#include "DShowUtil/DShowUtil.h"
#include "CharsetConverter.h"
#include "Filters/ffdshow_constants.h"
//Required for the gui buttons
#include "XMLUtils.h"
#include "FileSystem/SpecialProtocol.h"

#include "GuiSettings.h"
#include "FGLoader.h"
#include "Filters/VMR9AllocatorPresenter.h"

#include "DShowUtil/smartptr.h"

using namespace std;

class CDSConfig g_dsconfig;

CDSConfig::CDSConfig(void)
{
  pIffdshowDecoder = NULL;
  m_pStrDXVA = "";
}

CDSConfig::~CDSConfig(void)
{
}

void CDSConfig::ClearConfig()
{
  if (CFile::Exists("special://temp//dslang.xml", false))
    CFile::Delete("special://temp//dslang.xml");

  m_pStrDXVA = "";
  pIffdshowDecoder = NULL;
  m_pPropertiesFilters.clear();
}
HRESULT CDSConfig::ConfigureFilters()
{
  HRESULT hr = S_OK;

  BeginEnumFilters(g_dsGraph->pFilterGraph, pEF, pBF)
  {
    GetffdshowFilters(pBF);
    LoadPropertiesPage(pBF);

  }
  EndEnumFilters
  CreatePropertiesXml();

  return hr;
}

bool CDSConfig::LoadPropertiesPage(IBaseFilter *pBF)
{
  if ((pBF == CFGLoader::Filters.AudioRenderer.pBF && CFGLoader::Filters.AudioRenderer.guid != CLSID_ReClock)
    || pBF == CFGLoader::Filters.VideoRenderer.pBF )
    return false;

  Com::SmartQIPtr<ISpecifyPropertyPages> pProp = pBF;
  CAUUID pPages;
  if ( pProp )
  {
    pProp->GetPages(&pPages);
    if (pPages.cElems > 0)
    {
      m_pPropertiesFilters.push_back(pBF);
    
      CStdString filterName;
      g_charsetConverter.wToUTF8(DShowUtil::GetFilterName(pBF), filterName);
      CLog::Log(LOGNOTICE, "%s \"%s\" expose ISpecifyPropertyPages", __FUNCTION__, filterName.c_str());
    }
    CoTaskMemFree(pPages.pElems);
    return true;
    
  } 
  else
    return false;
}

void CDSConfig::CreatePropertiesXml()
{
  //verify we have at least one property page
  if (m_pPropertiesFilters.empty())
    return;

  CStdString pStrName;
  CStdString pStrId;
  int pIntId = 0;
  TiXmlDocument xmlDoc;
  TiXmlElement xmlRootElement("strings");
  TiXmlNode *pRoot = xmlDoc.InsertEndChild(xmlRootElement);
  if (!pRoot) 
    return;
  
  for (std::vector<IBaseFilter *>::const_iterator it = m_pPropertiesFilters.begin() ; it != m_pPropertiesFilters.end(); it++)
  {
    g_charsetConverter.wToUTF8(DShowUtil::GetFilterName(*it), pStrName);
    TiXmlElement newFilterElement("string");
    
    //Set the id of the lang
    pStrId.Format("%i", pIntId);
    newFilterElement.SetAttribute("id", pStrId.c_str());

    //set the name of the filter in the element
    TiXmlNode *pNewNode = pRoot->InsertEndChild(newFilterElement);
    if (! pNewNode)
      break;
    
    TiXmlText value(pStrName.c_str());
    pNewNode->InsertEndChild(value);
    pIntId++;
  }
  xmlDoc.SaveFile("special://temp//dslang.xml");
}

void CDSConfig::ShowPropertyPage(IBaseFilter *pBF)
{
  m_pCurrentProperty = new CDSPropertyPage(pBF);
  m_pCurrentProperty->Initialize();
}


CStdString CDSConfig::GetDXVAMode()
{
  return m_pStrDXVA;
}
void CDSConfig::SetDXVAGuid( const GUID& dxvaguid )
{
  // IPinHook is calling the SetDxvaGuid once

  if (dxvaguid == GUID_NULL)
    m_pStrDXVA = "";
  else
    m_pStrDXVA.Format("%s",DShowUtil::GetDXVAMode(&dxvaguid));
}

bool CDSConfig::GetffdshowFilters(IBaseFilter* pBF)
{
  HRESULT hr;

  if (!pIffdshowDecoder)
    hr = pBF->QueryInterface(IID_IffDecoder, (void **) &pIffdshowDecoder );
  
  return true;
}

bool CDSConfig::LoadffdshowSubtitles(CStdString filePath)
{
  if (pIffdshowDecoder)
  {
    if (SUCCEEDED(pIffdshowDecoder->compat_putParamStr(IDFF_subFilename, filePath.c_str())))
      return true;
  }
  
  return false;
}

#endif