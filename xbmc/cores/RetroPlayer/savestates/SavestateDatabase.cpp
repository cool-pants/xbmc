/*
 *  Copyright (C) 2012-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "SavestateDatabase.h"
#include "SavestateFlatBuffer.h"
#include "URL.h"
#include "FileItem.h" 
#include "filesystem/Directory.h"
#include "filesystem/File.h"
#include "utils/log.h"
#include "utils/URIUtils.h"
#include "XBDateTime.h"

constexpr auto SAVESTATE_EXTENSION = ".sav";
constexpr auto SAVESTATE_BASE_FOLDER = "special://home/saves/";

using namespace KODI;
using namespace RETRO;
using namespace XFILE;

CSavestateDatabase::CSavestateDatabase() = default;

std::unique_ptr<ISavestate> CSavestateDatabase::CreateSavestate()
{
  std::unique_ptr<ISavestate> savestate;

  savestate.reset(new CSavestateFlatBuffer);

  return savestate;
}

bool CSavestateDatabase::AddSavestate(std::string& savestatePath,
                                      const std::string& gamePath,
                                      const ISavestate& save)
{
  bool bSuccess = false;
  std::string path;

  if (savestatePath.empty())
  {
    path = MakePath(gamePath);
    path = URIUtils::AddFileToFolder(path, CDateTime::GetCurrentDateTime().GetAsSaveString() +
                                               SAVESTATE_EXTENSION);

    // return the path to the new savestate
    savestatePath = path;
  }
  else
  {
    path = savestatePath;
  }

  CLog::Log(LOGDEBUG, "Saving savestate to %s", CURL::GetRedacted(path).c_str());

  const uint8_t* data = nullptr;
  size_t size = 0;
  if (save.Serialize(data, size))
  {
    XFILE::CFile file;
    if (file.OpenForWrite(path))
    {
      const ssize_t written = file.Write(data, size);
      if (written == static_cast<ssize_t>(size))
      {
        CLog::Log(LOGDEBUG, "Wrote savestate of %u bytes", size);
        bSuccess = true;
      }
    }
    else
      CLog::Log(LOGERROR, "Failed to open savestate for writing");
  }

  return bSuccess;
}

bool CSavestateDatabase::GetSavestate(const std::string& savestatePath, ISavestate& save)
{
  bool bSuccess = false;

  CLog::Log(LOGDEBUG, "Loading savestate from %s", CURL::GetRedacted(savestatePath).c_str());

  std::vector<uint8_t> savestateData;

  XFILE::CFile savestateFile;
  if (savestateFile.Open(savestatePath, XFILE::READ_TRUNCATED))
  {
    int64_t size = savestateFile.GetLength();
    if (size > 0)
    {
      savestateData.resize(static_cast<size_t>(size));

      const ssize_t readLength = savestateFile.Read(savestateData.data(), savestateData.size());
      if (readLength != static_cast<ssize_t>(savestateData.size()))
      {
        CLog::Log(LOGERROR, "Failed to read savestate %s of size %d bytes",
                  CURL::GetRedacted(savestatePath).c_str(), size);
        savestateData.clear();
      }
    }
    else
      CLog::Log(LOGERROR, "Failed to get savestate length: %s",
                CURL::GetRedacted(savestatePath).c_str());
  }
  else
    CLog::Log(LOGERROR, "Failed to open savestate file %s",
              CURL::GetRedacted(savestatePath).c_str());

  if (!savestateData.empty())
    bSuccess = save.Deserialize(std::move(savestateData));

  return bSuccess;
}

bool CSavestateDatabase::GetSavestatesNav(CFileItemList& items,
                                          const std::string& gamePath,
                                          const std::string& gameClient /* = "" */)
{
  const std::string savesFolder = MakePath(gamePath);

  CDirectory::CHints hints;
  hints.mask = SAVESTATE_EXTENSION;

  if (!CDirectory::GetDirectory(savesFolder, items, hints))
    return false;

  if (!gameClient.empty())
  {
    for (int i = items.Size() - 1; i >= 0; i--)
    {
      std::unique_ptr<ISavestate> save = CreateSavestate();
      GetSavestate(items[i]->GetPath(), *save);
      if (save->GameClientID() != gameClient)
        items.Remove(i);
    }
  }

  for (int i = 0; i < items.Size(); i++)
  {
    std::unique_ptr<ISavestate> savestate = CreateSavestate();
    GetSavestate(items[i]->GetPath(), *savestate);

    const std::string label = savestate->Label();
    const CDateTime created = savestate->Created();

    if (label.empty())
      items[i]->SetLabel(created.GetAsLocalizedDateTime());
    else
    {
      items[i]->SetLabel(label);
      items[i]->SetLabel2(created.GetAsLocalizedDateTime());
    }

    items[i]->SetIconImage(MakeThumbnailPath(items[i]->GetPath()));

    items[i]->SetProperty("game.savedate", created.GetAsLocalizedDateTime());
  }

  return true;
}

bool CSavestateDatabase::RenameSavestate(const std::string& savestatePath, const std::string& label)
{
  std::unique_ptr<ISavestate> savestate = CreateSavestate();
  if (!GetSavestate(savestatePath, *savestate))
    return false;

  std::unique_ptr<ISavestate> newSavestate = CreateSavestate();

  newSavestate->SetLabel(label);
  newSavestate->SetType(savestate->Type());
  newSavestate->SetCreated(savestate->Created());
  newSavestate->SetGameFileName(savestate->GameFileName());
  newSavestate->SetTimestampFrames(savestate->TimestampFrames());
  newSavestate->SetTimestampWallClock(savestate->TimestampWallClock());
  newSavestate->SetGameClientID(savestate->GameClientID());
  newSavestate->SetGameClientVersion(savestate->GameClientVersion());

  size_t memorySize = savestate->GetMemorySize();
  std::memcpy(newSavestate->GetMemoryBuffer(memorySize), savestate->GetMemoryData(),
              memorySize);

  newSavestate->Finalize();

  std::string path = savestatePath;
  if (!AddSavestate(path, "", *newSavestate))
    return false;

  return true;
}

bool CSavestateDatabase::DeleteSavestate(const std::string& savestatePath)
{
  if (!CFile::Delete(savestatePath))
  {
    CLog::Log(LOGERROR, "Failed to delete savestate file %s", CURL::GetRedacted(savestatePath).c_str());
    return false;
  }

  CFile::Delete(MakeThumbnailPath(savestatePath));
  return true;
}

bool CSavestateDatabase::ClearSavestatesOfGame(const std::string& gamePath,
                                               const std::string& gameClient /* = "" */)
{
  //! @todo
  return false;
}

std::string CSavestateDatabase::MakeThumbnailPath(const std::string& savestatePath)
{
  return URIUtils::ReplaceExtension(savestatePath, ".jpg");
}

std::string CSavestateDatabase::MakePath(const std::string& gamePath)
{
  if (!CreateFolderIfNotExists(SAVESTATE_BASE_FOLDER))
    return "";

  std::string gameName = URIUtils::GetFileName(gamePath);
  std::string folderPath = SAVESTATE_BASE_FOLDER + gameName;

  if (!CreateFolderIfNotExists(folderPath))
    return "";

  return folderPath;
}

bool CSavestateDatabase::CreateFolderIfNotExists(const std::string& path)
{
  if (!CDirectory::Exists(path))
  {
    if (!CDirectory::Create(path))
    {
      CLog::Log(LOGERROR, "Failed to create folder: %s", path);
      return false;
    }
  }

  return true;
}
