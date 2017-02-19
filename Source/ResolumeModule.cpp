/*
  ==============================================================================

    ResolumeOutput.cpp
    Created: 29 Oct 2016 7:21:01pm
    Author:  bkupe

  ==============================================================================
*/

#include "ResolumeModule.h"
#include "OSCCommand.h"
#include "CommandFactory.h"

ResolumeModule::ResolumeModule() :
	OSCModule("Resolume")
{	
/*
	"Composition"
		"Stop Composition"

	"Clip Transport"
		"Launch Clip"
		"Launch MultiClip"
		"Stop Clip"

	"Column Transport"
		"Launch Column"
		"Launch MultiColumn"
		"Stop Column"

	"Video"
		"Video Opacity"
		"Video Parameter"
		"Source Parameter"
		"Dashboard Parameter"

	"Audio"
		"Audio Volume"
		"Audio Parameter"
*/
	defManager.add(CommandDefinition::createDef(this, "Composition", "Stop Composition", &OSCCommand::create, CommandContext::ACTION)->addParam("address", "/composition/stop"));	
}
