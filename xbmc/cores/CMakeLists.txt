set(SOURCES DataCacheCore.cpp
            FFmpeg.cpp
            PlaybackControl.cpp
            VideoSettings.cpp)

set(HEADERS DataCacheCore.h
            FFmpeg.h
            GameSettings.h
            IPlaybackControl.h
            IPlayer.h
            IPlayerCallback.h
            PlaybackControl.h
            VideoSettings.h)

if(CORE_PLATFORM_NAME_LC STREQUAL rbpi)
  add_subdirectory(omxplayer)
endif()

core_add_library(cores)
