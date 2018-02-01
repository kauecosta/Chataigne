/*
  ==============================================================================

    SerialModule.cpp
    Created: 15 Mar 2017 10:17:22am
    Author:  Ben-Portable

  ==============================================================================
*/

#include "SerialModule.h"


SerialModule::SerialModule(const String &name) :
	StreamingModule(name),
	port(nullptr)
{
	portParam = new SerialDeviceParameter("Port", "Serial Port to connect",true);
	moduleParams.addParameter(portParam);
	baudRate = moduleParams.addIntParameter("Baud Rate", "The connection speed. Common values are 9600, 57600, 115200", 9600, 9600, 1000000);
	portParam->openBaudRate = baudRate->intValue();
	SerialManager::getInstance()->addSerialManagerListener(this);
}

SerialModule::~SerialModule()
{
	if (SerialManager::getInstanceWithoutCreating() != nullptr)
	{
		SerialManager::getInstance()->removeSerialManagerListener(this);
	}

	setCurrentPort(nullptr);
}

void SerialModule::setCurrentPort(SerialDevice * _port)
{
	if (port == _port) return;

	if (port != nullptr)
	{
		port->removeSerialDeviceListener(this);
	}

	port = _port;

	if (port != nullptr)
	{
		DBG(" > " << port->info->port);

		port->addSerialDeviceListener(this);
		port->open(baudRate->intValue());
		if (!port->isOpen())
		{
			NLOG(niceName, "Could not open port : " << port->info->port);
		}
		else
		{
			port->setMode(streamingType->getValueDataAsEnum<SerialDevice::PortMode>());
		}
		lastOpenedPortID = port->info->port;
	}

	serialModuleListeners.call(&SerialModuleListener::currentPortChanged);
}

void SerialModule::onControllableFeedbackUpdateInternal(ControllableContainer * cc, Controllable * c)
{
	StreamingModule::onControllableFeedbackUpdateInternal(cc, c);

	if (c == baudRate)
	{
		portParam->openBaudRate = baudRate->intValue();
		if (portParam->getDevice() != nullptr)
		{
			var val = portParam->value;
			portParam->setValue(var());
			portParam->setValue(val);
		}
		
	}
	if (c == portParam)
	{
		 setCurrentPort(portParam->getDevice());
	}if (c == streamingType)
	{
		if (port != nullptr) port->setMode((SerialDevice::PortMode)(int)streamingType->getValueData());
	}
};


void SerialModule::portOpened(SerialDevice *)
{
	serialModuleListeners.call(&SerialModuleListener::portOpened);
}

void SerialModule::portClosed(SerialDevice *)
{
	serialModuleListeners.call(&SerialModuleListener::portClosed);
}

void SerialModule::portRemoved(SerialDevice *)
{
	setCurrentPort(nullptr);
}

void SerialModule::serialDataReceived(const var & data)
{
	switch (port->mode)
	{

	case SerialDevice::LINES:
		processDataLine(data.toString());
		break;

	case SerialDevice::DATA255:
	case SerialDevice::RAW:
	case SerialDevice::COBS:
		Array<uint8> bytes((const uint8_t *)data.getBinaryData()->getData(), (int)data.getBinaryData()->getSize());
		processDataBytes(bytes);
	break;

	}
}

void SerialModule::portAdded(SerialDeviceInfo * info)
{
	//DBG("SerialModule, portAdded >" << info->hardwareID << "< > " << lastOpenedPortID);
	if (port == nullptr && lastOpenedPortID == info->port)
	{
		setCurrentPort(SerialManager::getInstance()->getPort(info));
	}
}

void SerialModule::portRemoved(SerialDeviceInfo *)
{
}