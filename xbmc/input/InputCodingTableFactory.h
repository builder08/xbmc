/*
 *  Copyright (c) 2020 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include <string>

class TiXmlElement;
class IInputCodingTable;

class CInputCodingTableFactory
{
public:
  static IInputCodingTable* CreateCodingTable(const std::string& strTableName, const TiXmlElement* element);
};
