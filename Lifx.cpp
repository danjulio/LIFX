/************************************************************************/
/* A library to control LIFX lights over the LAN                        */
/* Based on sample code from this fine person at                        */
/* https://community.lifx.com/t/sending-lan-packet-using-arduino/1460/3 */
/*                                                                      */
/* This library is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or    */
/* (at your option) any later version.                                  */
/*                                                                      */
/* This library is distributed in the hope that it will be useful, but  */
/* WITHOUT ANY WARRANTY; without even the implied warranty of           */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     */
/* General Public License for more details.                             */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */
/* along with this library. If not, see <http://www.gnu.org/licenses/>. */
/*                                                                      */
/* Written by Peter Humphrey July 2021.                                 */
/* Support for color devices and other features on ESP32 by Dan Julio   */
/* Jan 2022.                                                            */
/************************************************************************/


#include "Lifx.h"
#include "LifxProducts.h"


// Undefine for debugging
//#define DEBUG



Lifx::Lifx()
{
   // Initialise header
  memset(&_header, 0, sizeof(_header));

  //  Initialise payload
  memset(&_payload, 0, sizeof(_payload));
  
  // Setup the static bits of header
  _header.tagged = 1;
  _header.addressable = 1;
  _header.protocol = 1024;
  _header.ack_required = 0;
  _header.res_required = 1;
  _header.sequence = 100;

  return;
}

void Lifx::begin() {
  //  UDP
  _udp.begin(LIFX_PORT);          // Listen for incoming UDP packets
  
  //  random seed for source number
  #if defined(ESP8266)
  randomSeed(analogRead(0));
  #elif defined(ESP32)
  randomSeed(analogRead(39));
  #else
  randomSeed(analogRead(A0));
  #endif
}

void Lifx::loop() {
  //  INCOMING UDP
  int packetLen = _udp.parsePacket();
  byte packetBuffer[LIFX_INCOMING_PACKET_BUFFER_LEN];
  if (packetLen && packetLen < LIFX_INCOMING_PACKET_BUFFER_LEN) 
  {
    _udp.read(packetBuffer, sizeof(packetBuffer));
    ReceivedMessage(packetBuffer, packetLen);
  }

  if (_discoveryUnderway) DoDiscovery();
  
  //  kick a discovery off every LIFX_REDISCOVERY_INTERVAL millisecs
  if ((millis() - _discoveryTimer) > LIFX_REDISCOVERY_INTERVAL)
  {
    #ifdef DEBUG
    Serial.println("Rediscovery..");
    #endif
    StartDiscovery();
  }
}

void Lifx::StartDiscovery() {
  //  this may leave devices that no longer exist in the array, but that doesn't seem too bad a thing
  #ifdef DEBUG
  Serial.println("Start discovery..");
  #endif

  _discoveryTimer = millis();
  _discoveryNextMsec = 3000;
  _discoveryBroadcastCount = 0;
  _discoveryDeviceIndex = 0;
  _discoveryUnderway = true;
}

