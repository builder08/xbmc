/*****************************************************************
|
|   Platinum - Service Action
|
|   Copyright (c) 2004-2008, Plutinosoft, LLC.
|   Author: Sylvain Rebaud (sylvain@plutinosoft.com)
|
 ****************************************************************/

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "PltAction.h"
#include "PltService.h"
#include "PltXmlHelper.h"

NPT_SET_LOCAL_LOGGER("platinum.core.action")

/*----------------------------------------------------------------------
|   PLT_ActionDesc::PLT_ActionDesc
+---------------------------------------------------------------------*/
PLT_ActionDesc::PLT_ActionDesc(const char* name, PLT_Service* service) : 
    m_Name(name), 
    m_Service(service)
{
}

/*----------------------------------------------------------------------
|   PLT_ActionDesc::~PLT_ActionDesc
+---------------------------------------------------------------------*/
PLT_ActionDesc::~PLT_ActionDesc() 
{
    m_ArgumentDescs.Apply(NPT_ObjectDeleter<PLT_ArgumentDesc>());
}

/*----------------------------------------------------------------------
|   PLT_ActionDesc::GetSCPDXML
+---------------------------------------------------------------------*/
NPT_Result
PLT_ActionDesc::GetSCPDXML(NPT_XmlElementNode* node)
{
    NPT_XmlElementNode* action = new NPT_XmlElementNode("action");
    NPT_CHECK_SEVERE(node->AddChild(action));
    NPT_CHECK_SEVERE(PLT_XmlHelper::AddChildText(action, "name", m_Name));

    NPT_XmlElementNode* argumentList = new NPT_XmlElementNode("argumentList");
    NPT_CHECK_SEVERE(action->AddChild(argumentList));

    // no arguments is ok
    if (!m_ArgumentDescs.GetItemCount()) return NPT_SUCCESS;

    return m_ArgumentDescs.ApplyUntil(
        PLT_GetSCPDXMLIterator<PLT_ArgumentDesc>(argumentList), 
        NPT_UntilResultNotEquals(NPT_SUCCESS));
}

/*----------------------------------------------------------------------
|   PLT_ActionDesc::GetService
+---------------------------------------------------------------------*/
PLT_Service*
PLT_ActionDesc::GetService() 
{
    return m_Service;
}

/*----------------------------------------------------------------------
|   PLT_ActionDesc::GetArgumentDesc
+---------------------------------------------------------------------*/
PLT_ArgumentDesc*
PLT_ActionDesc::GetArgumentDesc(const char* name)
{
    PLT_ArgumentDesc* arg_desc = NULL;
    NPT_ContainerFind(m_ArgumentDescs, PLT_ArgumentDescNameFinder(name), arg_desc);
    return arg_desc;
}

/*----------------------------------------------------------------------
|   PLT_Action::PLT_Action
+---------------------------------------------------------------------*/
PLT_Action::PLT_Action(PLT_ActionDesc* action_desc) : 
    m_ActionDesc(action_desc),
    m_ErrorCode(0) 
{
}

/*----------------------------------------------------------------------
|   PLT_Action::~PLT_Action
+---------------------------------------------------------------------*/
PLT_Action::~PLT_Action() 
{
    m_Arguments.Apply(NPT_ObjectDeleter<PLT_Argument>());
}

