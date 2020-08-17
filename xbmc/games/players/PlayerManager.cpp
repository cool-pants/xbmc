/*
 *      Copyright (C) 2017 Team Kodi
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

#include "PlayerManager.h"
#include "input/InputManager.h"
#include "ServiceBroker.h"
#include "peripherals/devices/Peripheral.h"
#include "peripherals/Peripherals.h"
#include "games/addons/GameClient.h"
//#include "games/controllers/ControllerTypes.h"
#include "addons/kodi-addon-dev-kit/include/kodi/kodi_game_types.h"
#include "games/addons/input/GameClientController.h"
#include "games/addons/input/GameClientHardware.h"
#include "games/addons/input/GameClientJoystick.h"
#include "games/addons/input/GameClientKeyboard.h"
#include "games/addons/input/GameClientMouse.h"
#include "games/addons/input/GameClientPort.h"
#include "games/addons/input/GameClientTopology.h"
#include "games/GameServices.h"
#include "games/addons/GameClient.h"
#include "games/addons/GameClientCallbacks.h"
#include "games/controllers/Controller.h"
#include "games/controllers/ControllerTopology.h"
#include "input/joysticks/JoystickTypes.h"
#include "peripherals/EventLockHandle.h"
#include "threads/SingleLock.h"
#include "utils/log.h"

using namespace KODI;
using namespace GAME;

CPlayerManager::CPlayerManager(PERIPHERALS::CPeripherals& peripheralManager,
                               CInputManager& inputManager)
    : m_peripheralManager(peripheralManager)
    , m_inputManager(inputManager)
{
  m_peripheralManager.RegisterObserver(this);
  m_inputManager.RegisterKeyboardDriverHandler(this);
  m_inputManager.RegisterMouseDriverHandler(this);
}

CPlayerManager::~CPlayerManager()
{
  m_inputManager.UnregisterMouseDriverHandler(this);
  m_inputManager.UnregisterKeyboardDriverHandler(this);
  m_peripheralManager.UnregisterObserver(this);
}

void CPlayerManager::Notify(const Observable& obs, const ObservableMessage msg)
{
  switch (msg)
  {
  case ObservableMessagePeripheralsChanged:
  {
    OnJoystickEvent();
    break;
  }
  default:
    break;
  }
}

bool CPlayerManager::OpenKeyboard(CGameClientSubsystem& gameSub, ControllerPtr controller, 
                                  AddonInstance_Game& m_struct, PERIPHERALS::PeripheralVector& keyboards)
{
  CServiceBroker::GetPeripherals().GetPeripheralsWithFeature(keyboards,
                                                             PERIPHERALS::FEATURE_KEYBOARD);
  if (keyboards.empty())
    return false;

  bool bSuccess = false;

  {
    CSingleLock lock(gameSub.GetAccess());

    if (gameSub.GetClient().Initialized())
    {
      try
      {
        bSuccess = m_struct.toAddon.EnableKeyboard(true, controller->ID().c_str());
      }
      catch (...)
      {
        gameSub.GetClient().LogException("EnableKeyboard()");
      }
    }
  }
  return bSuccess;
}

bool CPlayerManager::OpenMouse(CGameClientSubsystem& gameSub, ControllerPtr controller, 
                                  AddonInstance_Game& m_struct, PERIPHERALS::PeripheralVector& mice)
{
  CServiceBroker::GetPeripherals().GetPeripheralsWithFeature(mice, PERIPHERALS::FEATURE_MOUSE);
  if (mice.empty())
    return false;

  bool bSuccess = false;

  {
    CSingleLock lock(gameSub.GetAccess());

    if (gameSub.GetClient().Initialized())
    {
      try
      {
        bSuccess = m_struct.toAddon.EnableMouse(true, controller->ID().c_str());
      }
      catch (...)
      {
        gameSub.GetClient().LogException("EnableMouse()");
      }
    }
  }
  return bSuccess;
}

void CPlayerManager::SetJoystick(CControllerTree controllers, CGameClientSubsystem& gamesub, AddonInstance_Game& addonStruct)
{
  CGameClientInput inputClient(gamesub.GetClient(),addonStruct,gamesub.GetAccess());
  for (const auto& port : controllers.Ports())
  {
    if (port.PortType() == PORT_TYPE::CONTROLLER && !port.CompatibleControllers().empty())
    {
      ControllerPtr controller = port.ActiveController().Controller();
      inputClient.OpenJoystick(port.Address(), controller);
    }
  }
}

bool CPlayerManager::OpenJoystick(const std::string& portAddress, const ControllerPtr& controller, CControllerPortNode port, 
                                  CGameClientSubsystem& gameSub, AddonInstance_Game& m_struct)
{
  if (!port.IsControllerAccepted(portAddress, controller->ID()))
  {
    CLog::Log(LOGERROR, "Failed to open port: Invalid controller \"%s\" on port \"%s\"",
              controller->ID().c_str(), portAddress.c_str());
    return false;
  }
  CLog::Log(LOGDEBUG, "Controller \"%s\" on port \"%s\"",
              controller->ID().c_str(), portAddress.c_str());
  bool bSuccess = false;

  {
    CSingleLock lock(gameSub.GetAccess());

    if (gameSub.GetClient().Initialized())
    {
      try
      {
        bSuccess =
            m_struct.toAddon.ConnectController(true, portAddress.c_str(), controller->ID().c_str());
      }
      catch (...)
      {
        gameSub.GetClient().LogException("ConnectController()");
      }
    }
  }
  return bSuccess;
}

bool CPlayerManager::OnKeyPress(const CKey& key)
{
  OnKeyboardAction();
  return false;
}

void CPlayerManager::OnKeyRelease(const CKey& key)
{
  OnKeyboardAction();
}

bool CPlayerManager::OnPosition(int x, int y)
{
  OnMouseAction();
  return false;
}

bool CPlayerManager::OnButtonPress(MOUSE::BUTTON_ID button)
{
  OnMouseAction();
  return false;
}

void CPlayerManager::OnButtonRelease(MOUSE::BUTTON_ID button)
{
  OnMouseAction();
}

void CPlayerManager::OnJoystickEvent()
{
  //! @todo
  using namespace PERIPHERALS;

  PeripheralVector peripherals;
  m_peripheralManager.GetPeripheralsWithFeature(peripherals, FEATURE_JOYSTICK);
}

// int CPlayerManager::GetPeripheralPort(PERIPHERALS::PeripheralVector peripherals)
// {
  
// }

void CPlayerManager::OnKeyboardAction()
{
  if (!m_bHasKeyboard)
  {
    m_bHasKeyboard = true;

    //! @todo Notify of state update
    using namespace PERIPHERALS;

    PeripheralVector peripherals;
    m_peripheralManager.GetPeripheralsWithFeature(peripherals, FEATURE_KEYBOARD);
  }
}

void CPlayerManager::OnMouseAction()
{
  if (!m_bHasMouse)
  {
    m_bHasMouse = true;

    //! @todo Notify of state update
    using namespace PERIPHERALS;

    PeripheralVector peripherals;
    m_peripheralManager.GetPeripheralsWithFeature(peripherals, FEATURE_MOUSE);
  }
}
