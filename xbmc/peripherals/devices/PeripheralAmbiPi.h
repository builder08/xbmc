#pragma once
/*
 *      Copyright (C) 2005-2011 Team XBMC
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "Peripheral.h"

namespace PERIPHERALS
{
  class CPeripheralAmbiPi : public CPeripheral
  {
  public:
    CPeripheralAmbiPi(const PeripheralType type, const PeripheralBusType busType, const CStdString &strLocation, const CStdString &strDeviceName, int iVendorId, int iProductId);
    virtual CPeripheralAmbiPi::~CPeripheralAmbiPi(void);

  protected:
	  bool InitialiseFeature(const PeripheralFeature feature);

    void CPeripheralAmbiPi::ConnectToDevice();
    void CPeripheralAmbiPi::LoadAddressFromConfiguration();

    bool                              m_bStarted;
    int                               m_port;
    CStdString                        m_address;
  };
}
