/*
 *      Copyright (C) 2016 Christian Browet
 *      http://xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "LinkAddress.h"
#include "InetAddress.h"

#include "jutils/jutils-details.hpp"

using namespace jni;


CJNIInetAddress CJNILinkAddress::getAddress()
{
  return call_method<jhobject>(m_object,
                               "getAddress", "()Ljava/net/InetAddress;");
}

int CJNILinkAddress::getFlags()
{
  return call_method<jint>(m_object,
    "getFlags", "()I");
}

int CJNILinkAddress::getPrefixLength()
{
  return call_method<jint>(m_object,
    "getPrefixLength", "()I");
}

int CJNILinkAddress::getScope()
{
  return call_method<jint>(m_object,
    "getScope", "()I");
}

bool CJNILinkAddress::equals(const CJNILinkAddress& other)
{
  return call_method<jboolean>(m_object,
    "equals", "(Ljava/lang/Object;)Z", other.get_raw());
}

std::string CJNILinkAddress::toString() const
{
  return jcast<std::string>(call_method<jhstring>(m_object,
    "toString", "()Ljava/lang/String;"));
}

int CJNILinkAddress::describeContents() const
{
  return call_method<jint>(m_object,
    "describeContents", "()I");
}

