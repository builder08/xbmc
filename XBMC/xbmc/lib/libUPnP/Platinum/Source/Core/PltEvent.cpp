/*****************************************************************
|
|   Platinum - Control/Event
|
| Copyright (c) 2004-2008, Plutinosoft, LLC.
| All rights reserved.
| http://www.plutinosoft.com
|
| This program is free software; you can redistribute it and/or
| modify it under the terms of the GNU General Public License
| as published by the Free Software Foundation; either version 2
| of the License, or (at your option) any later version.
|
| OEMs, ISVs, VARs and other distributors that combine and 
| distribute commercially licensed software with Platinum software
| and do not wish to distribute the source code for the commercially
| licensed software under version 2, or (at your option) any later
| version, of the GNU General Public License (the "GPL") must enter
| into a commercial license agreement with Plutinosoft, LLC.
| 
| This program is distributed in the hope that it will be useful,
| but WITHOUT ANY WARRANTY; without even the implied warranty of
| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
| GNU General Public License for more details.
|
| You should have received a copy of the GNU General Public License
| along with this program; see the file LICENSE.txt. If not, write to
| the Free Software Foundation, Inc., 
| 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
| http://www.gnu.org/licenses/gpl-2.0.html
|
****************************************************************/

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "PltTaskManager.h"
#include "PltEvent.h"
#include "PltService.h"
#include "PltUPnP.h"
#include "PltDeviceData.h"
#include "PltXmlHelper.h"
#include "PltCtrlPointTask.h"

NPT_SET_LOCAL_LOGGER("platinum.core.event")

/*----------------------------------------------------------------------
|   PLT_EventSubscriber::PLT_EventSubscriber
+---------------------------------------------------------------------*/
PLT_EventSubscriber::PLT_EventSubscriber(PLT_TaskManager* task_manager, 
                                         PLT_Service*     service,
                                         const char*      sid,
                                         int              timeout /* = -1 */) : 
    m_TaskManager(task_manager), 
    m_Service(service), 
    m_EventKey(0),
    m_SubscriberTask(NULL),
    m_SID(sid)
{
    NPT_LOG_FINE_1("Creating new subscriber (%s)", m_SID.GetChars());
    SetTimeout(timeout);
}

/*----------------------------------------------------------------------
|   PLT_EventSubscriber::~PLT_EventSubscriber
+---------------------------------------------------------------------*/
PLT_EventSubscriber::~PLT_EventSubscriber() 
{
    NPT_LOG_FINE_1("Deleting subscriber (%s)", m_SID.GetChars());
    if (m_SubscriberTask) {
        m_SubscriberTask->Kill();
        m_SubscriberTask = NULL;
    }
}

/*----------------------------------------------------------------------
|   PLT_EventSubscriber::GetService
+---------------------------------------------------------------------*/
PLT_Service*
PLT_EventSubscriber::GetService() 
{
    return m_Service;
}

/*----------------------------------------------------------------------
|   PLT_EventSubscriber::GetEventKey
+---------------------------------------------------------------------*/
NPT_Ordinal
PLT_EventSubscriber::GetEventKey() 
{
    return m_EventKey;
}

