/*
 *      Copyright (C) 2005-2008 Team XBMC
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

//
// GeminiServer
//
#include "stdafx.h"
#include "TuxBoxUtil.h"
#include "Util.h"
#include "FileSystem/FileCurl.h"
#include "GUIDialogContextMenu.h"
#include "Application.h"
#include "GUIInfoManager.h"
#include "VideoInfoTag.h"
#include "GUIWindowManager.h"
#include "GUIDialogOK.h"
#include "GUIDialogYesNo.h"
#include "FileSystem/File.h"
#include "URL.h"
#include "Settings.h"
#include "FileItem.h"

using namespace XFILE;

CTuxBoxUtil g_tuxbox;
CTuxBoxService g_tuxboxService;

CTuxBoxService::CTuxBoxService()
{
}
CTuxBoxService::~CTuxBoxService()
{
}
CTuxBoxUtil::CTuxBoxUtil(void)
{
}
CTuxBoxUtil::~CTuxBoxUtil(void)
{
}
bool CTuxBoxService::Start()
{
  if(g_advancedSettings.m_iTuxBoxEpgRequestTime != 0)
  {
    StopThread();
    Create(false);
    return true;
  }
  else
    return false;
}
void CTuxBoxService::Stop()
{
  CLog::Log(LOGDEBUG, "%s - Stopping CTuxBoxService thread", __FUNCTION__);
  StopThread();
}
void CTuxBoxService::OnStartup()
{
  CLog::Log(LOGDEBUG, "%s - Starting CTuxBoxService thread", __FUNCTION__);
  SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_LOWEST);
}
void CTuxBoxService::OnExit()
{
  CThread::m_bStop = true;
}
bool CTuxBoxService::IsRunning()
{
  return !CThread::m_bStop;
}
void CTuxBoxService::Process()
{
  CStdString strCurrentServiceName = g_tuxbox.sCurSrvData.service_name;
  CStdString strURL;

  while(!CThread::m_bStop && g_application.IsPlaying())
  {
    strURL = g_application.CurrentFileItem().m_strPath;
    if(!CUtil::IsTuxBox(strURL))
      break;

    int iRequestTimer = g_advancedSettings.m_iTuxBoxEpgRequestTime *1000; //seconds
    Sleep(iRequestTimer);

    CURL url(strURL);
    if(g_tuxbox.GetHttpXML(url,"currentservicedata"))
    {
      CLog::Log(LOGDEBUG, "%s - receive current service data was successful", __FUNCTION__);
      if(!strCurrentServiceName.IsEmpty()&& 
        !strCurrentServiceName.Equals("NULL") &&
        !g_tuxbox.sCurSrvData.service_name.IsEmpty() &&
        !g_tuxbox.sCurSrvData.service_name.Equals("-") &&
        !g_tuxbox.vVideoSubChannel.mode)
      {
        //Detect Channel Change
        //We need to detect the channel on the TuxBox Device! 
        //On changing the channel on the device we will loose the stream and mplayer seems not able to detect it to stop
        if (strCurrentServiceName != g_tuxbox.sCurSrvData.service_name && g_application.IsPlaying())
        {
          CLog::Log(LOGDEBUG," - ERROR: Non controlled channel change detected! Stopping current playing stream!");
          g_applicationMessenger.MediaStop();
          break;
        }
      }
      //Update infomanager from tuxbox client
      g_infoManager.UpdateFromTuxBox();
    }
    else
    {
      CLog::Log(LOGDEBUG, "%s - Could not receive current service data", __FUNCTION__);
    }
  }
  return;
}
bool CTuxBoxUtil::CreateNewItem(const CFileItem& item, CFileItem& item_new)
{
  //Build new Item
  item_new.SetLabel(item.GetLabel());
  item_new.m_strPath = item.m_strPath;
  item_new.SetThumbnailImage(item.GetThumbnailImage());
  
  if(g_tuxbox.GetZapUrl(item.m_strPath, item_new))
  {
    if(vVideoSubChannel.mode)
      vVideoSubChannel.current_name = item_new.GetLabel()+" ("+vVideoSubChannel.current_name+")";
    return true;
  }
  else
  {
    if(!sBoxStatus.recording.Equals("1")) //Don't Show this Dialog, if the Box is in Recording mode! A previos YN Dialog was send to user!
    {
      CLog::Log(LOGDEBUG, "%s ---------------------------------------------------------", __FUNCTION__);
      CLog::Log(LOGDEBUG, "%s - WARNING: Zaping Failed no Zap Point found!", __FUNCTION__);
      CLog::Log(LOGDEBUG, "%s ---------------------------------------------------------", __FUNCTION__);
      CGUIDialogOK *dialog = (CGUIDialogOK *)m_gWindowManager.GetWindow(WINDOW_DIALOG_OK);
      if (dialog)
      {
        CStdString strText;
        strText.Format(g_localizeStrings.Get(21334).c_str(), item.GetLabel());
        dialog->SetHeading( 21331 );
        dialog->SetLine( 0, strText);
        dialog->SetLine( 1, 21333 );
        dialog->SetLine( 2, "" );
        dialog->DoModal();
      }
    }
  }
  return false;
}
bool CTuxBoxUtil::ParseBouquets(TiXmlElement *root, CFileItemList &items, CURL &url, CStdString strFilter, CStdString strChild)
{
  //
  //strChild.Format("bouquet");
  //

  CStdString strItemName, strItemPath, strOptions, strPort;
  TiXmlElement *pRootElement =root;
  TiXmlNode *pNode = NULL;
  TiXmlNode *pIt = NULL;
  items.m_idepth =1;
  // Get Options
  strOptions = url.GetOptions();
  // Detect Port
  if (url.GetPort()!=0 && url.GetPort()!=80)
  {
    strPort.Format(":%i",url.GetPort());
  }
  else 
  {
    strPort = "";
  }

  if (!pRootElement)
  {    
    CLog::Log(LOGWARNING, "%s - No %s found", __FUNCTION__, strChild.c_str());
    return false;
  }
  if (strFilter.IsEmpty())
  {
    pNode = pRootElement->FirstChild(strChild.c_str());
    if (!pNode)
    {
      CLog::Log(LOGWARNING, "%s - No %s found", __FUNCTION__,strChild.c_str());
      return false;
    }
    while(pNode)
    {
        pIt = pNode->FirstChild("name");
        if (pIt)
        {
          strItemName = pIt->FirstChild()->Value();
          
          pIt = pNode->FirstChild("reference");
          if (pIt)
          {
            strItemPath = pIt->FirstChild()->Value();
            // add. bouquets to item list! 
            CFileItem* pItem = new CFileItem;
            pItem->m_bIsFolder = true;
            pItem->SetLabel(strItemName);
            url.SetOptions("/"+strOptions+"&reference="+strItemPath);
            pItem->m_strPath = "tuxbox://"+url.GetUserName()+":"+url.GetPassWord()+"@"+url.GetHostName()+strPort+url.GetOptions();  
            items.Add(pItem);
            //DEBUG Log
            CLog::Log(LOGDEBUG, "%s - Name:    %s", __FUNCTION__,strItemName.c_str());
            CLog::Log(LOGDEBUG, "%s - Adress:  %s", __FUNCTION__,pItem->m_strPath.c_str());
          }
        }
        pNode = pNode->NextSibling(strChild.c_str());
    }
  }
  return true;
}

bool CTuxBoxUtil::ParseChannels(TiXmlElement *root, CFileItemList &items, CURL &url, CStdString strFilter, CStdString strChild)
{
  //
  //strChild.Format("bouquet");
  //

  CStdString strItemName, strItemPath,strPort;
  TiXmlElement *pRootElement =root;
  TiXmlNode *pNode = NULL;
  TiXmlNode *pIt = NULL;
  TiXmlNode *pIta = NULL;
  items.m_idepth =2;

  //Detect Port
  if (url.GetPort()!=0 && url.GetPort()!=80)
  {
    strPort.Format(":%i",url.GetPort());
  }
  else 
  {
    strPort = "";
  }

  if (!pRootElement)
  {    
    CLog::Log(LOGWARNING, "%s - No %ss found", __FUNCTION__,strChild.c_str());
    return false;
  }
  if(!strFilter.IsEmpty())
  {
    pNode = pRootElement->FirstChild(strChild.c_str());
    if (!pNode)
    {
      CLog::Log(LOGWARNING, "%s - No %s found", __FUNCTION__,strChild.c_str());
      return false;
    }
    while(pNode)
    {
        pIt = pNode->FirstChild("name");
        if (pIt)
        {
          strItemName = pIt->FirstChild()->Value();
          
          pIt = pNode->FirstChild("reference");
          if (strFilter.Equals(pIt->FirstChild()->Value()))
          {
            pIt = pNode->FirstChild("service");
            if (!pIt)
            {
              CLog::Log(LOGWARNING, "%s - No service found", __FUNCTION__);
              return false;
            }
            while(pIt)
            {
                pIta = pIt->FirstChild("name");
                if (pIta)
                {
                  strItemName = pIta->FirstChild()->Value();
                  
                  pIta = pIt->FirstChild("reference");
                  if (pIta)
                  {
                    strItemPath = pIta->FirstChild()->Value();
                    // channel listing add. to item list!
                    CFileItem* pbItem = new CFileItem;
                    pbItem->m_bIsFolder = false;
                    pbItem->SetLabel(strItemName);
                    pbItem->m_strPath = "tuxbox://"+url.GetUserName()+":"+url.GetPassWord()+"@"+url.GetHostName()+strPort+"/cgi-bin/zapTo?path="+strItemPath+".ts";  
                    pbItem->SetContentType("video/x-ms-asf");
                    //pbItem->SetThumbnailImage(GetPicon(strItemName)); //Set Picon Image

                    //DEBUG Log
                    CLog::Log(LOGDEBUG, "%s - Name:    %s", __FUNCTION__,strItemName.c_str());
                    CLog::Log(LOGDEBUG, "%s - Adress:  %s", __FUNCTION__,pbItem->m_strPath.c_str());
                    
                    //add to the list
                    items.Add(pbItem);
                  }
                }
                pIt = pIt->NextSibling("service");
            }
          }
        }
        pNode = pNode->NextSibling(strChild.c_str());
    }
    return true;
  }
  return false;
}
bool CTuxBoxUtil::ZapToUrl(CURL url, CStdString strOptions, int ipoint) 
{
  // send Zap 
  CStdString strZapUrl, strPostUrl, strZapName, strFilter;
  //Extract the ZAP to Service String
  strFilter = strOptions.Right((strOptions.size()-(ipoint+6)));
  //Remove the ".ts"
  strFilter = strFilter.Left(strFilter.size()-3);
  //Get the Service Name
  strZapName = url.GetFileNameWithoutPath();

  // Detect Port and Create ZAP URL
  if (url.GetPort()!=0 && url.GetPort()!=80)
    strZapUrl.Format("http://%s:%s@%s:%i",url.GetUserName().c_str(),url.GetPassWord().c_str(),url.GetHostName().c_str(),url.GetPort());
  else
    strZapUrl.Format("http://%s:%s@%s",url.GetUserName().c_str(),url.GetPassWord().c_str(),url.GetHostName().c_str());
  strPostUrl.Format("/cgi-bin/zapTo?path=%s",strFilter.c_str());

  //Set Zap URL
  CURL urlx(strZapUrl);

  //Check Recording State!
  if(GetHttpXML(urlx,"boxstatus"))
  {
    if(sBoxStatus.recording.Equals("1"))
    {
      CLog::Log(LOGDEBUG, "%s ---------------------------------------------------------", __FUNCTION__);
      CLog::Log(LOGDEBUG, "%s - WARNING: Device is Recording! Record Mode is: %s", __FUNCTION__,sBoxStatus.recording.c_str());
      CLog::Log(LOGDEBUG, "%s ---------------------------------------------------------", __FUNCTION__);
      CGUIDialogYesNo* dialog = (CGUIDialogYesNo*)m_gWindowManager.GetWindow(WINDOW_DIALOG_YES_NO);
      if (dialog)
      {
        //Target TuxBox is in Recording mode! Are you sure to stream ?YN
        dialog->SetHeading(21331);
        dialog->SetLine( 0, 21332);
        dialog->SetLine( 1, 21335);
        dialog->SetLine( 2, "" );
        dialog->DoModal();
      }
      if (!dialog->IsConfirmed())
      {
        //DialogYN = NO -> Return false!
        return false;
      }
    }
  }
    
  //Send ZAP Command
  CFileCurl http;
  if(http.Open(strZapUrl+strPostUrl, false))
  {
    //DEBUG LOG
    CLog::Log(LOGDEBUG, "%s - Zapped to: %s (%s)", __FUNCTION__,strZapName.c_str(),strZapUrl.c_str());
    CLog::Log(LOGDEBUG, "%s - Zap String: %s", __FUNCTION__,strPostUrl.c_str());

    //Request StreamInfo
    GetHttpXML(urlx,"streaminfo");
    
    //Extract StreamInformations
    int iRetry=0;
    //PMT must be a valid value to be sure that the ZAP is OK and we can stream!
    while(sStrmInfo.pmt.Equals("ffffffffh") && iRetry!=10) //try 10 Times
    {
      CLog::Log(LOGDEBUG, "%s - Requesting STREAMINFO! TryCount: %i!", __FUNCTION__,iRetry);
      GetHttpXML(urlx,"streaminfo");
      iRetry=iRetry+1;
    }
    
    // PMT Not Valid? Try Time 10 reached, checking for advancedSettings m_iTuxBoxZapWaitTime
    if(sStrmInfo.pmt.Equals("ffffffffh") && g_advancedSettings.m_iTuxBoxZapWaitTime > 0 )
    {
      iRetry = 0;
      CLog::Log(LOGDEBUG, "%s - Starting TuxBox ZapWaitTimer!", __FUNCTION__);
      while(sStrmInfo.pmt.Equals("ffffffffh") && iRetry!=10) //try 10 Times
      {
        CLog::Log(LOGDEBUG, "%s - Requesting STREAMINFO! TryCount: %i!", __FUNCTION__,iRetry);
        GetHttpXML(urlx,"streaminfo");
        iRetry=iRetry+1;
        if(sStrmInfo.pmt.Equals("ffffffffh"))
        {
          CLog::Log(LOGERROR, "%s - STREAMINFO ERROR! Could not receive all data, TryCount: %i!", __FUNCTION__,iRetry);
          CLog::Log(LOGERROR, "%s - PMT is: %s (not a Valid Value)! Waiting %i sec.", __FUNCTION__,sStrmInfo.pmt.c_str(), g_advancedSettings.m_iTuxBoxZapWaitTime);
          Sleep(g_advancedSettings.m_iTuxBoxZapWaitTime*1000);
        }
      }
    }
    
    //PMT Failed! No StreamInformations availible.. closing stream 
    if (sStrmInfo.pmt.Equals("ffffffffh"))
    {
      CLog::Log(LOGERROR, "%s-------------------------------------------------------------", __FUNCTION__);
      CLog::Log(LOGERROR, "%s - STREAMINFO ERROR! Could not receive all data, TryCount: %i!", __FUNCTION__,iRetry);
      CLog::Log(LOGERROR, "%s - PMT is: %s (not a Valid Value)! There is nothing to Stream!", __FUNCTION__,sStrmInfo.pmt.c_str());
      CLog::Log(LOGERROR, "%s - The Stream will stopped!", __FUNCTION__);
      CLog::Log(LOGERROR, "%s-------------------------------------------------------------", __FUNCTION__);
      return false;
    }
    //Currentservicedata
    GetHttpXML(urlx,"currentservicedata");
    //boxstatus
    GetHttpXML(urlx,"boxstatus");
    //boxinfo
    GetHttpXML(urlx,"boxinfo");
    //serviceepg
    GetHttpXML(urlx,"serviceepg");
    return true;
  }
  return false;
}
bool CTuxBoxUtil::GetZapUrl(const CStdString& strPath, CFileItem &items )
{
  CURL url(strPath);
  CStdString strOptions = url.GetOptions();
  if (strOptions.IsEmpty())
    return false;

  int ipoint = strOptions.Find("?path=");
  if (ipoint >=0)
  {
    if(ZapToUrl(url, strOptions, ipoint))
    {
      //Check VideoSubChannels
      if(GetHttpXML(url,"currentservicedata")) //Update Currentservicedata
      {
        //Detect VideoSubChannels
        CStdString strVideoSubChannelName, strVideoSubChannelPID;
        if(GetVideoSubChannels(strVideoSubChannelName,strVideoSubChannelPID ))
        {
          // new videosubchannel selected! settings options to new video zap id
          strOptions = "?path="+strVideoSubChannelPID+".ts";
          // zap again now to new videosubchannel
          if(ZapToUrl(url, strOptions, ipoint))
          {
            vVideoSubChannel.mode = true;
            vVideoSubChannel.current_name = strVideoSubChannelName;
          }
        }
        else 
          vVideoSubChannel.mode= false;
      }

      CStdString strStreamURL, strVideoStream;
      CStdString strLabel, strLabel2;
      CStdString strAudioChannelName, strAudioChannelPid;
      
      if (GetAudioChannels(strAudioChannelName, strAudioChannelPid))
      {
        if(strAudioChannelPid.Left(2).Equals("0x"))
          strAudioChannelPid.Replace("0x","");

        strVideoStream.Format("0,%s,%s,%s,,,%s",sStrmInfo.pmt.Left(4).c_str(), sStrmInfo.vpid.Left(4).c_str(), strAudioChannelPid.Left(4).c_str(),sStrmInfo.pcrpid.Left(4).c_str());
      }
      else
      {
        if(sStrmInfo.apid.Left(2).Equals("0x")) 
          sStrmInfo.apid.Replace("0x","");
        if(sCurSrvData.audio_channel_1_pid.Left(2).Equals("0x"))
          sCurSrvData.audio_channel_1_pid.Replace("0x","");
        if(sCurSrvData.audio_channel_2_pid.Left(2).Equals("0x"))
          sCurSrvData.audio_channel_2_pid.Replace("0x","");

        if(g_application.m_eForcedNextPlayer == EPC_DVDPLAYER || g_advancedSettings.m_bTuxBoxSendAllAPids)
          strVideoStream.Format("0,%s,%s,%s,%s,%s,%s",sStrmInfo.pmt.Left(4).c_str(), sStrmInfo.vpid.Left(4).c_str(), sStrmInfo.apid.Left(4).c_str(), sCurSrvData.audio_channel_1_pid.Left(4).c_str(), sCurSrvData.audio_channel_2_pid.Left(4).c_str(), sStrmInfo.pcrpid.Left(4).c_str());
        else 
          strVideoStream.Format("0,%s,%s,%s,,,%s",sStrmInfo.pmt.Left(4).c_str(), sStrmInfo.vpid.Left(4).c_str(), sStrmInfo.apid.Left(4).c_str(), sStrmInfo.pcrpid.Left(4).c_str());
      }
      
      //strStreamURL.Format("http://%s:%i/%s",url.GetHostName().c_str(),TS_STREAM_PORT,strVideoStream.c_str());
      strStreamURL.Format("http://%s:%s@%s:%i/%s",url.GetUserName().c_str(),url.GetPassWord().c_str(), url.GetHostName().c_str(),TS_STREAM_PORT,strVideoStream.c_str());
      strLabel.Format("%s: %s %s-%s",items.GetLabel().c_str(), sCurSrvData.current_event_date.c_str(),sCurSrvData.current_event_start.c_str(), sCurSrvData.current_event_start.c_str());
      strLabel2.Format("%s", sCurSrvData.current_event_description.c_str());
      
      // Set Event details
      CStdString strGenre, strTitle;
      strGenre.Format("%s %s  -  (%s: %s)",g_localizeStrings.Get(143),sCurSrvData.current_event_description, g_localizeStrings.Get(209),sCurSrvData.next_event_description);
      strTitle.Format("%s",sCurSrvData.current_event_details);
      int iDuration = atoi(sCurSrvData.current_event_duration.c_str());
      
      items.GetVideoInfoTag()->m_strGenre = strGenre;  // VIDEOPLAYER_GENRE: current_event_description (Film Name)
      items.GetVideoInfoTag()->m_strTitle = strTitle; // VIDEOPLAYER_TITLE: current_event_details     (Film beschreibung)
      StringUtils::SecondsToTimeString(iDuration,items.GetVideoInfoTag()->m_strRuntime); //VIDEOPLAYER_DURATION: current_event_duration (laufzeit in sec.)
      
      //
      items.m_strPath = strStreamURL;
      items.m_iDriveType = url.GetPort(); // Dirty Hack! But i need to hold the Port ;)
      items.SetLabel(items.GetLabel()); // VIDEOPLAYER_DIRECTOR: service_name (Program Name)
      items.SetLabel2(sCurSrvData.current_event_description); // current_event_description (Film Name)
      items.m_bIsFolder = false;
      items.SetContentType("video/x-ms-asf");
      
      return true;
    }
  }
  return false;
}

bool CTuxBoxUtil::GetHttpXML(CURL url,CStdString strRequestType)
{
  // Check and Set URL Request Option
  if(!strRequestType.IsEmpty())
  {
    if(strRequestType.Equals("streaminfo"))
    {
      url.SetOptions("xml/streaminfo");
    }
    else if(strRequestType.Equals("currentservicedata"))
    {
      url.SetOptions("xml/currentservicedata");
    }
    else if(strRequestType.Equals("boxstatus"))
    {
      url.SetOptions("xml/boxstatus");
    }
    else if(strRequestType.Equals("boxinfo"))
    {
      url.SetOptions("xml/boxinfo");
    }
    else if(strRequestType.Equals("serviceepg"))
    {
      url.SetOptions("xml/serviceepg");
    }
    else 
    {
      CLog::Log(LOGERROR, "%s - Request Type is not defined! You requested: %s", __FUNCTION__,strRequestType.c_str());
      return false;
    }
  }
  else
  {
    CLog::Log(LOGERROR, "%s - strRequestType Request Type is Empty!", __FUNCTION__);
    return false;
  }
  
  // Clean Up the URL, so we have a clean request!
  url.SetFileName("");
  
  //Open 
  CFileCurl http;
  http.SetTimeout(20);
  if(http.Open(url, false)) 
  {
    CStdString strTmp;
    int size_read = 0;  
    int data_size = 0;
    int size_total = (int)http.GetLength();
        
    if(size_total >0)
    {
      // read response from server into string buffer
      strTmp.reserve(size_total);
      char buffer[16384];
      while( (size_read = http.Read( buffer, sizeof(buffer)-1) ) > 0 )
      {
        buffer[size_read] = 0;
        strTmp += buffer;
        data_size += size_read;
      }

      // parse returned xml
      TiXmlDocument doc;
      TiXmlElement *XMLRoot=NULL;
      strTmp.Replace("></",">-</"); //FILL EMPTY ELEMENTS WITH "-"!
      doc.Parse(strTmp.c_str());
      strTmp.Empty();

      XMLRoot = doc.RootElement();
      CStdString strRoot = XMLRoot->Value();
      if( strRoot.Equals("streaminfo"))
      {
        return StreamInformations(XMLRoot);
      }
      else if(strRoot.Equals("currentservicedata"))
      {
        return CurrentServiceData(XMLRoot);
      }
      else if(strRoot.Equals("boxstatus"))
      {
        return BoxStatus(XMLRoot);
      }
      else if(strRoot.Equals("boxinfo"))
      {
        return BoxInfo(XMLRoot);
      }
      else if(strRoot.Equals("serviceepg") || strRoot.Equals("service_epg"))
      {
        return ServiceEPG(XMLRoot);
      }
      else
      {
        CLog::Log(LOGERROR, "%s - Unable to parse xml", __FUNCTION__);
        CLog::Log(LOGERROR, "%s - Request String: %s", __FUNCTION__,strRoot.c_str());
      }
      return false;
    }
    else
    {
      CLog::Log(LOGERROR, "%s - http length is invalid!", __FUNCTION__);
      return false;
    }
  }
  CLog::Log(LOGERROR, "%s - Open URL Failed! Unable to get XML structure", __FUNCTION__);
  return false;
}
bool CTuxBoxUtil::StreamInformations(TiXmlElement *pRootElement)
{
  /*
  Sample:
  http://192.168.0.110:31339/0,0065,01ff,0200,0201,0203,01ff

  http://getIP:31339/0,pmtpid,vpid,apid,apids,apids,pcrpid;

  vpid,pmtpid,pcrpid,apid  --> xml/streaminfo
  apids --> /xml/currentservicedata
  
  apid: is the defined audio stream!
  Normal Stereo: http://192.168.0.110:31339/0,0065,01ff,0200,0201,0203,01ff
  Normal English: http://192.168.0.110:31339/0,0065,01ff,0201,,,01ff
  Normal DD5.1/AC3: http://192.168.0.110:31339/0,0065,01ff,0203,,,01ff
  */

  TiXmlNode *pNode = NULL;
  TiXmlNode *pIt = NULL;
  CStdString strRoot = pRootElement->Value();
  if(pRootElement !=NULL)
  {
    pNode = pRootElement->FirstChild("frontend");
    if (pNode)
    {
      sStrmInfo.frontend = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Frontend: %s", __FUNCTION__, sStrmInfo.frontend.c_str());
    }
    pNode = pRootElement->FirstChild("service");
    if (pNode)
    {
      CLog::Log(LOGDEBUG, "%s - Service", __FUNCTION__);
      pIt = pNode->FirstChild("name");
      if (pIt)
      {
        sStrmInfo.service_name = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s - Name: %s", __FUNCTION__, sStrmInfo.service_name.c_str());
      }
      pIt = pNode->FirstChild("reference");
      if (pIt)
      {
        sStrmInfo.service_reference = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s - Reference: %s", __FUNCTION__, sStrmInfo.service_reference.c_str());
      }
    }

    pNode = pRootElement->FirstChild("provider");
    if(pNode && pNode->FirstChild())
    {
      sStrmInfo.provider= pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Provider: %s", __FUNCTION__, sStrmInfo.provider.c_str());
    }
    pNode = pRootElement->FirstChild("vpid");
    if (pNode)
    {
      sStrmInfo.vpid = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Vpid: %s", __FUNCTION__, sStrmInfo.vpid.c_str());
    }
    pNode = pRootElement->FirstChild("apid");
    if (pNode)
    {
      sStrmInfo.apid = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Apid: %s", __FUNCTION__, sStrmInfo.apid.c_str());
    }
    pNode = pRootElement->FirstChild("pcrpid");
    if (pNode)
    {
      sStrmInfo.pcrpid = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - PcrPid: %s", __FUNCTION__, sStrmInfo.pcrpid.c_str());
    }
    pNode = pRootElement->FirstChild("tpid");
    if (pNode)
    {
      sStrmInfo.tpid = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Tpid: %s", __FUNCTION__, sStrmInfo.tpid.c_str());
    }
    pNode = pRootElement->FirstChild("tsid");
    if (pNode)
    {
      sStrmInfo.tsid = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Tsid: %s", __FUNCTION__, sStrmInfo.tsid.c_str());
    }
    pNode = pRootElement->FirstChild("onid");
    if (pNode)
    {
      sStrmInfo.onid = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Onid: %s", __FUNCTION__, sStrmInfo.onid.c_str());
    }
    pNode = pRootElement->FirstChild("sid");
    if (pNode)
    {
      sStrmInfo.sid = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Sid: %s", __FUNCTION__, sStrmInfo.sid.c_str());
    }
    pNode = pRootElement->FirstChild("pmt");
    if (pNode)
    {
      sStrmInfo.pmt = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Pmt: %s", __FUNCTION__, sStrmInfo.pmt.c_str());
    }
    pNode = pRootElement->FirstChild("video_format");
    if (pNode)
    {
      sStrmInfo.video_format = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Video Format: %s", __FUNCTION__, sStrmInfo.video_format.c_str());
    }
    pNode = pRootElement->FirstChild("supported_crypt_systems");
    if (pNode)
    {
      sStrmInfo.supported_crypt_systems = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Supported Crypt Systems: %s", __FUNCTION__, sStrmInfo.supported_crypt_systems.c_str());
    }
    pNode = pRootElement->FirstChild("used_crypt_systems");
    if (pNode)
    {
      sStrmInfo.used_crypt_systems = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Used Crypt Systems: %s", __FUNCTION__, sStrmInfo.used_crypt_systems.c_str());
    }
    pNode = pRootElement->FirstChild("satellite");
    if (pNode)
    {
      sStrmInfo.satellite = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Satellite: %s", __FUNCTION__, sStrmInfo.satellite.c_str());
    }
    pNode = pRootElement->FirstChild("frequency");
    if (pNode)
    {
      sStrmInfo.frequency = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Frequency: %s", __FUNCTION__, sStrmInfo.frequency.c_str());
    }
    pNode = pRootElement->FirstChild("symbol_rate");
    if (pNode)
    {
      sStrmInfo.symbol_rate = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Symbol Rate: %s", __FUNCTION__, sStrmInfo.symbol_rate.c_str());
    }
    pNode = pRootElement->FirstChild("polarisation");
    if (pNode)
    {
      sStrmInfo.polarisation = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Polarisation: %s", __FUNCTION__, sStrmInfo.polarisation.c_str());
    }
    pNode = pRootElement->FirstChild("inversion");
    if (pNode)
    {
      sStrmInfo.inversion = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Inversion: %s", __FUNCTION__, sStrmInfo.inversion.c_str());
    }
    pNode = pRootElement->FirstChild("fec");
    if (pNode)
    {
      sStrmInfo.fec = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Fec: %s", __FUNCTION__, sStrmInfo.fec.c_str());
    }
    pNode = pRootElement->FirstChild("snr");
    if (pNode)
    {
      sStrmInfo.snr = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Snr: %s", __FUNCTION__, sStrmInfo.snr.c_str());
    }
    pNode = pRootElement->FirstChild("agc");
    if (pNode)
    {
      sStrmInfo.agc = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Agc: %s", __FUNCTION__,  sStrmInfo.agc.c_str());
    }
    pNode = pRootElement->FirstChild("ber");
    if (pNode)
    {
      sStrmInfo.ber = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - ber: %s", __FUNCTION__, sStrmInfo.ber.c_str());
    }
    pNode = pRootElement->FirstChild("lock");
    if (pNode)
    {
      sStrmInfo.lock = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Lock: %s", __FUNCTION__, sStrmInfo.lock.c_str());
    }
    pNode = pRootElement->FirstChild("sync");
    if (pNode)
    {
      sStrmInfo.sync = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Sync: %s", __FUNCTION__, sStrmInfo.sync.c_str());
    }
    return true;
  }
  return false;
}
bool CTuxBoxUtil::CurrentServiceData(TiXmlElement *pRootElement)
{
  TiXmlNode *pNode = NULL;
  TiXmlNode *pIt = NULL;
  TiXmlNode *pVal = NULL;
  if(pRootElement)
  {
    CLog::Log(LOGDEBUG, "%s - Current Service Data", __FUNCTION__);
    pNode = pRootElement->FirstChild("service");
    if (pNode)
    {
      CLog::Log(LOGDEBUG, "%s - Service", __FUNCTION__);
      pIt = pNode->FirstChild("name");
      if (pIt)
      {
        sCurSrvData.service_name = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s Service Name: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }
      pIt = pNode->FirstChild("reference");
      if (pIt)
      {
        sCurSrvData.service_reference = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s Service Reference: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }
    }

    pNode = pRootElement->FirstChild("audio_channels");
    if (pNode)
    {
      CLog::Log(LOGDEBUG, "%s - Audio Channels", __FUNCTION__);
      int i = 0;
      pIt = pNode->FirstChild("channel");
      while(pIt)
      {
        pVal = pIt->FirstChild("pid");
        if(pVal)
        {
          if(i==0) sCurSrvData.audio_channel_0_pid = pVal->FirstChild()->Value();
          if(i==1) sCurSrvData.audio_channel_1_pid = pVal->FirstChild()->Value();
          if(i==2) sCurSrvData.audio_channel_2_pid = pVal->FirstChild()->Value();
        }
        
        pVal = pIt->FirstChild("selected");
        if(pVal)
        {
          if(i==0) sCurSrvData.audio_channel_0_selected = pVal->FirstChild()->Value();
          if(i==1) sCurSrvData.audio_channel_1_selected = pVal->FirstChild()->Value();
          if(i==2) sCurSrvData.audio_channel_2_selected = pVal->FirstChild()->Value();
        }
        
        pVal = pIt->FirstChild("name");
        if(pVal)
        {
          if(i==0) sCurSrvData.audio_channel_0_name = pVal->FirstChild()->Value();
          if(i==1) sCurSrvData.audio_channel_1_name = pVal->FirstChild()->Value();
          if(i==2) sCurSrvData.audio_channel_2_name = pVal->FirstChild()->Value();
        }

        if(i==0) CLog::Log(LOGDEBUG, "%s - Audio Channels: Channel %i -> PID: %s Selected: %s Name: %s", __FUNCTION__, i, sCurSrvData.audio_channel_0_pid.c_str(), sCurSrvData.audio_channel_0_selected.c_str(), sCurSrvData.audio_channel_0_name.c_str() );
        if(i==1) CLog::Log(LOGDEBUG, "%s - Audio Channels: Channel %i -> PID: %s Selected: %s Name: %s", __FUNCTION__, i, sCurSrvData.audio_channel_1_pid.c_str(), sCurSrvData.audio_channel_1_selected.c_str(), sCurSrvData.audio_channel_1_name.c_str() );
        if(i==2) CLog::Log(LOGDEBUG, "%s - Audio Channels: Channel %i -> PID: %s Selected: %s Name: %s", __FUNCTION__, i, sCurSrvData.audio_channel_2_pid.c_str(), sCurSrvData.audio_channel_2_selected.c_str(), sCurSrvData.audio_channel_2_name.c_str() );

        i=i+1;
        pIt = pIt->NextSibling("channel");
      }
    }
    pNode = pRootElement->FirstChild("audio_track");
    if (pNode)
    {
      sCurSrvData.audio_track = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Audio Track: %s", __FUNCTION__, pNode->FirstChild()->Value() );
    }
    pNode = pRootElement->FirstChild("video_channels");
    if (pNode)
    {
      CLog::Log(LOGDEBUG, "%s - Video Channels", __FUNCTION__);
      pIt = pNode->FirstChild("service");
      if (pIt)
      {
        vVideoSubChannel.name.clear();
        vVideoSubChannel.reference.clear();
        vVideoSubChannel.selected.clear();
        int i = 0;
        while(pIt)
        {
          pVal = pIt->FirstChild("name");
          if(pVal)
          {
            vVideoSubChannel.name.push_back(pVal->FirstChild()->Value());
            CLog::Log(LOGDEBUG, "%s - Video Sub Channel %i:      Name: %s", __FUNCTION__, i,pVal->FirstChild()->Value());
          }
          pVal = pIt->FirstChild("reference");
          if(pVal)
          {
            vVideoSubChannel.reference.push_back(pVal->FirstChild()->Value());
            CLog::Log(LOGDEBUG, "%s - Video Sub Channel %i: Reference: %s", __FUNCTION__, i,pVal->FirstChild()->Value());
          }
          pVal = pIt->FirstChild("selected");
          if(pVal)
          {
            vVideoSubChannel.selected.push_back(pVal->FirstChild()->Value());
            CLog::Log(LOGDEBUG, "%s - Video Sub Channel %i: Selected: %s", __FUNCTION__, i,pVal->FirstChild()->Value());
          }
          pIt = pIt->NextSibling("service");
          i++;
        }
      }
      else
      {
        vVideoSubChannel.name.clear();
        vVideoSubChannel.reference.clear();
        vVideoSubChannel.selected.clear();
      }
    }
    pNode = pRootElement->FirstChild("current_event");
    if (pNode)
    {
      CLog::Log(LOGDEBUG, "%s - Current Event", __FUNCTION__);
      pIt = pNode->FirstChild("date");
      if (pIt)
      {
        sCurSrvData.current_event_date = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s - Date: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }
      pIt = pNode->FirstChild("time");
      if (pIt)
      {
        sCurSrvData.current_event_time = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s - Time: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }

      pIt = pNode->FirstChild("start");
      if (pIt)
      {
        sCurSrvData.current_event_start = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s - Start: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }

      pIt = pNode->FirstChild("duration");
      if (pIt)
      {
        sCurSrvData.current_event_duration = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s - Duration: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }

      pIt = pNode->FirstChild("description");
      if (pIt)
      {
        sCurSrvData.current_event_description = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s - Description: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }
      pIt = pNode->FirstChild("details");
      if (pIt)
      {
        sCurSrvData.current_event_details = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s - Details: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }
    }
    pNode = pRootElement->FirstChild("next_event");
    if (pNode)
    {
      CLog::Log(LOGDEBUG, "%s - Next Event", __FUNCTION__);
      pIt = pNode->FirstChild("date");
      if (pIt)
      {
        sCurSrvData.next_event_date = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s - Date: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }
      pIt = pNode->FirstChild("time");
      if (pIt)
      {
        sCurSrvData.next_event_time = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s - Time: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }

      pIt = pNode->FirstChild("start");
      if (pIt)
      {
        sCurSrvData.next_event_start = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s - Start: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }

      pIt = pNode->FirstChild("duration");
      if (pIt)
      {
        sCurSrvData.next_event_duration = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s - Duration: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }

      pIt = pNode->FirstChild("description");
      if (pIt)
      {
        sCurSrvData.next_event_description = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s - Description: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }
      pIt = pNode->FirstChild("details");
      if (pIt)
      {
        sCurSrvData.next_event_details = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s - Details: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }
    }
    return true;
  }
  return false;

}
bool CTuxBoxUtil::BoxStatus(TiXmlElement *pRootElement)
{
  //Tuxbox Controll Commands
  /*
    /cgi-bin/admin?command=wakeup
    /cgi-bin/admin?command=standby
    /cgi-bin/admin?command=shutdown
    /cgi-bin/admin?command=reboot
    /cgi-bin/admin?command=restart
  */

  TiXmlNode *pNode = NULL;

  if(pRootElement)
  {
    CLog::Log(LOGDEBUG, "%s - BoxStatus", __FUNCTION__);
    pNode = pRootElement->FirstChild("current_time");
    if (pNode)
    {
      sBoxStatus.current_time = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Current Time: %s", __FUNCTION__, pNode->FirstChild()->Value());
    }
    pNode = pRootElement->FirstChild("standby");
    if (pNode)
    {
      sBoxStatus.standby = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Standby: %s", __FUNCTION__, pNode->FirstChild()->Value());
    }
    pNode = pRootElement->FirstChild("recording");
    if (pNode)
    {
      sBoxStatus.recording = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Recording: %s", __FUNCTION__, pNode->FirstChild()->Value());
    }
    pNode = pRootElement->FirstChild("mode");
    if (pNode)
    {
      sBoxStatus.mode = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Mode: %s", __FUNCTION__, pNode->FirstChild()->Value());
    }
    pNode = pRootElement->FirstChild("ip");
    if (pNode)
    {
      sBoxStatus.ip = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Ip: %s", __FUNCTION__, pNode->FirstChild()->Value());
    }
    return true;
  }
  return false;
}
bool CTuxBoxUtil::BoxInfo(TiXmlElement *pRootElement)
{
  TiXmlNode *pNode = NULL;
  TiXmlNode *pIt = NULL;
  
  if(pRootElement)
  {
    CLog::Log(LOGDEBUG, "%s - BoxInfo", __FUNCTION__);
    pNode = pRootElement->FirstChild("image");
    if (pNode)
    {
      CLog::Log(LOGDEBUG, "%s - Image", __FUNCTION__);
      pIt = pNode->FirstChild("version");
      if (pIt)
      {
        sBoxInfo.image_version = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s Image Version: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }
      pIt = pNode->FirstChild("url");
      if (pIt)
      {
        sBoxInfo.image_url = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s Image Url: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }
      pIt = pNode->FirstChild("comment");
      if (pIt)
      {
        sBoxInfo.image_comment = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s Image Comment: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }
      pIt = pNode->FirstChild("catalog");
      if (pIt)
      {
        sBoxInfo.image_catalog = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s Image Catalog: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }
    }
    pNode = pRootElement->FirstChild("firmware");
    if (pNode)
    {
      sBoxInfo.firmware = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Firmware: %s", __FUNCTION__, pNode->FirstChild()->Value());
    }
    pNode = pRootElement->FirstChild("fpfirmware");
    if (pNode)
    {
      sBoxInfo.fpfirmware = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - FP Firmware: %s", __FUNCTION__, pNode->FirstChild()->Value());
    }
    pNode = pRootElement->FirstChild("webinterface");
    if (pNode)
    {
      sBoxInfo.webinterface = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Web Interface: %s", __FUNCTION__, pNode->FirstChild()->Value());
    }
    pNode = pRootElement->FirstChild("model");
    if (pNode)
    {
      sBoxInfo.model = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Model: %s", __FUNCTION__, pNode->FirstChild()->Value());
    }
    pNode = pRootElement->FirstChild("manufacturer");
    if (pNode)
    {
      sBoxInfo.manufacturer = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Manufacturer: %s", __FUNCTION__, pNode->FirstChild()->Value());
    }
    pNode = pRootElement->FirstChild("processor");
    if (pNode)
    {
      sBoxInfo.processor = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Processor: %s", __FUNCTION__, pNode->FirstChild()->Value());
    }
    pNode = pRootElement->FirstChild("usbstick");
    if (pNode)
    {
      sBoxInfo.usbstick = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - USB Stick: %s", __FUNCTION__, pNode->FirstChild()->Value());
    }
    pNode = pRootElement->FirstChild("disk");
    if (pNode)
    {
      sBoxInfo.disk = pNode->FirstChild()->Value();
      CLog::Log(LOGDEBUG, "%s - Disk: %s", __FUNCTION__, pNode->FirstChild()->Value());
    }
    return true;
  }
  return false;
}
bool CTuxBoxUtil::ServiceEPG(TiXmlElement *pRootElement)
{
  TiXmlNode *pNode = NULL;
  TiXmlNode *pIt = NULL;
  
  if(pRootElement)
  {
    CLog::Log(LOGDEBUG, "%s - Service EPG", __FUNCTION__);
    pNode = pRootElement->FirstChild("service");
    if (pNode)
    {
      CLog::Log(LOGDEBUG, "%s - Service", __FUNCTION__);
      pIt = pNode->FirstChild("reference");
      if (pIt)
      {
        sServiceEPG.service_reference = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s - Service Reference: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }
      pIt = pNode->FirstChild("name");
      if (pIt)
      {
        sServiceEPG.service_name = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s - Service Name: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }
    }
    //Todo there is more then 1 event! Create a Event List!
    pNode = pRootElement->FirstChild("event");
    if (pNode)
    {
      CLog::Log(LOGDEBUG, "%s - Event", __FUNCTION__);
      pIt = pNode->FirstChild("date");
      if (pIt)
      {
        sServiceEPG.date = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s - Date: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }
      pIt = pNode->FirstChild("time");
      if (pIt)
      {
        sServiceEPG.time = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s - Time: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }
      pIt = pNode->FirstChild("duration");
      if (pIt)
      {
        sServiceEPG.duration = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s - Duration: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }
      pIt = pNode->FirstChild("descritption");
      if (pIt)
      {
        sServiceEPG.descritption = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s - Descritption: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }
      pIt = pNode->FirstChild("genre");
      if (pIt)
      {
        sServiceEPG.genre = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s - Genre: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }
      pIt = pNode->FirstChild("genrecategory");
      if (pIt)
      {
        sServiceEPG.genrecategory = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s - Genrecategory: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }
      pIt = pNode->FirstChild("start");
      if (pIt)
      {
        sServiceEPG.start = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s - Start: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }
      pIt = pNode->FirstChild("details");
      if (pIt)
      {
        sServiceEPG.details = pIt->FirstChild()->Value();
        CLog::Log(LOGDEBUG, "%s - Details: %s", __FUNCTION__, pIt->FirstChild()->Value());
      }
    }
    return true;
  }
  return false;
}
//PopUp and request the AudioChannel
//No PopUp: On 1x detected AudioChannel
bool CTuxBoxUtil::GetAudioChannels(CStdString& strAudioChannelName, CStdString& strAudioChannelPid)
{
  //DVDPlayer Can play all AudioStreams! No need to popup the AudioChannel selector!
  if(g_application.m_eForcedNextPlayer == EPC_DVDPLAYER)
  {
    CLog::Log(LOGDEBUG, "%s - DVDPlayer is used to play the Stream! Disabling Audio Channel Selection! Returning False to use All Possible Audio channels!", __FUNCTION__);
    return false;
  }
  // Audio Selection is Disabled! Return false to use default values!
  if(!g_advancedSettings.m_bTuxBoxAudioChannelSelection)
  {
    CLog::Log(LOGDEBUG, "%s - Audio Channel Selection is Disabled! Returning False to use the default values!", __FUNCTION__);
    return false;
  }
  // We have only one Audio Channel return false to use default values!
  if(sCurSrvData.audio_channel_1_name.IsEmpty() && sCurSrvData.audio_channel_2_name.IsEmpty())
    return false;

  // popup the context menu
  CGUIDialogContextMenu *pMenu;
  pMenu = (CGUIDialogContextMenu *)m_gWindowManager.GetWindow(WINDOW_DIALOG_CONTEXT_MENU);
  if (pMenu)
  {
    // load Audio context menu
    pMenu->Initialize();
    // add the needed Audio buttons
    int btn_AudioChannel_0=-1;
    if(!sCurSrvData.audio_channel_0_name.IsEmpty())
      btn_AudioChannel_0 = pMenu->AddButton(sCurSrvData.audio_channel_0_name); // A0

    int btn_AudioChannel_1=-1;
    if(!sCurSrvData.audio_channel_1_name.IsEmpty())
      btn_AudioChannel_1 = pMenu->AddButton(sCurSrvData.audio_channel_1_name); // A1

    int btn_AudioChannel_2=-1;
    if(!sCurSrvData.audio_channel_2_name.IsEmpty())
      btn_AudioChannel_2 = pMenu->AddButton(sCurSrvData.audio_channel_2_name); // A2

    pMenu->CenterWindow();
    pMenu->DoModal();
    int btnid = pMenu->GetButton();
    if (btnid == btn_AudioChannel_0)
    {
      strAudioChannelName = sCurSrvData.audio_channel_0_name;
      strAudioChannelPid = sCurSrvData.audio_channel_0_pid;
      return true;
    }
    if (btnid == btn_AudioChannel_1)
    {
      strAudioChannelName = sCurSrvData.audio_channel_1_name;
      strAudioChannelPid = sCurSrvData.audio_channel_1_pid;
      return true;
    }
    if (btnid == btn_AudioChannel_2)
    {
      strAudioChannelName = sCurSrvData.audio_channel_2_name;
      strAudioChannelPid = sCurSrvData.audio_channel_2_pid;
      return true;
    }
  }
  return false;
}
bool CTuxBoxUtil::GetVideoSubChannels(CStdString& strVideoSubChannelName, CStdString& strVideoSubChannelPid)
{
  // no video sub channel return false!
  if(vVideoSubChannel.name.size() <= 0 || vVideoSubChannel.reference.size() <= 0)
    return false;

  // IsPlaying, Stop it..
  if(g_application.IsPlaying())
    g_applicationMessenger.MediaStop();

  // popup the context menu
  CGUIDialogContextMenu *pMenu;
  pMenu = (CGUIDialogContextMenu *)m_gWindowManager.GetWindow(WINDOW_DIALOG_CONTEXT_MENU);
  if (pMenu)
  {
    pMenu->Initialize();
    // load Video Sub Channels to context menu
    std::vector<int> btn;
    for (unsigned int i=0; vVideoSubChannel.name.size() > i; ++i)
      btn.push_back(pMenu->AddButton(vVideoSubChannel.name[i]));
    
    pMenu->CenterWindow();
    pMenu->DoModal();
    // get selected Video Sub Channel name and reference zap
    int btnid = pMenu->GetButton();
    for(int i=0; btn[i] >0; i++)
    {
      if(btnid == btn[i])
      {
        strVideoSubChannelName = vVideoSubChannel.name[i];
        strVideoSubChannelPid = vVideoSubChannel.reference[i];
        btn.clear();
        vVideoSubChannel.name.clear();
        vVideoSubChannel.reference.clear();
        vVideoSubChannel.selected.clear();
        return true;
      }
    }
  }
  return false;
}
//Input: Service Name (Channel Namne)
//Output: picon url (on ERROR the default icon path will be returned)
CStdString CTuxBoxUtil::GetPicon(CStdString strServiceName)
{
  if(!g_advancedSettings.m_bTuxBoxPictureIcon)
  {
    CLog::Log(LOGDEBUG, "%s PictureIcon Detection is Disabled! Using default icon", __FUNCTION__);
    return "";
  }
  if (strServiceName.IsEmpty())
  {
    CLog::Log(LOGDEBUG, "%s Service Name is Empty! Can not detect a PictureIcon. Using default icon!", __FUNCTION__);
    return "";
  }
  else
  {
    CStdString piconXML, piconPath, defaultPng;
    CStdString strName, strPng;
    piconPath = "Q:\\userdata\\pictureicon\\Picon\\";
    defaultPng = piconPath+"default.png";
    piconXML = "Q:\\userdata\\pictureicon\\picon.xml";
    TiXmlDocument piconDoc;
    
    if (!CFile::Exists(piconXML))
    { 
      return defaultPng;
    }
    if (!piconDoc.LoadFile(piconXML))
    {
      CLog::Log(LOGERROR, "Error loading %s, Line %d\n%s", piconXML.c_str(), piconDoc.ErrorRow(), piconDoc.ErrorDesc());
      return defaultPng;
    }

    TiXmlElement *pRootElement = piconDoc.RootElement();
    if (!pRootElement || strcmpi(pRootElement->Value(),"picon") != 0)
    {
      CLog::Log(LOGERROR, "Error loading %s, no <picon> node", piconXML.c_str());
      return defaultPng;
    }

    TiXmlElement* pServices = pRootElement->FirstChildElement("services");
    TiXmlElement* pService;
    pService = pServices->FirstChildElement("service");
    while(pService)
    {
      if(pService->Attribute("name"))
      {
        strName.Format("%s",pService->Attribute("name"));
      }
      if(pService->Attribute("png"))
      {
        strPng.Format("%s",pService->Attribute("png"));
      }
      
      if(strName.Equals(strServiceName))
      {
        strPng.Format("%s%s",piconPath.c_str(), strPng.c_str());
        strPng.ToLower();
        CLog::Log(LOGDEBUG, "%s %s: Path is: %s", __FUNCTION__,strServiceName.c_str(), strPng.c_str());
        return strPng;
      }
      pService = pService->NextSiblingElement("service");
    }
    return defaultPng;
  }
}

