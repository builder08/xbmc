/*
 *      Copyright (C) 2012-present Team Kodi
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

#include "PlayerController.h"
#include "ServiceBroker.h"
#include "dialogs/GUIDialogSelect.h"
#include "dialogs/GUIDialogSlider.h"
#include "settings/AdvancedSettings.h"
#include "settings/DisplaySettings.h"
#include "settings/MediaSettings.h"
#include "settings/Settings.h"
#include "cores/IPlayer.h"
#include "input/Key.h"
#include "guilib/GUIComponent.h"
#include "guilib/LocalizeStrings.h"
#include "guilib/GUISliderControl.h"
#include "guilib/GUIWindowManager.h"
#include "dialogs/GUIDialogKaiToast.h"
#include "video/dialogs/GUIDialogAudioSettings.h"
#include "cores/VideoPlayer/VideoRenderers/OverlayRendererGUI.h"
#include "Application.h"
#include "utils/LangCodeExpander.h"
#include "utils/StringUtils.h"
#include "utils/Variant.h"

CPlayerController::CPlayerController()
  : m_sliderAction(0)
{
}

CPlayerController::~CPlayerController() = default;

CPlayerController& CPlayerController::GetInstance()
{
  static CPlayerController instance;
  return instance;
}

bool CPlayerController::OnAction(const CAction &action)
{
  const unsigned int MsgTime = 300;
  const unsigned int DisplTime = 2000;

  if (g_application.GetAppPlayer().IsPlayingVideo())
  {
    switch (action.GetID())
    {
      case ACTION_SHOW_SUBTITLES:
      {
        if (g_application.GetAppPlayer().GetSubtitleCount() == 0)
          return true;

        bool subsOn = !g_application.GetAppPlayer().GetSubtitleVisible();
        g_application.GetAppPlayer().SetSubtitleVisible(subsOn);
        std::string sub, lang;
        if (subsOn)
        {
          SubtitleStreamInfo info;
          g_application.GetAppPlayer().GetSubtitleStreamInfo(g_application.GetAppPlayer().GetSubtitle(), info);
          if (!g_LangCodeExpander.Lookup(info.language, lang))
            lang = g_localizeStrings.Get(13205); // Unknown

          if (info.name.length() == 0)
            sub = lang;
          else
            sub = StringUtils::Format("%s - %s", lang.c_str(), info.name.c_str());
        }
        else
          sub = g_localizeStrings.Get(1223);
        CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info,
                                              g_localizeStrings.Get(287), sub, DisplTime, false, MsgTime);
        return true;
      }

      case ACTION_NEXT_SUBTITLE:
      case ACTION_CYCLE_SUBTITLE:
      {
        if (g_application.GetAppPlayer().GetSubtitleCount() == 0)
          return true;

        int currentSub = g_application.GetAppPlayer().GetSubtitle();
        bool currentSubVisible = true;

        if (g_application.GetAppPlayer().GetSubtitleVisible())
        {
          if (++currentSub >= g_application.GetAppPlayer().GetSubtitleCount())
          {
            currentSub = 0;
            if (action.GetID() == ACTION_NEXT_SUBTITLE)
            {
              g_application.GetAppPlayer().SetSubtitleVisible(false);
              currentSubVisible = false;
            }
          }
          g_application.GetAppPlayer().SetSubtitle(currentSub);
        }
        else if (action.GetID() == ACTION_NEXT_SUBTITLE)
        {
          g_application.GetAppPlayer().SetSubtitleVisible(true);
        }

        std::string sub, lang;
        if (currentSubVisible)
        {
          SubtitleStreamInfo info;
          g_application.GetAppPlayer().GetSubtitleStreamInfo(currentSub, info);
          if (!g_LangCodeExpander.Lookup(info.language, lang))
            lang = g_localizeStrings.Get(13205); // Unknown

          if (info.name.length() == 0)
            sub = lang;
          else
            sub = StringUtils::Format("%s - %s", lang.c_str(), info.name.c_str());
        }
        else
          sub = g_localizeStrings.Get(1223);
        CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, g_localizeStrings.Get(287), sub, DisplTime, false, MsgTime);
        return true;
      }

      case ACTION_SUBTITLE_DELAY_MIN:
      {
        CVideoSettings vs = g_application.GetAppPlayer().GetVideoSettings();
        vs.m_SubtitleDelay -= 0.1f;
        if (vs.m_SubtitleDelay < -g_advancedSettings.m_videoSubsDelayRange)
          vs.m_SubtitleDelay = -g_advancedSettings.m_videoSubsDelayRange;
        g_application.GetAppPlayer().SetSubTitleDelay(vs.m_SubtitleDelay);

        ShowSlider(action.GetID(), 22006, g_application.GetAppPlayer().GetVideoSettings().m_SubtitleDelay,
                                          -g_advancedSettings.m_videoSubsDelayRange, 0.1f,
                                           g_advancedSettings.m_videoSubsDelayRange);
        return true;
      }

      case ACTION_SUBTITLE_DELAY_PLUS:
      {
        CVideoSettings vs = g_application.GetAppPlayer().GetVideoSettings();
        vs.m_SubtitleDelay += 0.1f;
        if (vs.m_SubtitleDelay > g_advancedSettings.m_videoSubsDelayRange)
          vs.m_SubtitleDelay = g_advancedSettings.m_videoSubsDelayRange;
        g_application.GetAppPlayer().SetSubTitleDelay(vs.m_SubtitleDelay);

        ShowSlider(action.GetID(), 22006, g_application.GetAppPlayer().GetVideoSettings().m_SubtitleDelay,
                                          -g_advancedSettings.m_videoSubsDelayRange, 0.1f,
                                           g_advancedSettings.m_videoSubsDelayRange);
        return true;
      }

      case ACTION_SUBTITLE_DELAY:
      {
        ShowSlider(action.GetID(), 22006, g_application.GetAppPlayer().GetVideoSettings().m_SubtitleDelay,
                                          -g_advancedSettings.m_videoSubsDelayRange, 0.1f,
                                           g_advancedSettings.m_videoSubsDelayRange, true);
        return true;
      }

      case ACTION_AUDIO_DELAY:
      {
        ShowSlider(action.GetID(), 297, g_application.GetAppPlayer().GetVideoSettings().m_AudioDelay,
                                        -g_advancedSettings.m_videoAudioDelayRange, 0.025f,
                                         g_advancedSettings.m_videoAudioDelayRange, true);
        return true;
      }

      case ACTION_AUDIO_DELAY_MIN:
      {
        CVideoSettings vs = g_application.GetAppPlayer().GetVideoSettings();
        vs.m_AudioDelay -= 0.025f;
        if (vs.m_AudioDelay < -g_advancedSettings.m_videoAudioDelayRange)
          vs.m_AudioDelay = -g_advancedSettings.m_videoAudioDelayRange;
        g_application.GetAppPlayer().SetAVDelay(vs.m_AudioDelay);

        ShowSlider(action.GetID(), 297, g_application.GetAppPlayer().GetVideoSettings().m_AudioDelay,
                                        -g_advancedSettings.m_videoAudioDelayRange, 0.025f,
                                         g_advancedSettings.m_videoAudioDelayRange);
        return true;
      }

      case ACTION_AUDIO_DELAY_PLUS:
      {
        CVideoSettings vs = g_application.GetAppPlayer().GetVideoSettings();
        vs.m_AudioDelay += 0.025f;
        if (vs.m_AudioDelay > g_advancedSettings.m_videoAudioDelayRange)
          vs.m_AudioDelay = g_advancedSettings.m_videoAudioDelayRange;
        g_application.GetAppPlayer().SetAVDelay(vs.m_AudioDelay);

        ShowSlider(action.GetID(), 297, g_application.GetAppPlayer().GetVideoSettings().m_AudioDelay,
                                        -g_advancedSettings.m_videoAudioDelayRange, 0.025f,
                                         g_advancedSettings.m_videoAudioDelayRange);
        return true;
      }

      case ACTION_AUDIO_NEXT_LANGUAGE:
      {
        if (g_application.GetAppPlayer().GetAudioStreamCount() == 1)
          return true;

        int currentAudio = g_application.GetAppPlayer().GetAudioStream();

        if (++currentAudio >= g_application.GetAppPlayer().GetAudioStreamCount())
          currentAudio = 0;
        g_application.GetAppPlayer().SetAudioStream(currentAudio);    // Set the audio stream to the one selected
        std::string aud;
        std::string lan;
        AudioStreamInfo info;
        g_application.GetAppPlayer().GetAudioStreamInfo(currentAudio, info);
        if (!g_LangCodeExpander.Lookup(info.language, lan))
          lan = g_localizeStrings.Get(13205); // Unknown
        if (info.name.empty())
          aud = lan;
        else
          aud = StringUtils::Format("%s - %s", lan.c_str(), info.name.c_str());
        CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, g_localizeStrings.Get(460), aud, DisplTime, false, MsgTime);
        return true;
      }

      case ACTION_ZOOM_IN:
      {
        CVideoSettings vs = g_application.GetAppPlayer().GetVideoSettings();
        vs.m_CustomZoomAmount += 0.01f;
        if (vs.m_CustomZoomAmount > 2.f)
          vs.m_CustomZoomAmount = 2.f;
        vs.m_ViewMode = ViewModeCustom;
        g_application.GetAppPlayer().SetRenderViewMode(ViewModeCustom, vs.m_CustomZoomAmount,
                                                   vs.m_CustomPixelRatio, vs.m_CustomVerticalShift,
                                                   vs.m_CustomNonLinStretch);
        ShowSlider(action.GetID(), 216, vs.m_CustomZoomAmount, 0.5f, 0.1f, 2.0f);
        return true;
      }

      case ACTION_ZOOM_OUT:
      {
        CVideoSettings vs = g_application.GetAppPlayer().GetVideoSettings();
        vs.m_CustomZoomAmount -= 0.01f;
        if (vs.m_CustomZoomAmount < 0.5f)
          vs.m_CustomZoomAmount = 0.5f;
        vs.m_ViewMode = ViewModeCustom;
        g_application.GetAppPlayer().SetRenderViewMode(ViewModeCustom, vs.m_CustomZoomAmount,
                                                   vs.m_CustomPixelRatio, vs.m_CustomVerticalShift,
                                                   vs.m_CustomNonLinStretch);
        ShowSlider(action.GetID(), 216, vs.m_CustomZoomAmount, 0.5f, 0.1f, 2.0f);
        return true;
      }

      case ACTION_INCREASE_PAR:
      {
        CVideoSettings vs = g_application.GetAppPlayer().GetVideoSettings();
        vs.m_CustomPixelRatio += 0.01f;
        if (vs.m_CustomPixelRatio > 2.f)
          vs.m_CustomPixelRatio = 2.f;
        vs.m_ViewMode = ViewModeCustom;
        g_application.GetAppPlayer().SetRenderViewMode(ViewModeCustom, vs.m_CustomZoomAmount,
                                                   vs.m_CustomPixelRatio, vs.m_CustomVerticalShift,
                                                   vs.m_CustomNonLinStretch);
        ShowSlider(action.GetID(), 217, vs.m_CustomPixelRatio, 0.5f, 0.1f, 2.0f);
        return true;
      }

      case ACTION_DECREASE_PAR:
      {
        CVideoSettings vs = g_application.GetAppPlayer().GetVideoSettings();
        vs.m_CustomPixelRatio -= 0.01f;
        if (vs.m_CustomPixelRatio < 0.5f)
          vs.m_CustomPixelRatio = 0.5f;
        vs.m_ViewMode = ViewModeCustom;
        g_application.GetAppPlayer().SetRenderViewMode(ViewModeCustom, vs.m_CustomZoomAmount,
                                                   vs.m_CustomPixelRatio, vs.m_CustomVerticalShift,
                                                   vs.m_CustomNonLinStretch);
        ShowSlider(action.GetID(), 217, vs.m_CustomPixelRatio, 0.5f, 0.1f, 2.0f);
        return true;
      }

      case ACTION_VSHIFT_UP:
      {
        CVideoSettings vs = g_application.GetAppPlayer().GetVideoSettings();
        vs.m_CustomVerticalShift -= 0.01f;
        if (vs.m_CustomVerticalShift < -2.0f)
          vs.m_CustomVerticalShift = -2.0f;
        vs.m_ViewMode = ViewModeCustom;
        g_application.GetAppPlayer().SetRenderViewMode(ViewModeCustom, vs.m_CustomZoomAmount,
                                                   vs.m_CustomPixelRatio, vs.m_CustomVerticalShift,
                                                   vs.m_CustomNonLinStretch);
        ShowSlider(action.GetID(), 225, vs.m_CustomVerticalShift, -2.0f, 0.1f, 2.0f);
        return true;
      }

      case ACTION_VSHIFT_DOWN:
      {
        CVideoSettings vs = g_application.GetAppPlayer().GetVideoSettings();
        vs.m_CustomVerticalShift += 0.01f;
        if (vs.m_CustomVerticalShift > 2.0f)
          vs.m_CustomVerticalShift = 2.0f;
        vs.m_ViewMode = ViewModeCustom;
        g_application.GetAppPlayer().SetRenderViewMode(ViewModeCustom, vs.m_CustomZoomAmount,
                                                   vs.m_CustomPixelRatio, vs.m_CustomVerticalShift,
                                                   vs.m_CustomNonLinStretch);
        ShowSlider(action.GetID(), 225, vs.m_CustomVerticalShift, -2.0f, 0.1f, 2.0f);
        return true;
      }

      case ACTION_SUBTITLE_VSHIFT_UP:
      {
        RESOLUTION_INFO res_info = CServiceBroker::GetWinSystem()->GetGfxContext().GetResInfo();
        int subalign = CServiceBroker::GetSettings().GetInt(CSettings::SETTING_SUBTITLES_ALIGN);
        if ((subalign == SUBTITLE_ALIGN_BOTTOM_OUTSIDE) || (subalign == SUBTITLE_ALIGN_TOP_INSIDE))
        {
          res_info.iSubtitles ++;
          if (res_info.iSubtitles >= res_info.iHeight)
            res_info.iSubtitles = res_info.iHeight - 1;

          ShowSlider(action.GetID(), 274, (float) res_info.iHeight - res_info.iSubtitles, 0.0f, 1.0f, (float) res_info.iHeight);
        }
        else
        {
          res_info.iSubtitles --;
          if (res_info.iSubtitles < 0)
            res_info.iSubtitles = 0;

          if (subalign == SUBTITLE_ALIGN_MANUAL)
            ShowSlider(action.GetID(), 274, (float) res_info.iSubtitles, 0.0f, 1.0f, (float) res_info.iHeight);
          else
            ShowSlider(action.GetID(), 274, (float) res_info.iSubtitles - res_info.iHeight, (float) -res_info.iHeight, -1.0f, 0.0f);
        }
        CServiceBroker::GetWinSystem()->GetGfxContext().SetResInfo(CServiceBroker::GetWinSystem()->GetGfxContext().GetVideoResolution(), res_info);
        return true;
      }

      case ACTION_SUBTITLE_VSHIFT_DOWN:
      {
        RESOLUTION_INFO res_info =  CServiceBroker::GetWinSystem()->GetGfxContext().GetResInfo();
        int subalign = CServiceBroker::GetSettings().GetInt(CSettings::SETTING_SUBTITLES_ALIGN);
        if ((subalign == SUBTITLE_ALIGN_BOTTOM_OUTSIDE) || (subalign == SUBTITLE_ALIGN_TOP_INSIDE))
        {
          res_info.iSubtitles--;
          if (res_info.iSubtitles < 0)
            res_info.iSubtitles = 0;

          ShowSlider(action.GetID(), 274, (float) res_info.iHeight - res_info.iSubtitles, 0.0f, 1.0f, (float) res_info.iHeight);
        }
        else
        {
          res_info.iSubtitles++;
          if (res_info.iSubtitles >= res_info.iHeight)
            res_info.iSubtitles = res_info.iHeight - 1;

          if (subalign == SUBTITLE_ALIGN_MANUAL)
            ShowSlider(action.GetID(), 274, (float) res_info.iSubtitles, 0.0f, 1.0f, (float) res_info.iHeight);
          else
            ShowSlider(action.GetID(), 274, (float) res_info.iSubtitles - res_info.iHeight, (float) -res_info.iHeight, -1.0f, 0.0f);
        }
        CServiceBroker::GetWinSystem()->GetGfxContext().SetResInfo(CServiceBroker::GetWinSystem()->GetGfxContext().GetVideoResolution(), res_info);
        return true;
      }

      case ACTION_SUBTITLE_ALIGN:
      {
        RESOLUTION_INFO res_info = CServiceBroker::GetWinSystem()->GetGfxContext().GetResInfo();
        int subalign = CServiceBroker::GetSettings().GetInt(CSettings::SETTING_SUBTITLES_ALIGN);

        subalign++;
        if (subalign > SUBTITLE_ALIGN_TOP_OUTSIDE)
          subalign = SUBTITLE_ALIGN_MANUAL;

        res_info.iSubtitles = res_info.iHeight - 1;

        CServiceBroker::GetSettings().SetInt(CSettings::SETTING_SUBTITLES_ALIGN, subalign);
        CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info,
                                              g_localizeStrings.Get(21460),
                                              g_localizeStrings.Get(21461 + subalign), 
                                              TOAST_DISPLAY_TIME, false);
        CServiceBroker::GetWinSystem()->GetGfxContext().SetResInfo(CServiceBroker::GetWinSystem()->GetGfxContext().GetVideoResolution(), res_info);
        return true;
      }

      case ACTION_VOLAMP_UP:
      case ACTION_VOLAMP_DOWN:
      {
        // Don't allow change with passthrough audio
        if (g_application.GetAppPlayer().IsPassthrough())
        {
          CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Warning,
                                                g_localizeStrings.Get(660),
                                                g_localizeStrings.Get(29802),
                                                TOAST_DISPLAY_TIME, false);
          return false;
        }

        float sliderMax = VOLUME_DRC_MAXIMUM / 100.0f;
        float sliderMin = VOLUME_DRC_MINIMUM / 100.0f;

        CVideoSettings vs = g_application.GetAppPlayer().GetVideoSettings();
        if (action.GetID() == ACTION_VOLAMP_UP)
          vs.m_VolumeAmplification += 1.0f;
        else
          vs.m_VolumeAmplification -= 1.0f;

        vs.m_VolumeAmplification =
          std::max(std::min(vs.m_VolumeAmplification, sliderMax), sliderMin);

        g_application.GetAppPlayer().SetDynamicRangeCompression((long)(vs.m_VolumeAmplification * 100));

        ShowSlider(action.GetID(), 660, vs.m_VolumeAmplification, sliderMin, 1.0f, sliderMax);
        return true;
      }

      case ACTION_VOLAMP:
      {
        float sliderMax = VOLUME_DRC_MAXIMUM / 100.0f;
        float sliderMin = VOLUME_DRC_MINIMUM / 100.0f;
        ShowSlider(action.GetID(), 660,
                   g_application.GetAppPlayer().GetVideoSettings().m_VolumeAmplification,
                   sliderMin, 1.0f, sliderMax, true);
        return true;
      }

      case ACTION_PLAYER_PROGRAM_SELECT:
      {
        std::vector<ProgramInfo> programs;
        g_application.GetAppPlayer().GetPrograms(programs);
        CGUIDialogSelect *dialog = CServiceBroker::GetGUI()->GetWindowManager().GetWindow<CGUIDialogSelect>(WINDOW_DIALOG_SELECT);
        if (dialog)
        {
          int playing = 0;
          int idx = 0;
          for (auto prog : programs)
          {
            dialog->Add(prog.name);
            if (prog.playing)
              playing = idx;
            idx++;
          }
          dialog->SetHeading(CVariant{g_localizeStrings.Get(39109)});
          dialog->SetSelected(playing);
          dialog->Open();
          idx = dialog->GetSelectedItem();
          if (idx > 0)
            g_application.GetAppPlayer().SetProgram(programs[idx].id);
        }
        return true;
      }

      case ACTION_PLAYER_RESOLUTION_SELECT:
      {
        std::vector<CVariant> indexList = CServiceBroker::GetSettings().GetList(CSettings::SETTING_VIDEOSCREEN_WHITELIST);

        CGUIDialogSelect *dialog = CServiceBroker::GetGUI()->GetWindowManager().GetWindow<CGUIDialogSelect>(WINDOW_DIALOG_SELECT);
        if (dialog)
        {
          int current = 0;
          int idx = 0;
          auto currentRes = CServiceBroker::GetWinSystem()->GetGfxContext().GetVideoResolution();
          for (const CVariant &mode : indexList)
          {
            auto res = CDisplaySettings::GetInstance().GetResFromString(mode.asString());
            const RESOLUTION_INFO info = CServiceBroker::GetWinSystem()->GetGfxContext().GetResInfo(res);
            dialog->Add(info.strMode);
            if (res == currentRes)
              current = idx;
            idx++;
          }
          dialog->SetHeading(CVariant{g_localizeStrings.Get(39110)});
          dialog->SetSelected(current);
          dialog->Open();
          idx = dialog->GetSelectedItem();
          if (idx >= 0)
          {
            auto res = CDisplaySettings::GetInstance().GetResFromString(indexList[idx].asString());
            CServiceBroker::GetWinSystem()->GetGfxContext().SetVideoResolution(res, false);
          }
        }
        return true;
      }

      default:
        break;
    }
  }
  return false;
}

void CPlayerController::ShowSlider(int action, int label, float value, float min, float delta, float max, bool modal)
{
  m_sliderAction = action;
  if (modal)
    CGUIDialogSlider::ShowAndGetInput(g_localizeStrings.Get(label), value, min, delta, max, this);
  else
    CGUIDialogSlider::Display(label, value, min, delta, max, this);
}

void CPlayerController::OnSliderChange(void *data, CGUISliderControl *slider)
{
  if (!slider)
    return;

  if (m_sliderAction == ACTION_ZOOM_OUT || m_sliderAction == ACTION_ZOOM_IN ||
      m_sliderAction == ACTION_INCREASE_PAR || m_sliderAction == ACTION_DECREASE_PAR ||
      m_sliderAction == ACTION_VSHIFT_UP || m_sliderAction == ACTION_VSHIFT_DOWN ||
      m_sliderAction == ACTION_SUBTITLE_VSHIFT_UP || m_sliderAction == ACTION_SUBTITLE_VSHIFT_DOWN)
  {
    std::string strValue = StringUtils::Format("%1.2f",slider->GetFloatValue());
    slider->SetTextValue(strValue);
  }
  else if (m_sliderAction == ACTION_VOLAMP_UP ||
          m_sliderAction == ACTION_VOLAMP_DOWN ||
          m_sliderAction == ACTION_VOLAMP)
    slider->SetTextValue(CGUIDialogAudioSettings::FormatDecibel(slider->GetFloatValue()));
  else
    slider->SetTextValue(CGUIDialogAudioSettings::FormatDelay(slider->GetFloatValue(), 0.025f));

  if (g_application.GetAppPlayer().HasPlayer())
  {
    if (m_sliderAction == ACTION_AUDIO_DELAY)
    {
      g_application.GetAppPlayer().SetAVDelay(slider->GetFloatValue());
    }
    else if (m_sliderAction == ACTION_SUBTITLE_DELAY)
    {
      g_application.GetAppPlayer().SetSubTitleDelay(slider->GetFloatValue());
    }
    else if (m_sliderAction == ACTION_VOLAMP)
    {
      g_application.GetAppPlayer().SetDynamicRangeCompression((long)(slider->GetFloatValue() * 100));
    }
  }
}
