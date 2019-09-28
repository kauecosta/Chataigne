/*
  ==============================================================================

	ModuleRouter.cpp
	Created: 29 Apr 2017 10:35:18am
	Author:  Ben

  ==============================================================================
*/

#include "ModuleRouter.h"
#include "../ModuleManager.h"
#include "CustomVariables/CVGroupManager.h"
#include "Module/modules/customvariables/CustomVariablesModule.h"

ModuleRouter::ModuleRouter() :
	BaseItem("Router"),
	sourceModule(nullptr),
	destModule(nullptr),
	sourceValues("Source Values")
{
	sourceValues.userCanAddItemsManually = false;
	selectAllValues = addTrigger("Select All", "Select all values for routing");
	deselectAllValues = addTrigger("Deselect All", "Deselect all values");
}

ModuleRouter::~ModuleRouter()
{
	setSourceModule(nullptr);
	setDestModule(nullptr);
}

void ModuleRouter::setSourceModule(Module * m)
{
	if (m == sourceModule) return;

	//make sure that destModule is in a null or stable state but not in a fucked state before reassigning all the routerValues
	if (destModule != nullptr && destModuleRef.wasObjectDeleted()) setDestModule(nullptr);

	if (sourceModule != nullptr)
	{
		sourceModule->valuesCC.removeAsyncContainerListener(this);
		sourceModule->removeInspectableListener(this);
		sourceModule->removeControllableContainerListener(this);
		sourceValues.clear();
	}
	
	sourceModule = m;

	if (sourceModule != nullptr)
	{
		sourceModuleRef = sourceModule;

		sourceModule->valuesCC.addAsyncContainerListener(this);
		sourceModule->addInspectableListener(this);
		sourceModule->addControllableContainerListener(this);

		Array<WeakReference<Controllable>> values = sourceModule->valuesCC.getAllControllables(true);
		int index = 0;
		for (auto &c : values)
		{
			ModuleRouterValue * mrv = new ModuleRouterValue(c, index++);
			sourceValues.addItem(mrv, var(), false);
			mrv->forceDisabled = !enabled->boolValue();
			mrv->setSourceAndOutModule(sourceModule, destModule);
		}
	}

	routerListeners.call(&RouterListener::sourceModuleChanged, this);
}

void ModuleRouter::setDestModule(Module * m)
{
	if (m == destModule) return;

	if (!destModuleRef.wasObjectDeleted() && destModule != nullptr)
	{
		destModule->removeInspectableListener(this);
	}

	destModule = m;

	if (destModule != nullptr)
	{
		destModuleRef = destModule;
		destModule->addInspectableListener(this);
	}

	if (!sourceModuleRef.wasObjectDeleted())
	{
		for (auto &mrv : sourceValues.items)
		{
			mrv->setSourceAndOutModule(sourceModule, destModule);
			mrv->forceDisabled = !enabled->boolValue();
		}
	}

	routerListeners.call(&RouterListener::destModuleChanged, this);
}

void ModuleRouter::reloadSourceValues(bool keepData)
{
	var prevData = sourceValues.getJSONData();

	sourceValues.clear();

	Array<WeakReference<Controllable>> values = sourceModule->valuesCC.getAllControllables(true);
	int index = 0;
	for (auto& c : values)
	{
		ModuleRouterValue* mrv = new ModuleRouterValue(c, index++);
		sourceValues.addItem(mrv, var(), false);
		mrv->forceDisabled = !enabled->boolValue();
		mrv->setSourceAndOutModule(sourceModule, destModule);
	}

	if (keepData) sourceValues.loadItemsData(prevData);

}


var ModuleRouter::getJSONData()
{
	var data = BaseItem::getJSONData();
	if (sourceModule != nullptr)
	{
		data.getDynamicObject()->setProperty("sourceModule", sourceModule->shortName);
		data.getDynamicObject()->setProperty("sourceValues", sourceValues.getJSONData());
	}
	if(destModule != nullptr) data.getDynamicObject()->setProperty("destModule", destModule->shortName);

	return data;
}

void ModuleRouter::loadJSONDataInternal(var data)
{
	BaseItem::loadJSONDataInternal(data);

	String moduleName = data.getProperty("sourceModule", "");
	if (moduleName == CVGroupManager::getInstance()->module->shortName) setSourceModule(CVGroupManager::getInstance()->module.get());
	else setSourceModule(ModuleManager::getInstance()->getItemWithName(moduleName));
	
	setDestModule(ModuleManager::getInstance()->getItemWithName(data.getProperty("destModule", "")));
	
	if (data.getDynamicObject()->hasProperty("sourceValues")) sourceValues.loadItemsData(data.getProperty("sourceValues", var()));
}

void ModuleRouter::newMessage(const ContainerAsyncEvent & e)
{
	/*
	if (e.type == ContainerAsyncEvent::ControllableAdded)
	{
		if (sourceModule->valuesCC.containsControllable(e.targetControllable))
		{
			ModuleRouterValue * mrv = new ModuleRouterValue(e.targetControllable, sourceValues.items.size());
			sourceValues.addItem(mrv, var(), false);
			mrv->setSourceAndOutModule(sourceModule, destModule);
		}
	} else if (e.type == ContainerAsyncEvent::ControllableRemoved)
	{
		if (sourceModule->valuesCC.containsControllable(e.targetControllable))
		{
			ModuleRouterValue * v = getRouterValueForControllable(e.targetControllable);
			if (v == nullptr) return;
			sourceValues.removeItem(v, false);
		}
	}*/

	if (e.type == e.ChildStructureChanged)
	{
		if (!Engine::mainEngine->isLoadingFile && !isCurrentlyLoadingData && !Engine::mainEngine->isClearing)
		{
			reloadSourceValues();
		}
	}
}

ModuleRouterValue * ModuleRouter::getRouterValueForControllable(Controllable * c)
{
	for (auto &v : sourceValues.items)
	{
		if (v->sourceValue == c) return v;
	}

	return nullptr;
}

void ModuleRouter::onContainerParameterChangedInternal(Parameter * p)
{
	if (p == enabled)
	{
		for (auto &mrv : sourceValues.items) mrv->forceDisabled = !enabled->boolValue();
	}
}

void ModuleRouter::onContainerTriggerTriggered(Trigger * t)
{
	if (t == selectAllValues || t == deselectAllValues)
	{
		for (auto &sv : sourceValues.items)
		{
			sv->enabled->setValue(t == selectAllValues);
		}
	}
}

void ModuleRouter::inspectableDestroyed(Inspectable * i)
{
	if (i == sourceModule) setSourceModule(nullptr);
	else if (i == destModule) setDestModule(nullptr);
}

