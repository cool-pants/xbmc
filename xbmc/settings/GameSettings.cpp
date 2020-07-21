/*
 *  Copyright (C) 2017-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "GameSettings.h"

using namespace KODI;

CGameSettings &CGameSettings::operator=(const CGameSettings &rhs)
{
  if (this != &rhs)
  {
    m_videoFilter = rhs.m_videoFilter;
    m_stretchMode = rhs.m_stretchMode;
    m_rotationDegCCW = rhs.m_rotationDegCCW;
    m_saveDate = rhs.m_saveDate;
  }
  return *this;
}

void CGameSettings::Reset()
{
  m_videoFilter.clear();
  m_stretchMode = RETRO::STRETCHMODE::Normal;
  m_rotationDegCCW = 0;
  m_saveDate = "";
}

bool CGameSettings::operator==(const CGameSettings &rhs) const
{
  return m_videoFilter == rhs.m_videoFilter &&
         m_stretchMode == rhs.m_stretchMode &&
         m_rotationDegCCW == rhs.m_rotationDegCCW;
}

void CGameSettings::SetVideoFilter(const std::string &videoFilter)
{
  if (videoFilter != m_videoFilter)
  {
    m_videoFilter = videoFilter;
    SetChanged();
  }
}

void CGameSettings::SetStretchMode(RETRO::STRETCHMODE stretchMode)
{
  if (stretchMode != m_stretchMode)
  {
    m_stretchMode = stretchMode;
    SetChanged();
  }
}

void CGameSettings::SetRotationDegCCW(unsigned int rotation)
{
  if (rotation != m_rotationDegCCW)
  {
    m_rotationDegCCW = rotation;
    SetChanged();
  }
}

void CGameSettings::SetGameSaveDate(const std::string& saveDate)
{
  if (saveDate != m_saveDate)
  {
    m_saveDate = saveDate;
    SetChanged();
  }
}
