/*
 *      Copyright (C) 2016-2017 Team Kodi
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
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

#include "IPlayback.h"
#include "threads/CriticalSection.h"
#include "utils/Observer.h"

#include <memory>
#include <stddef.h>
#include <stdint.h>

namespace KODI
{
namespace RETRO
{
  class CGameClient;
  class CSavestateReader;
  class CSavestateWriter;
  class IMemoryStream;

  class CReversiblePlayback : public IPlayback,
                              public RETRO::IRetroPlayerClockCallback,
                              public Observer
  {
  public:
    CReversiblePlayback(CGameClient* gameClient, double fps, size_t serializeSize);

    virtual ~CReversiblePlayback();

    // implementation of IGameClientPlayback
    virtual bool CanPause() const override               { return true; }
    virtual bool CanSeek() const override                { return true; }
    virtual unsigned int GetTimeMs() const override      { return m_playTimeMs; }
    virtual unsigned int GetTotalTimeMs() const override { return m_totalTimeMs; }
    virtual unsigned int GetCacheTimeMs() const override { return m_cacheTimeMs; }
    virtual void SeekTimeMs(unsigned int timeMs) override;
    virtual double GetSpeed() const override;
    virtual void SetSpeed(double speedFactor) override;
    virtual std::string CreateSavestate() override;
    virtual bool LoadSavestate(const std::string& path) override;

    // implementation of IGameLoopCallback
    virtual void FrameEvent() override;
    virtual void RewindEvent() override;

    // implementation of Observer
    virtual void Notify(const Observable &obs, const ObservableMessage msg) override;

  private:
    void AddFrame();
    void RewindFrames(unsigned int frames);
    void AdvanceFrames(unsigned int frames);
    void UpdatePlaybackStats();

    // Construction parameter
    CGameClient* const m_gameClient;

    // Savestate functionality
    std::unique_ptr<CSavestateWriter> m_savestateWriter;
    std::unique_ptr<CSavestateReader> m_savestateReader;

    // Playback stats
    uint64_t     m_totalFrameCount;
    unsigned int m_pastFrameCount;
    unsigned int m_futureFrameCount;
    unsigned int m_playTimeMs;
    unsigned int m_totalTimeMs;
    unsigned int m_cacheTimeMs;
  };
}
}