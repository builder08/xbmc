/*
 *  Copyright (c) 2020 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "Builtins.h"

//! \brief Class providing player related built-in commands.
class CPlayerBuiltins
{
public:
  //! \brief Returns the map of operations.
  CBuiltins::CommandMap GetOperations() const;
};
