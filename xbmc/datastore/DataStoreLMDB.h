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

#pragma once

#include "IDataStore.h"

#include <map>
#include <utility>

struct MDB_env;
struct MDB_txn;

namespace KODI
{
namespace DATASTORE
{
class CDataStoreLMDB : public IDataStore
{
public:
  ~CDataStoreLMDB() override = default;

  // Implementation of IDataStore
  const char* Name() const override;
  bool Open(const std::string& dataStorePath) override;
  void Close() override;
  bool Has(const uint8_t* key, size_t keySize) override;
  bool Get(const uint8_t* key, size_t keySize, std::vector<uint8_t>& data) override;
  bool Reserve(const uint8_t* key, size_t keySize, uint8_t*& data) override;
  void Commit(const uint8_t* data) override;
  bool Put(const uint8_t* key, size_t keySize, const uint8_t* data, size_t dataSize) override;
  bool Delete(const uint8_t* key, size_t keySize) override;

private:
  MDB_txn* CreateTransaction();
  void CommitTransaction(MDB_txn* transaction);
  bool OpenDataStore(MDB_txn* transaction, unsigned int& databaseHandle);
  void CloseDataStore(unsigned int databaseHandle);

  // LMDB parameters
  MDB_env* m_environment = nullptr;
  using DataStoreHandle = std::pair<MDB_txn*, unsigned int>;
  std::map<const void*, DataStoreHandle> m_reservedData;
};
} // namespace DATASTORE
} // namespace KODI
