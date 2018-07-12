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

#include <memory>
#include <stdint.h>
#include <vector>

class CProfileManager;

namespace KODI
{
namespace DATASTORE
{
class IDataStore;

class CDataStore
{
public:
  CDataStore(const CProfileManager& profileManager);
  ~CDataStore();

  // Wrappers for IDataStore functions
  bool Open();
  void Close();
  bool Has(const uint8_t* key, size_t keySize);
  bool Get(const uint8_t* key, size_t keySize, std::vector<uint8_t>& data);
  bool Reserve(const uint8_t* key, size_t keySize, uint8_t*& data);
  void Commit(const uint8_t* data);
  bool Put(const uint8_t* key, size_t keySize, const uint8_t* data, size_t dataSize);
  bool Delete(const uint8_t* key, size_t keySize);

private:
  // Factory function
  std::unique_ptr<IDataStore> CreateDataStore();

  // Construction parameters
  const CProfileManager& m_profileManager;

  // Data store parameters
  std::unique_ptr<IDataStore> m_dataStore;
};
} // namespace DATASTORE
} // namespace KODI
