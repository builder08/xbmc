#pragma once
/*
 *      Copyright (C) 2013-present Team Kodi
 *      This file is part of Kodi - https://kodi.tv
 *
 *  Kodi is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Kodi is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kodi. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <set>

#include "IFileItemListModifier.h"

class CFileItemListModification : public IFileItemListModifier
{
public:
  ~CFileItemListModification() override;

  static CFileItemListModification& GetInstance();

  bool CanModify(const CFileItemList &items) const override;
  bool Modify(CFileItemList &items) const override;

private:
  CFileItemListModification();
  CFileItemListModification(const CFileItemListModification&) = delete;
  CFileItemListModification& operator=(CFileItemListModification const&) = delete;

  std::set<IFileItemListModifier*> m_modifiers;
};
