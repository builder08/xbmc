/*
 *      Copyright (C) 2014-present Team Kodi
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

#pragma once

#include <memory>

bool OMXPlayerUnsuitable(bool m_HasVideo, bool m_HasAudio, CDVDDemux* m_pDemuxer, std::shared_ptr<CDVDInputStream> m_pInputStream, CSelectionStreams &m_SelectionStreams);
bool OMXDoProcessing(struct SOmxPlayerState &m_OmxPlayerState, int m_playSpeed, IDVDStreamPlayerVideo *m_VideoPlayerVideo, IDVDStreamPlayerAudio *m_VideoPlayerAudio,
                     CCurrentStream m_CurrentAudio, CCurrentStream m_CurrentVideo, bool m_HasVideo, bool m_HasAudio, CProcessInfo &processInfo);
bool OMXStillPlaying(bool waitVideo, bool waitAudio, bool eosVideo, bool eosAudio);