void Lifx::DoDiscovery() {
  //  i had trouble making this reliable. slowing it down helped. i had it arrivale of a response from a light kicking off the next
  //  get message and think that there was too much UDP traffic to keep up with
  unsigned long msecs = millis() - _discoveryTimer;
  
  //  at milliseconds 0, 1000 and 2000 send out discovery broadcasts. do this three times because it seems like a dodgy process
  //  devices are recognised in ReceivedMessage
  if ((msecs > (1000 * _discoveryBroadcastCount)) && (_discoveryBroadcastCount < 3)) {
    _discoveryBroadcastCount++;
    //  send the get service broadcast
    SendMessage(LIFX_DEVICE_GETSERVICE, NULL, IPAddress(255,255,255,255), 0);
  }

  if (msecs > _discoveryNextMsec) {
    //  check if we are complete
    if (_discoveryDeviceIndex >= _devices.size()) {
      _discoveryUnderway = false;
      if (_discoveryCompleteFunction != NULL) _discoveryCompleteFunction (*this);
      return;
    }

    //  wait 250 msecs for devices to reply
    _discoveryNextMsec += 250;

    //  then get next bit of state information from device 
    switch (_devices[_discoveryDeviceIndex]->LastMessageType)
    {
      case LIFX_DEVICE_STATESERVICE:
        SendMessage(LIFX_DEVICE_GETLABEL, _devices[_discoveryDeviceIndex]->MacAddress(), IPAddress(_devices[_discoveryDeviceIndex]->IpAddress()), 0);
        break;
      
      case LIFX_DEVICE_STATELABEL:
        SendMessage(LIFX_DEVICE_GETVERSION, _devices[_discoveryDeviceIndex]->MacAddress(), IPAddress(_devices[_discoveryDeviceIndex]->IpAddress()), 0);
        break;
      
      case LIFX_DEVICE_STATEVERSION:
        SendMessage(LIFX_DEVICE_GETLOCATION, _devices[_discoveryDeviceIndex]->MacAddress(), IPAddress(_devices[_discoveryDeviceIndex]->IpAddress()), 0);
        break;
      
      case LIFX_DEVICE_STATELOCATION:
        SendMessage(LIFX_DEVICE_GETGROUP, _devices[_discoveryDeviceIndex]->MacAddress(), IPAddress(_devices[_discoveryDeviceIndex]->IpAddress()), 0);
        break;
  
      case LIFX_DEVICE_STATEGROUP:
        SendMessage(LIFX_LIGHT_GET, _devices[_discoveryDeviceIndex]->MacAddress(), IPAddress(_devices[_discoveryDeviceIndex]->IpAddress()), 0);
        break;
  
      case LIFX_LIGHT_STATE:
        //  move on to next device
        _discoveryDeviceIndex++;
        break;
    }
  }
}

void Lifx::ReceivedMessage(byte packet[], int packetLen) {
  //  we get 'get service' messages from the phone app and should ignore these
  if (((lifx_header *)packet)->type == LIFX_DEVICE_GETSERVICE)
  {
    #ifdef DEBUG
    Serial.println("Get service msg received");
    #endif
  }
  else
  {
    Device *dev = DeviceAddToArray(((lifx_header *)packet)->target, (uint32_t)_udp.remoteIP());
    
    #ifdef DEBUG
    Serial.printf("Recd %s %d, msg type %d, source %d, MAC addr %s\n", _udp.remoteIP().toString().c_str(), _udp.remotePort(), ((lifx_header *)packet)->type, ((lifx_header *)packet)->source, dev->MacAddressString());
    #endif
  
    DealWithReceivedMessage(packet, packetLen, dev);
  }  
  return;
}

void Lifx::DealWithReceivedMessage(byte packet[], int packetLen, Device *device) {
  device->LastMessageType = ((lifx_header *)packet)->type;
  
  switch (device->LastMessageType)
  {
    case LIFX_DEVICE_STATESERVICE:
      //  nothing more to do
      break;

    case LIFX_DEVICE_STATEPOWER:
      device->Power=((lifx_payload_device_power *)(packet + sizeof(lifx_header)))->level;
      break;

    case LIFX_DEVICE_STATELABEL:
      memcpy(device->Label, ((lifx_payload_device_label *)(packet + sizeof(lifx_header)))->label, 32);
      break;
      
    case LIFX_DEVICE_STATEVERSION:
      device->Product=((lifx_payload_device_version *)(packet + sizeof(lifx_header)))->product;
      break;

    case LIFX_DEVICE_STATELOCATION:
      memcpy(device->Location, ((lifx_payload_device_location *)(packet + sizeof(lifx_header)))->label, 32);
      break;
      
    case LIFX_DEVICE_STATEGROUP:
      memcpy(device->Group, ((lifx_payload_device_group *)(packet + sizeof(lifx_header)))->label, 32);
      break;
      
    case LIFX_LIGHT_STATE:
      device->Hue = ((lifx_payload_light_state *)(packet + sizeof(lifx_header)))->hue;
      device->Saturation = ((lifx_payload_light_state *)(packet + sizeof(lifx_header)))->saturation;
      device->Brightness = ((lifx_payload_light_state *)(packet + sizeof(lifx_header)))->brightness;
      device->Kelvin = ((lifx_payload_light_state *)(packet + sizeof(lifx_header)))->kelvin;
      device->Power = ((lifx_payload_light_state *)(packet + sizeof(lifx_header)))->power;
      _lightUpdateUnderway = 0;
      break;
  }
}