/*----------------------------------------------------------------------
|   PLT_Action::GetArgumentValue
+---------------------------------------------------------------------*/
NPT_Result 
PLT_Action::GetArgumentValue(const char* name, NPT_String& value) 
{
    PLT_Argument* arg = GetArgument(name);
    if (arg == NULL) {
        return NPT_FAILURE;
    }
    value = arg->GetValue();
    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   PLT_Action::GetArgumentValue
+---------------------------------------------------------------------*/
NPT_Result 
PLT_Action::GetArgumentValue(const char* name, NPT_UInt32& value) 
{
    NPT_String tmp_value;
    NPT_CHECK_WARNING(GetArgumentValue(name, tmp_value));
    return tmp_value.ToInteger((long&)value);
}

/*----------------------------------------------------------------------
|   PLT_Action::GetArgumentValue
+---------------------------------------------------------------------*/
NPT_Result 
PLT_Action::GetArgumentValue(const char* name, NPT_Int32& value) 
{
    NPT_String tmp_value;
    NPT_CHECK_WARNING(GetArgumentValue(name, tmp_value));
    return tmp_value.ToInteger((long&)value);
}

/*----------------------------------------------------------------------
|   PLT_Action::GetArgument
+---------------------------------------------------------------------*/
PLT_Argument*
PLT_Action::GetArgument(const char* name)
{
    PLT_Argument* argument = NULL;
    NPT_ContainerFind(m_Arguments, PLT_ArgumentNameFinder(name), argument);
    return argument;
}

/*----------------------------------------------------------------------
|   PLT_Action::SetArgumentValue
+---------------------------------------------------------------------*/
NPT_Result
PLT_Action::SetArgumentValue(const char* name,
                             const char* value) 
{
    PLT_Argument* arg = NULL;

    // look for this argument in our argument list
    // and replace the value if we found it 
    if (NPT_SUCCEEDED(NPT_ContainerFind(m_Arguments, PLT_ArgumentNameFinder(name), arg))) {
        return arg->SetValue(value);
    }

    // since we didn't find it, create a clone 
    // from the action arguments
    PLT_ArgumentDesc* arg_desc = m_ActionDesc->GetArgumentDesc(name);
    if (arg_desc == NULL) {
        return NPT_ERROR_INVALID_PARAMETERS;
    }
    arg = new PLT_Argument(arg_desc);

    NPT_Result res = arg->SetValue(value);
    if (NPT_FAILED(res)) {
        delete arg;
        return res;
    }

    m_Arguments.Add(arg);
    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   PLT_Action::VerifyArgumentValue
+---------------------------------------------------------------------*/
NPT_Result
PLT_Action::VerifyArgumentValue(const char* name, const char* value)
{
    NPT_String str;
    NPT_CHECK_SEVERE(GetArgumentValue(name, str));

    return str.Compare(value, true)?NPT_FAILURE:NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   PLT_Action::VerifyArguments
+---------------------------------------------------------------------*/
NPT_Result
PLT_Action::VerifyArguments(bool input)
{
    NPT_Cardinal count = 0;

    // Check we have all the required parameters (in or out)
    for(unsigned int i=0; i<m_ActionDesc->GetArgumentDescs().GetItemCount(); i++) {
        PLT_ArgumentDesc* arg_desc = m_ActionDesc->GetArgumentDescs()[i];

        // only input arguments are needed
        if (arg_desc->GetDirection().Compare(input?"in":"out", true))
            continue;

        // look for this argument in the list we received
        PLT_Argument* arg = NULL;
        if (NPT_FAILED(NPT_ContainerFind(m_Arguments, PLT_ArgumentNameFinder(arg_desc->GetName()), arg))) {
            return NPT_FAILURE;
        }
        ++count;
    }

    SetError(0, "");
    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   PLT_Action::SetArgumentOutFromStateVariable
+---------------------------------------------------------------------*/
NPT_Result
PLT_Action::SetArgumentOutFromStateVariable(PLT_ArgumentDesc* arg_desc)
{
    // only output arguments can use a state variable
    if (arg_desc->GetDirection().Compare("out", true)) {
        return NPT_FAILURE;
    }

    PLT_StateVariable* variable = arg_desc->GetRelatedStateVariable();
    if (!variable) return NPT_FAILURE;

    // assign the value to an argument
    NPT_CHECK_SEVERE(SetArgumentValue(arg_desc->GetName(), variable->GetValue()));
    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   PLT_Action::SetArgumentOutFromStateVariable
+---------------------------------------------------------------------*/
NPT_Result
PLT_Action::SetArgumentOutFromStateVariable(const char* name)
{
    // look for this argument in the action list of arguments
    PLT_ArgumentDesc* arg_desc = NULL;
    NPT_CHECK_SEVERE(NPT_ContainerFind(m_ActionDesc->GetArgumentDescs(), 
        PLT_ArgumentDescNameFinder(name), arg_desc));

    return SetArgumentOutFromStateVariable(arg_desc);
}

/*----------------------------------------------------------------------
|   PLT_Action::SetArgumentsOutFromStateVariable
+---------------------------------------------------------------------*/
NPT_Result
PLT_Action::SetArgumentsOutFromStateVariable()
{
    // go through the list of the action output arguments
    for(unsigned int i=0; i<m_ActionDesc->GetArgumentDescs().GetItemCount(); i++) {
        PLT_ArgumentDesc* arg_desc = m_ActionDesc->GetArgumentDescs()[i];

        // only output arguments are needed
        if (arg_desc->GetDirection().Compare("out", true))
            continue;

        NPT_CHECK_SEVERE(SetArgumentOutFromStateVariable(arg_desc));
    }

    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   PLT_Action::SetError
+---------------------------------------------------------------------*/
NPT_Result 
PLT_Action::SetError(unsigned int code, const char* description) 
{
    m_ErrorCode = code;
    m_ErrorDescription = description;
    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   PLT_Action::GetError
+---------------------------------------------------------------------*/
const char* 
PLT_Action::GetError(unsigned int& code) 
{
    code = m_ErrorCode;
    return m_ErrorDescription;
}

/*----------------------------------------------------------------------
|   PLT_Action::GetErrorCode
+---------------------------------------------------------------------*/
unsigned int 
PLT_Action::GetErrorCode() 
{
    return m_ErrorCode;
}

/*----------------------------------------------------------------------
|   PLT_Action::FormatSoapRequest
+---------------------------------------------------------------------*/
NPT_Result
PLT_Action::FormatSoapRequest(NPT_OutputStream& stream)
{
    NPT_String str;
    NPT_Result res;
    NPT_XmlElementNode* body = NULL;
    NPT_XmlElementNode* request = NULL;
    NPT_XmlElementNode* envelope = new NPT_XmlElementNode("s", "Envelope");

    NPT_CHECK_LABEL_SEVERE(res = envelope->SetNamespaceUri("s", "http://schemas.xmlsoap.org/soap/envelope/"), cleanup);
    NPT_CHECK_LABEL_SEVERE(res = envelope->SetAttribute("s", "encodingStyle", "http://schemas.xmlsoap.org/soap/encoding/"), cleanup);

    body = new NPT_XmlElementNode("s", "Body");
    NPT_CHECK_LABEL_SEVERE(res = envelope->AddChild(body), cleanup);

    request = new NPT_XmlElementNode("u", m_ActionDesc->GetName());
    NPT_CHECK_LABEL_SEVERE(res = request->SetNamespaceUri("u", m_ActionDesc->GetService()->GetServiceType()), cleanup);
    NPT_CHECK_LABEL_SEVERE(res = body->AddChild(request), cleanup);

    for(unsigned int i=0; i<m_Arguments.GetItemCount(); i++) {
        PLT_Argument* argument = m_Arguments[i];
        if (argument->GetDesc()->GetDirection().Compare("in", true) == 0) {
            NPT_CHECK_LABEL_SEVERE(res = PLT_XmlHelper::AddChildText(
                request, 
                argument->GetDesc()->GetName(), 
                argument->GetValue()), cleanup);
        }
    }

    NPT_CHECK_LABEL_SEVERE(res = PLT_XmlHelper::Serialize(*envelope, str), cleanup);
    delete envelope;

    return stream.Write((const char*)str, str.GetLength());

cleanup:
    delete envelope;
    return res;
}

/*----------------------------------------------------------------------
|   PLT_Action::FormatSoapResponse
+---------------------------------------------------------------------*/
NPT_Result
PLT_Action::FormatSoapResponse(NPT_OutputStream& stream)
{
    if (m_ErrorCode) {
        return FormatSoapError(m_ErrorCode, m_ErrorDescription, stream);
    }

    NPT_String str;
    NPT_Result res;
    NPT_XmlElementNode* body = NULL;
    NPT_XmlElementNode* response = NULL;
    NPT_XmlElementNode* node = NULL;
    NPT_XmlElementNode* envelope = new NPT_XmlElementNode("s", "Envelope");

    NPT_CHECK_LABEL_SEVERE(res = envelope->SetNamespaceUri("s", "http://schemas.xmlsoap.org/soap/envelope/"), cleanup);
    NPT_CHECK_LABEL_SEVERE(res = envelope->SetAttribute("s", "encodingStyle", "http://schemas.xmlsoap.org/soap/encoding/"), cleanup);

    body = new NPT_XmlElementNode("s", "Body");
    NPT_CHECK_LABEL_SEVERE(res = envelope->AddChild(body), cleanup);        

    response = new NPT_XmlElementNode("u", m_ActionDesc->GetName() + "Response");
    NPT_CHECK_LABEL_SEVERE(response->SetNamespaceUri("u", m_ActionDesc->GetService()->GetServiceType()), cleanup);
    NPT_CHECK_LABEL_SEVERE(res = body->AddChild(response), cleanup);    

    for(unsigned int i=0; i<m_Arguments.GetItemCount(); i++) {
        PLT_Argument* argument = m_Arguments[i];
        if (argument->GetDesc()->GetDirection().Compare("out", true) == 0) {
            node = new NPT_XmlElementNode(argument->GetDesc()->GetName());
            NPT_CHECK_LABEL_SEVERE(res = node->AddText(argument->GetValue()), cleanup);
            NPT_CHECK_LABEL_SEVERE(res = response->AddChild(node), cleanup);

#ifndef REMOVE_WMP_DATATYPE_EXTENSION
            PLT_StateVariable* var = argument->GetDesc()->GetRelatedStateVariable();
            if (var) {
                node->SetNamespaceUri("dt", "urn:schemas-microsoft-com:datatypes");
                node->SetAttribute("dt", "dt", var->GetDataType());
            }
#endif
        }
    }

    // this will xmlescape any values that contain xml characters
    NPT_CHECK_LABEL_SEVERE(PLT_XmlHelper::Serialize(*envelope, str), cleanup);
    delete envelope;

    return stream.Write((const char*)str, str.GetLength());

cleanup:
    delete envelope;
    return res;
}

/*----------------------------------------------------------------------
|   PLT_Action::FormatSoapError
+---------------------------------------------------------------------*/
NPT_Result
PLT_Action::FormatSoapError(unsigned int code, NPT_String desc, NPT_OutputStream& stream)
{
    NPT_String str;
    NPT_Result res;
    NPT_XmlElementNode* body = NULL;
    NPT_XmlElementNode* fault = NULL;
    NPT_XmlElementNode* detail = NULL;
    NPT_XmlElementNode* UPnPError = NULL;
    NPT_XmlElementNode* envelope = new NPT_XmlElementNode("s", "Envelope");

    NPT_CHECK_LABEL_SEVERE(res = envelope->SetNamespaceUri("s", "http://schemas.xmlsoap.org/soap/envelope/"), cleanup);
    NPT_CHECK_LABEL_SEVERE(res = envelope->SetAttribute("s", "encodingStyle", "http://schemas.xmlsoap.org/soap/encoding/"), cleanup);

    body = new NPT_XmlElementNode("s", "Body");
    NPT_CHECK_LABEL_SEVERE(res = envelope->AddChild(body), cleanup);   

    fault = new NPT_XmlElementNode("s", "Fault");
    NPT_CHECK_LABEL_SEVERE(res = body->AddChild(fault), cleanup);

    NPT_CHECK_LABEL_SEVERE(res = PLT_XmlHelper::AddChildText(fault, "faultCode", "s:Client"), cleanup);
    NPT_CHECK_LABEL_SEVERE(res = PLT_XmlHelper::AddChildText(fault, "faultString", "UPnPError"), cleanup);

    detail = new NPT_XmlElementNode("detail");
    NPT_CHECK_LABEL_SEVERE(res = fault->AddChild(detail), cleanup);

    UPnPError = new NPT_XmlElementNode("UPnPError");
    NPT_CHECK_LABEL_SEVERE(res = UPnPError->SetNamespaceUri("", "urn:schemas-upnp-org:control-1-0"), cleanup);
    NPT_CHECK_LABEL_SEVERE(res = detail->AddChild(UPnPError), cleanup);

    NPT_CHECK_LABEL_SEVERE(res = PLT_XmlHelper::AddChildText(UPnPError, "errorCode", NPT_String::FromInteger(code)), cleanup);
    NPT_CHECK_LABEL_SEVERE(res = PLT_XmlHelper::AddChildText(UPnPError, "errorDescription", desc), cleanup);

    NPT_CHECK_LABEL_SEVERE(res = PLT_XmlHelper::Serialize(*envelope, str), cleanup);
    delete envelope;

    return stream.Write((const char*)str, str.GetLength());

cleanup:
    delete envelope;
    return res;
}
