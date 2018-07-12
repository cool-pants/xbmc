/*
 *      Copyright (C) 2018 Team Kodi
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

#include "DataStore.h"
#include "DataStoreLMDB.h"
#include "IDataStore.h"
#include "URL.h"
#include "filesystem/SpecialProtocol.h"
#include "profiles/ProfileManager.h"
#include "utils/log.h"

using namespace KODI;
using namespace DATASTORE;

CDataStore::CDataStore(const CProfileManager& profileManager)
    : m_profileManager(profileManager)
{
}

CDataStore::~CDataStore() = default;

bool CDataStore::Open()
{
  std::unique_ptr<IDataStore> dataStore = CreateDataStore();
  if (!dataStore)
    return false;

  std::string dataStorePath =
      CSpecialProtocol::TranslatePath(m_profileManager.GetDataStoreFolder());
  CLog::Log(LOGINFO, "Opening %s data store at %s", dataStore->Name(),
            CURL::GetRedacted(dataStorePath).c_str());
  if (!dataStore->Open(dataStorePath))
    return false;

  m_dataStore = std::move(dataStore);
  return true;
}

void CDataStore::Close()
{
  if (m_dataStore)
  {
    m_dataStore->Close();
    m_dataStore.reset();
  }
}

bool CDataStore::Has(const uint8_t* key, size_t keySize)
{
  if (m_dataStore)
    return m_dataStore->Has(key, keySize);

  return false;
}

bool CDataStore::Get(const uint8_t* key, size_t keySize, std::vector<uint8_t>& data)
{
  if (m_dataStore)
    return m_dataStore->Get(key, keySize, data);

  return false;
}

bool CDataStore::Reserve(const uint8_t* key, size_t keySize, uint8_t*& data)
{
  if (m_dataStore)
    return m_dataStore->Reserve(key, keySize, data);

  return false;
}

void CDataStore::Commit(const uint8_t* data)
{
  if (m_dataStore)
    m_dataStore->Commit(data);
}

bool CDataStore::Put(const uint8_t* key, size_t keySize, const uint8_t* data, size_t dataSize)
{
  if (m_dataStore)
    return m_dataStore->Put(key, keySize, data, dataSize);

  return false;
}

bool CDataStore::Delete(const uint8_t* key, size_t keySize)
{
  if (m_dataStore)
    return m_dataStore->Delete(key, keySize);

  return false;
}

std::unique_ptr<IDataStore> CDataStore::CreateDataStore()
{
  std::unique_ptr<IDataStore> dataStore;

  dataStore.reset(new CDataStoreLMDB);

  return dataStore;
}