void Lifx::SendMessage(uint16_t messageType, byte *macAddress, IPAddress ipAddress, int payloadLen) {  
  _header.size = sizeof(lifx_header) + payloadLen;
  _header.source = random(4294967295);
  _header.type = messageType;
  if (macAddress == NULL)
  {
    memset(_header.target, 0, sizeof(uint8_t) * LIFX_MAC_LEN);
    _header.tagged = 1;
  }
  else
  {
    memcpy(_header.target, macAddress, sizeof(uint8_t) * LIFX_MAC_LEN);
    _header.tagged = 0;
  }
    
  // Send the packet
  _udp.beginPacket(ipAddress, LIFX_PORT);
  _udp.write((const uint8_t *) &_header, sizeof(lifx_header));
  if (payloadLen)
    _udp.write((const uint8_t *) &_payload, payloadLen);
  _udp.endPacket();

  #ifdef DEBUG
  if (macAddress) 
    Serial.printf("Send %s, Message type: %i, Source: %d, MAC Address: %02x %02x %02x %02x %02x %02x\n", ipAddress.toString().c_str(), messageType, _header.source, macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);   
  else
    Serial.printf("Send %s, Message type: %i, Source: %d\n", ipAddress.toString().c_str(), messageType, _header.source);   
  #endif
}

Device* Lifx::DeviceAddToArray(byte macAddress[6], IPAddress ipAddress) {
  //  check if we already have this one
  std::vector <Device*> :: iterator it;
  for(it = _devices.begin(); it != _devices.end(); ++it)
  {
    if (memcmp(macAddress, (*it)->MacAddress(), 6) == 0)
        return *it;
  } 
  //  if not add it to the vector
  Device* dev = new Device(macAddress, (uint32_t)ipAddress);
  _devices.push_back(dev);
  return dev;
}

uint16_t Lifx::DeviceCount() {
  return _devices.size();
}

Device* Lifx::GetIndexedDevice(int n) {
	return _devices[n];  // Make darn sure n is valid...
}

void Lifx::SetDevicePower(Device *dev, uint16_t power) {
  _payload.power.level = power;
  dev->Power = power;
  SendMessage(LIFX_DEVICE_SETPOWER, dev->MacAddress(), IPAddress(dev->IpAddress()), sizeof(lifx_payload_device_power));
}

void Lifx::SetDeviceBrightness(Device *dev, uint16_t brightness, uint32_t duration) {
  _payload.setColor.hue  = dev->Hue;
  _payload.setColor.saturation = dev->Saturation;
  _payload.setColor.brightness = brightness;
  dev->Brightness = brightness;
  _payload.setColor.kelvin = dev->Kelvin;
  _payload.setColor.duration = duration;
  SendMessage(LIFX_LIGHT_SETCOLOR, dev->MacAddress(), IPAddress(dev->IpAddress()), sizeof(lifx_payload_light_setcolor));
}

void Lifx::SetDeviceColor(Device *dev, uint16_t hue, uint16_t saturation, uint16_t brightness, uint16_t kelvin, uint32_t duration) {
  _payload.setColor.hue  = hue;
  dev->Hue = hue;
  _payload.setColor.saturation = saturation;
  dev->Saturation = saturation;
  _payload.setColor.brightness = brightness;
  dev->Brightness = brightness;
  _payload.setColor.kelvin = kelvin;
  dev->Kelvin = kelvin;
  _payload.setColor.duration = duration;
  SendMessage(LIFX_LIGHT_SETCOLOR, dev->MacAddress(), IPAddress(dev->IpAddress()), sizeof(lifx_payload_light_setcolor));
}

void Lifx::SetBrightnessByLabel(char *label, uint16_t brightness, uint32_t duration) {
  for(Device *dev: _devices)
  {
    if (strcmp(dev->Label,label) == 0)
    {
      SetDeviceBrightness(dev, brightness, duration);
    }
  }
}

void Lifx::SetBrightnessByGroup(char *group, uint16_t brightness, uint32_t duration) {
  for(Device *dev: _devices)
  {
    if (strcmp(dev->Group, group) == 0)
    {
      SetDeviceBrightness(dev, brightness, duration);
    }
  }
}

