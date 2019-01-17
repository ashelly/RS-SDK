#include "rspoe.h"
#include "controllers/pd69104.h"
#include "../utils/tinyxml2.h"

#include <map>
#include <string>
#include <stdint.h>

std::string s_lastError;
AbstractPoeController *sp_controller = nullptr;
std::map<int, uint8_t> s_portMap;

bool initPoe(const char* initFile)
{
    using namespace tinyxml2;
    s_portMap.clear();
    if (sp_controller) delete sp_controller;
    sp_controller = nullptr;

    XMLDocument doc;
    if (doc.LoadFile(initFile) != XML_SUCCESS)
    {
        s_lastError = "XML Error: Unable to load file";
        return false;
    }

    XMLElement *comp = doc.FirstChildElement("computer");
    if (!comp)
    {
        s_lastError = "XML Error: Unable to find computer node";
        return false;
    }

    XMLElement *poe = comp->FirstChildElement("poe_controller");
    if (!poe)
    {
        s_lastError = "XML Error: Unable to find poe_controller node";
        return false;
    }

    std::string id(poe->Attribute("id"));
    try
    {
        if (id == "pd69104")
            sp_controller = new Pd69104(0xF040, 0x40);
        else
        {
            s_lastError = "XML Error: Invalid id found for poe_controller";
            return false;
        }
    }
    catch (std::exception &ex)
    {
        s_lastError = "POE Controller Error: " + std::string(ex.what());
        return false;
    }

    XMLElement *port = poe->FirstChildElement("port");
    for (; port; port = port->NextSiblingElement("port"))
    {
        int id, bit;
        if (port->QueryAttribute("id", &id) != XML_SUCCESS)
            continue;

        if (port->QueryAttribute("bit", &bit) != XML_SUCCESS)
            continue;

        s_portMap[id] = bit;
    }

    if (s_portMap.size() <= 0)
    {
        sp_controller = nullptr;
        s_lastError = "XML Error: No ports found";
        return false;
    }

    return true;
}

PoeState getPortState(int port)
{
    if (sp_controller == nullptr)
    {
        s_lastError = "POE Controller Error: Not initialized. Please run 'initPoe' first";
        return StateError;
    }

    if (s_portMap.find(port) == s_portMap.end())
    {
        s_lastError = "Argument Error: Invalid port " + std::to_string(port);
        return StateError;
    }

    try 
    { 
        return sp_controller->getPortState(s_portMap[port]); 
    }
    catch (PoeControllerError &ex) 
    {
        s_lastError = "PoE Controller Error: " + std::string(ex.what());
    }

    return StateError;
}

int setPortState(int port, PoeState state)
{
    if (sp_controller == nullptr)
    {
        s_lastError = "POE Controller Error: Not initialized. Please run 'initPoe' first";
        return -1;
    }

    if (s_portMap.find(port) == s_portMap.end())
    {
        s_lastError = "Argument Error: Invalid port " + std::to_string(port);
        return -1;
    }

    try
    {
        sp_controller->setPortState(s_portMap[port], state);
    }
    catch (PoeControllerError &ex)
    {
        s_lastError = "PoE Controller Error: " + std::string(ex.what());
        return -1;
    }

    return 0;
}

float getPortVoltage(int port)
{
    float volts = -1.0f;
    if (sp_controller == nullptr)
    {
        s_lastError = "POE Controller Error: Not initialized. Please run 'initPoe' first";
        return volts;
    }

    if (s_portMap.find(port) == s_portMap.end())
    {
        s_lastError = "Argument Error: Invalid port " + std::to_string(port);
        return volts;
    }
    
    try
    {
        volts = sp_controller->getPortVoltage(s_portMap[port]);
    }
    catch (PoeControllerError& ex)
    {
        s_lastError = "PoE Controller Error: " + std::string(ex.what());
        return volts;
    }

    return volts;
}

float getPortCurrent(int port)
{
    float cur = -1.0f;
    if (sp_controller == nullptr)
    {
        s_lastError = "POE Controller Error: Not initialized. Please run 'initPoe' first";
        return cur;
    }

    if (s_portMap.find(port) == s_portMap.end())
    {
        s_lastError = "Argument Error: Invalid port " + std::to_string(port);
        return cur;
    }

    try
    {
        cur = sp_controller->getPortCurrent(s_portMap[port]);
    }
    catch (PoeControllerError& ex)
    {
        s_lastError = "PoE Controller Error: " + std::string(ex.what());
        return cur;
    }

    return cur;
}

int getPortPower(int port)
{
    int power = -1;
    if (sp_controller == nullptr)
    {
        s_lastError = "POE Controller Error: Not initialized. Please run 'initPoe' first";
        return power;
    }

    if (s_portMap.find(port) == s_portMap.end())
    {
        s_lastError = "Argument Error: Invalid port " + std::to_string(port);
        return power;
    }

    try
    {
        power = sp_controller->getPortPower(s_portMap[port]);
    }
    catch (PoeControllerError& ex)
    {
        s_lastError = "PoE Controller Error: " + std::string(ex.what());
        return power;
    }

    return power;
}

const char* getLastPoeError()
{
    return s_lastError.c_str();
}