// iMODE: 0 = TV, 1 = Radio, 2 = Data, 3 = Movies, 4 = Root
// SUBMODE: 0 = n/a, 1 = All, 2 = Satellites, 2 = Providers, 4 = Bouquets
CStdString CTuxBoxUtil::GetSubMode(int iMode, CStdString& strXMLRootString, CStdString& strXMLChildString)
{
  //Todo: add a setting: "Don't Use Request mode" to advanced.xml
  
  // MODE: 0 = TV, 1 = Radio, 2 = Data, 3 = Movies, 4 = Root
	// SUBMODE: 0 = n/a, 1 = All, 2 = Satellites, 2 = Providers, 4 = Bouquets
  // Default Submode
  int iSubmode = 4;
  CStdString strSubMode;

  if(iMode <0 || iMode >4)
  {
    strSubMode.Format("xml/services?mode=0&submode=4");
    strXMLRootString.Format("bouquets");
    strXMLChildString.Format("bouquet");
    return strSubMode;
  }
  
  // popup the context menu
  CGUIDialogContextMenu *pMenu;
  pMenu = (CGUIDialogContextMenu *)m_gWindowManager.GetWindow(WINDOW_DIALOG_CONTEXT_MENU);
  if (pMenu)
  {
    // load Audio context menu
    pMenu->Initialize();
    // add the needed Audio buttons
    int iSubmode_1 = pMenu->AddButton("All"); //;
    int iSubmode_2 = pMenu->AddButton("Satellites"); //;
    int iSubmode_3 = pMenu->AddButton("Providers");
    int iSubmode_4 = pMenu->AddButton("Bouquets");
    
    pMenu->CenterWindow();
    pMenu->DoModal();
    int btnid = pMenu->GetButton();
    if(btnid == iSubmode_1)
    {
      iSubmode = 1;
      strXMLRootString.Format("services");
      strXMLChildString.Format("service");
    }
    else if(btnid == iSubmode_2)
    {
      iSubmode = 2;
      strXMLRootString.Format("satellites");
      strXMLChildString.Format("satellite");

    }
    else if(btnid == iSubmode_3)
    {
      iSubmode = 3;
      strXMLRootString.Format("providers");
      strXMLChildString.Format("provider");
    }
    else if(btnid == iSubmode_4)
    {
      iSubmode = 4;
      strXMLRootString.Format("bouquets");
      strXMLChildString.Format("bouquet");
    }
  }
  strSubMode.Format("xml/services?mode=%i&submode=%i",iMode,iSubmode);
  return strSubMode;
}
//Input: url/path of share/item file/folder
//Output: the detected submode root and child string
CStdString CTuxBoxUtil::DetectSubMode(CStdString strSubMode, CStdString& strXMLRootString, CStdString& strXMLChildString)
{
  //strSubMode = "xml/services?mode=0&submode=1"
  CStdString strFilter;
  int ipointMode = strSubMode.Find("?mode=");
  int ipointSubMode = strSubMode.Find("&submode=");
  if (ipointMode >=0)
  {
    strFilter = strSubMode.GetAt(ipointMode+6);
  }
  if (ipointSubMode >=0)
  {
    CStdString strTemp;
    strTemp = strSubMode.GetAt(ipointSubMode+9);
    if(strTemp.Equals("1"))
    {
      strXMLRootString.Format("unknowns");
      strXMLChildString.Format("unknown");
    }
    else if(strTemp.Equals("2"))
    {
      strXMLRootString.Format("satellites");
      strXMLChildString.Format("satellite");
    }
    else if(strTemp.Equals("3"))
    {
      strXMLRootString.Format("providers");
      strXMLChildString.Format("provider");
    }
    else if(strTemp.Equals("4"))
    {
      strXMLRootString.Format("bouquets");
      strXMLChildString.Format("bouquet");
    }

  }
  return strFilter;
}