void Lifx::SetColorByGroup(char *group, uint16_t hue, uint16_t saturation, uint16_t brightness, uint16_t kelvin, uint32_t duration) {
  for(Device *dev: _devices)
  {
    if (strcmp(dev->Group, group) == 0)
    {
      SetDeviceColor(dev, hue, saturation, brightness, kelvin, duration);
    }
  }
}

void Lifx::SetColorByLabel(char *label, uint16_t hue, uint16_t saturation, uint16_t brightness, uint16_t kelvin, uint32_t duration) {
  for(Device *dev: _devices)
  {
    if (strcmp(dev->Label,label) == 0)
    {
      SetDeviceColor(dev, hue, saturation, brightness, kelvin, duration);
    }
  }
}

void Lifx::SetPowerByGroup(char *group, uint16_t power) {
  _payload.power.level = power;
  for(Device *dev: _devices)
  {
    if (strcmp(dev->Group,group) == 0)
      SetDevicePower(dev, power);
  }
}

void Lifx::SetPowerByLabel(char *label, uint16_t power) {
  _payload.power.level = power;
  for(Device *dev: _devices)
  {
    if (strcmp(dev->Label,label) == 0)
      //SendMessage(LIFX_DEVICE_SETPOWER, dev->MacAddress(), IPAddress(dev->IpAddress()), sizeof(lifx_payload_device_power));
      SetDevicePower(dev, power);
  }
}

void Lifx::StartDeviceLightUpdate(Device *dev) {
  SendMessage(LIFX_LIGHT_GET, dev->MacAddress(), IPAddress(dev->IpAddress()), 0);
  _lightUpdateUnderway = 1;
}

bool Lifx::DeviceLightUpdateDone() {
  return _lightUpdateUnderway;
}

uint16_t Lifx::StatePowerByGroup(char *group) {
  //  returns the power of the first device found in the group
  for(Device *dev: _devices)
  {
    if (strcmp(dev->Group,group) == 0)
      return dev->Power;
  }
  return 0;
}

uint16_t Lifx::StatePowerByLabel(char *label) {
  //  returns the power of the device with matching label
  for(Device *dev: _devices)
  {
    if (strcmp(dev->Label,label) == 0)
      return dev->Power;
  }
  return 0;
}

uint16_t Lifx::StateBrightnessByGroup(char *group) {
  //  returns the brightness of the first device found in the group
  for(Device *dev: _devices)
  {
    if (strcmp(dev->Group,group) == 0)
      return dev->Brightness;
  }
  return 0;
}

uint16_t Lifx::StateBrightnessByLabel(char *label) {
  //  returns the brightness of the first device found in the group
  for(Device *dev: _devices)
  {
    if (strcmp(dev->Label,label) == 0)
      return dev->Brightness;
  }
  return 0;
}

void Lifx::DiscoveryCompleteCallback(CallbackFunction f) {
  _discoveryCompleteFunction = f;
}

void Lifx::PrintDevices() {
  int i;
  
  Serial.println("IP Address,MAC Address,Location,Group,Label,Power,Hue,Saturation,Brightness,Kelvin");
  for(Device *dev: _devices)
  {
    Serial.printf("%s,%s,%s,%s,%s,%i,%i,%i,%i,%i",
      IPAddress(dev->IpAddress()).toString().c_str(),
      dev->MacAddressString(),
      dev->Location,
      dev->Group,
      dev->Label,
      dev->Power,
      dev->Hue,
      dev->Saturation,
      dev->Brightness,
      dev->Kelvin);

    i = lifx_find_pid_index(dev->Product);
    if (i >= 0) {
      Serial.printf("->%d,%s\n", dev->Product, lifx_types[i].name);
    } else {
      Serial.printf("->%d,unknown\n", dev->Product);
    }
  }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Device::Device(byte macAddress[], uint32_t ipAddress)
{
  memcpy(_macAddress, macAddress, LIFX_MAC_LEN);
  _ipAddress = ipAddress;
  Label[0] = 0;
  Group[0] = 0;
  Location[0] = 0;
  return;
}

byte *Device::MacAddress()
{
  return _macAddress;
}

uint32_t Device::IpAddress()
{
  return _ipAddress;
}

char *Device::MacAddressString()
{
  sprintf(_macString, "%02x:%02x:%02x:%02x:%02x:%02x", _macAddress[0], _macAddress[1], _macAddress[2], _macAddress[3], _macAddress[4], _macAddress[5]);
  return _macString;
}
