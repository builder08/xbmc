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
#include "DBusMessage.h"
#include "log.h"
#ifdef HAS_DBUS

CDBusMessage::CDBusMessage(const char *destination, const char *object, const char *interface, const char *method)
{
  m_reply = NULL;
  m_message = dbus_message_new_method_call (destination, object, interface, method);
  CLog::Log(LOGDEBUG, "DBus: Creating message to %s on %s with interface %s and method %s\n", destination, object, interface, method);
}

CDBusMessage::~CDBusMessage()
{
  Close();
}

bool CDBusMessage::AppendObjectPath(const char *object)
{
  return dbus_message_append_args(m_message, DBUS_TYPE_OBJECT_PATH, &object, DBUS_TYPE_INVALID);
}

bool CDBusMessage::AppendArgument(const char *string)
{
  return dbus_message_append_args(m_message, DBUS_TYPE_STRING, &string, DBUS_TYPE_INVALID);
}

bool CDBusMessage::AppendArgument(const char **arrayString, int length)
{
  return dbus_message_append_args(m_message, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING, &arrayString, length, DBUS_TYPE_INVALID);
}

DBusMessage *CDBusMessage::SendSystem()
{
  return Send(DBUS_BUS_SYSTEM);
}

DBusMessage *CDBusMessage::SendSession()
{
  return Send(DBUS_BUS_SESSION);
}

DBusMessage *CDBusMessage::Send(DBusBusType type)
{
  DBusError error;
  dbus_error_init (&error);
  DBusConnection *con = dbus_bus_get(type, &error);

  DBusMessage *returnMessage = Send(con, &error);

  if (dbus_error_is_set(&error))
    CLog::Log(LOGERROR, "DBus: Error %s - %s", error.name, error.message);

  dbus_error_free (&error);
  dbus_connection_unref(con);

  return returnMessage;
}

DBusMessage *CDBusMessage::Send(DBusConnection *con, DBusError *error)
{
  if (con && m_message)
  {
    if (m_reply)
      dbus_message_unref(m_reply);
    m_reply = dbus_connection_send_with_reply_and_block(con, m_message, -1, error);
  }

  return m_reply;
}

void CDBusMessage::Close()
{
  if (m_message)
    dbus_message_unref(m_message);

  if (m_reply)
    dbus_message_unref(m_reply);
}
#endif
