#pragma once
/*
 *      Copyright (C) 2015-2016 Team KODI
 *      http://kodi.tv
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
 *  along with KODI; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

API_NAMESPACE

namespace KodiAPI
{
namespace GUI
{
  class CWindow;

  //============================================================================
  ///
  /// \defgroup CPP_KodiAPI_GUI_CControlLabel Control Label
  /// \ingroup CPP_KodiAPI_GUI
  /// @{
  /// @brief <b>Window control used to show some lines of text.</b>
  ///
  /// The  label control  is used for  displaying text  in Kodi.  You can choose
  /// the font, size, colour, location and contents of the text to be displayed.
  ///
  /// It has the header \ref ControlLabel.hpp "#include <kodi/api3/gui/ControlLabel.hpp>"
  /// be included to enjoy it.
  ///
  /// Here you find the needed skin part for a \ref Label_Control "label control"
  ///
  /// @note The  call  of the control  is only possible  from the  corresponding
  /// window as its class and identification number is required.
  ///
  class CControlLabel
  {
  public:
    //==========================================================================
    ///
    /// \ingroup CPP_KodiAPI_GUI_CControlLabel
    /// @brief Construct a new control
    ///
    /// @param[in] window               related window control class
    /// @param[in] controlId            Used skin xml control id
    ///
    CControlLabel(CWindow* window, int controlId);
    //--------------------------------------------------------------------------


    //==========================================================================
    ///
    /// \ingroup CPP_KodiAPI_GUI_CControlLabel
    /// @brief Destructor
    ///
    virtual ~CControlLabel();
    //--------------------------------------------------------------------------


    //==========================================================================
    ///
    /// \ingroup CPP_KodiAPI_GUI_CControlLabel
    /// @brief Set the control on window to visible
    ///
    /// @param[in] visible              If true visible, otherwise hidden
    ///
    void SetVisible(bool visible);
    //--------------------------------------------------------------------------


    //==========================================================================
    ///
    /// \ingroup CPP_KodiAPI_GUI_CControlLabel
    /// @brief To set the text string on label
    ///
    /// @param[in] text                 Text to show
    ///
    void SetLabel(const std::string& text);
    //--------------------------------------------------------------------------


    //==========================================================================
    ///
    /// \ingroup CPP_KodiAPI_GUI_CControlLabel
    /// @brief Get the used text from control
    ///
    /// @return                         Used text on label control
    ///
    std::string GetLabel() const;
    //--------------------------------------------------------------------------

  #ifndef DOXYGEN_SHOULD_SKIP_THIS
    IMPL_GUI_LABEL_CONTROL;
  #endif
  };
  /// @}

} /* namespace GUI */
} /* namespace KodiAPI */

END_NAMESPACE()