/*----------------------------------------------------------------------
|   PLT_EventSubscriber::SetEventKey
+---------------------------------------------------------------------*/
NPT_Result
PLT_EventSubscriber::SetEventKey(NPT_Ordinal value) 
{
    m_EventKey = value;
    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   PLT_EventSubscriber::GetLocalIf
+---------------------------------------------------------------------*/
NPT_SocketAddress
PLT_EventSubscriber::GetLocalIf() 
{
    return m_LocalIf;
}

/*----------------------------------------------------------------------
|   PLT_EventSubscriber::SetLocalIf
+---------------------------------------------------------------------*/
NPT_Result
PLT_EventSubscriber::SetLocalIf(NPT_SocketAddress value) 
{
    m_LocalIf = value;
    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   PLT_EventSubscriber::GetExpirationTime
+---------------------------------------------------------------------*/
// a TimeStamp of 0 means no expiration
NPT_TimeStamp
PLT_EventSubscriber::GetExpirationTime()
{
    return m_ExpirationTime;
}

/*----------------------------------------------------------------------
|   PLT_EventSubscriber::SetExpirationTime
+---------------------------------------------------------------------*/
NPT_Result
PLT_EventSubscriber::SetTimeout(NPT_Cardinal timeout) 
{
    NPT_LOG_FINE_2("subscriber (%s) expiring in %d seconds",
        m_SID.GetChars(),
        timeout);

    // -1 means infinite but we default to 300 secs
    if (timeout == (NPT_Cardinal)-1) timeout = 300;
    
    NPT_System::GetCurrentTimeStamp(m_ExpirationTime);
    m_ExpirationTime += NPT_TimeInterval(timeout, 0);

    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   PLT_EventSubscriber::FindCallbackURL
+---------------------------------------------------------------------*/
NPT_Result
PLT_EventSubscriber::FindCallbackURL(const char* callback_url) 
{
    NPT_String res;
    return NPT_ContainerFind(m_CallbackURLs, 
                             NPT_StringFinder(callback_url), 
                             res);
}

/*----------------------------------------------------------------------
|   PLT_EventSubscriber::AddCallbackURL
+---------------------------------------------------------------------*/
NPT_Result
PLT_EventSubscriber::AddCallbackURL(const char* callback_url) 
{
    NPT_CHECK_POINTER_FATAL(callback_url);

    NPT_LOG_FINE_2("Adding callback \"%s\" to subscriber %s", 
        callback_url, 
        m_SID.GetChars());
    return m_CallbackURLs.Add(callback_url);
}

/*----------------------------------------------------------------------
|   PLT_EventSubscriber::Notify
+---------------------------------------------------------------------*/
NPT_Result
PLT_EventSubscriber::Notify(NPT_List<PLT_StateVariable*>& vars)
{
    // verify we have eventable variables
    bool foundVars = false;
    NPT_XmlElementNode* propertyset = 
        new NPT_XmlElementNode("e", "propertyset");
    NPT_CHECK_SEVERE(propertyset->SetNamespaceUri(
        "e", 
        "urn:schemas-upnp-org:event-1-0"));

    NPT_List<PLT_StateVariable*>::Iterator var = vars.GetFirstItem();
    while (var) {
        if ((*var)->IsSendingEvents()) {
            NPT_XmlElementNode* property = 
                new NPT_XmlElementNode("e", "property");
            propertyset->AddChild(property);
            PLT_XmlHelper::AddChildText(property, 
                                        (*var)->GetName(), 
                                        (*var)->GetValue());
            foundVars = true;
        }
        ++var;
    }

    // no eventable state variables found!
    if (foundVars == false) {
        delete propertyset;
        return NPT_FAILURE;
    }

    // format the body with the xml
    NPT_String xml;
    if (NPT_FAILED(PLT_XmlHelper::Serialize(*propertyset, 
                                            xml))) {
        delete propertyset;
        NPT_CHECK_FATAL(NPT_FAILURE);
    }
    delete propertyset;


    // parse the callback url
    NPT_HttpUrl url(m_CallbackURLs[0]);
    if (!url.IsValid()) {
        NPT_CHECK_FATAL(NPT_FAILURE);
    }
    // format request
    NPT_HttpRequest* request = 
        new NPT_HttpRequest(url,
                            "NOTIFY",
                            NPT_HTTP_PROTOCOL_1_1);
    // add the extra headers
    PLT_HttpHelper::SetContentType(*request, 
                                   "text/xml; charset=\"utf-8\"");
    PLT_UPnPMessageHelper::SetNT(*request, 
                                 "upnp:event");
    PLT_UPnPMessageHelper::SetNTS(*request, 
                                  "upnp:propchange");
    PLT_UPnPMessageHelper::SetSID(*request, 
                                  m_SID);
    PLT_UPnPMessageHelper::SetSeq(*request, 
                                  m_EventKey);

    // wrap around sequence to 1
    if (++m_EventKey == 0) m_EventKey = 1;

    PLT_HttpHelper::SetBody(*request, xml);

    // start the task now if not started already
    if (!m_SubscriberTask) {
        m_SubscriberTask = new PLT_HttpClientSocketTask(request, true);
        
        NPT_TimeInterval delay(0.2f);
        // delay start to make sure ctrlpoint receives response to subscription
        // before our first NOTIFY. Also make sure task is not auto-destroy
        // since we want to destroy it ourselves when the subscriber goes away.
        NPT_CHECK_FATAL(m_TaskManager->StartTask(m_SubscriberTask, &delay, false));
    } else {
        m_SubscriberTask->AddRequest(request);
    }
     
    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   PLT_EventSubscriber::Renew
+---------------------------------------------------------------------*/
NPT_Result
PLT_EventSubscriber::Renew(PLT_CtrlPoint* ctrl_point)
{
    NPT_LOG_FINE_3("Renewing subscriber \"%s\" for service \"%s\" of device \"%s\"", 
        (const char*)GetSID(),
        (const char*)m_Service->GetServiceID(),
        (const char*)m_Service->GetDevice()->GetFriendlyName());

    NPT_HttpUrl url(m_Service->GetEventSubURL(true));
    
    // create the request
    NPT_HttpRequest* request = new NPT_HttpRequest(url, "SUBSCRIBE", NPT_HTTP_PROTOCOL_1_1);

    PLT_UPnPMessageHelper::SetSID(*request, GetSID());
    PLT_UPnPMessageHelper::SetTimeOut(*request, (NPT_Timeout)PLT_Constants::GetInstance().m_DefaultSubscribeLease);
        
    // Prepare the request
    // create a task to post the request
    PLT_ThreadTask* task = new PLT_CtrlPointSubscribeEventTask(
        request,
        ctrl_point, 
        m_Service);
    return m_TaskManager->StartTask(task);
}

/*----------------------------------------------------------------------
|   PLT_EventSubscriberFinderByService::operator()
+---------------------------------------------------------------------*/
bool 
PLT_EventSubscriberFinderByService::operator()(PLT_EventSubscriber* const & eventSub) const 
{
    return m_Service->GetDevice()->GetUUID().Compare(
        eventSub->GetService()->GetDevice()->GetUUID(), true) ? false : true;
}
