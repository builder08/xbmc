/*****************************************************************
|
|   Neptune - Messaging System
|
|   (c) 2001-2006 Gilles Boccon-Gibod
|   Author: Gilles Boccon-Gibod (bok@bok.net)
|
 ****************************************************************/

#ifndef _NPT_MESSAGING_H_
#define _NPT_MESSAGING_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "NptConstants.h"
#include "NptTypes.h"
#include "NptResults.h"
#include "NptList.h"

/*----------------------------------------------------------------------
|   forward references
+---------------------------------------------------------------------*/
class NPT_Message;

/*----------------------------------------------------------------------
|   NPT_MessageHandler
+---------------------------------------------------------------------*/
class NPT_MessageHandler
{
 public:
    // methods
    virtual ~NPT_MessageHandler() {}

    // default message handler
    virtual void OnMessage(NPT_Message*) {}

    // this method is a central point of handling for received messages.
    // it can be overloaded by subclasses who wish to process all 
    // incoming messages
    virtual NPT_Result HandleMessage(NPT_Message* message);
};

/*----------------------------------------------------------------------
|   NPT_Messsage
+---------------------------------------------------------------------*/
class NPT_Message
{
 public:
    // types
    typedef const char* Type;

    // static members
    static Type MessageType;

    // methods
    virtual           ~NPT_Message() {}
    virtual Type       GetType() { return MessageType; }
    virtual NPT_Result Dispatch(NPT_MessageHandler* handler) {
        return DefaultDeliver(handler);
    }
    // this method should really be called 'Deliver', but this would
    // cause a problem when subclasses overload it 
    virtual NPT_Result DefaultDeliver(NPT_MessageHandler* handler) {
        handler->OnMessage(this);
        return NPT_SUCCESS;
    }
};

/*----------------------------------------------------------------------
|   NPT_TerminateMesssage
+---------------------------------------------------------------------*/
class NPT_TerminateMessage : public NPT_Message
{
 public:
    // methods
    NPT_Result Dispatch(NPT_MessageHandler* /*handler*/) {
        return NPT_FAILURE;
    }
};

/*----------------------------------------------------------------------
|   NPT_MessageQueue
+---------------------------------------------------------------------*/
class NPT_MessageQueue
{
 public:
    // methods
    virtual           ~NPT_MessageQueue() {}
    virtual NPT_Result PumpMessage(bool blocking = true) = 0;
    virtual NPT_Result QueueMessage(NPT_Message*        message, 
                                    NPT_MessageHandler* handler) = 0;
};

/*----------------------------------------------------------------------
|   NPT_MessageReceiver
+---------------------------------------------------------------------*/
class NPT_MessageReceiver
{
public:
    // methods
    NPT_MessageReceiver() : m_Queue(NULL), m_Handler(NULL) {}
    NPT_MessageReceiver(NPT_MessageHandler* handler) : 
        m_Queue(NULL), m_Handler(handler) {}
    NPT_MessageReceiver(NPT_MessageQueue* queue) : 
        m_Queue(queue), m_Handler(NULL) {}
    NPT_MessageReceiver(NPT_MessageHandler* handler, 
                        NPT_MessageQueue*   queue) : 
        m_Queue(queue), m_Handler(handler) {}
    virtual ~NPT_MessageReceiver() {}
    NPT_Result SetQueue(NPT_MessageQueue* queue) {
        m_Queue = queue;
        return NPT_SUCCESS;
    }
    NPT_Result SetHandler(NPT_MessageHandler* handler) {
        m_Handler = handler;
        return NPT_SUCCESS;
    }
    virtual NPT_Result PostMessage(NPT_Message* message) {
        if (m_Queue) {
            return m_Queue->QueueMessage(message, m_Handler);
        } else {
            return NPT_FAILURE;
        }
    }

protected:
    // members
    NPT_MessageQueue*   m_Queue;
    NPT_MessageHandler* m_Handler;
};

/*----------------------------------------------------------------------
|   NPT_MessageBroadcaster
+---------------------------------------------------------------------*/
class NPT_MessageBroadcaster
{
public:
    // methods
    NPT_MessageBroadcaster(NPT_Message* message) : m_Message(message) {}
    NPT_Result operator()(NPT_MessageReceiver*& receiver) const {
        receiver->PostMessage(m_Message);
        return NPT_SUCCESS;
    }

private:
    // members
    NPT_Message* m_Message;
};

#endif // _NPT_MESSAGING_H_
