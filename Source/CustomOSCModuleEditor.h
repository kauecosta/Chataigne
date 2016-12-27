/*
  ==============================================================================

    CustomOSCModuleEditor.h
    Created: 27 Dec 2016 1:54:47pm
    Author:  Ben

  ==============================================================================
*/

#ifndef CUSTOMOSCMODULEEDITOR_H_INCLUDED
#define CUSTOMOSCMODULEEDITOR_H_INCLUDED

#include "OSCModuleBaseEditor.h"
#include "CustomOSCModule.h"

class CustomOSCModuleEditor :
	public OSCModuleBaseEditor
{
public:
	CustomOSCModuleEditor(CustomOSCModule * cModule, bool isRoot);
	~CustomOSCModuleEditor();

	CustomOSCModule * cModule;

	ScopedPointer<BoolToggleUI> autoAddUI;
	ScopedPointer<InspectableEditor> valuesEditor;

	int resizedInternal(Rectangle<int> r) override;
};




#endif  // CUSTOMOSCMODULEEDITOR_H_INCLUDED
