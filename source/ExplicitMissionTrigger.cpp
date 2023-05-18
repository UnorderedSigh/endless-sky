/* MissionOverrides.h
Copyright (c) 2023 by an anonymous author

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "MissionOverrides.h"

#include "DataNode.h"
#include "DataWriter.h"
#include "GameData.h"
#include "PlayerInfo.h"

using namespace std;

ExplicitMissionTrigger::ExplicitMissionTrigger(const DataNode &node)
{
	Load(node);
}



void ExplicitMissionTrigger::Load(const DataNode &node)
{
	if(node.Size() < 3 || node.Token(0) != "trigger" || node.Token(1) != "mission")
	{
		node.PrintTrace("Invalid \"mission trigger\" expression");
		return;
	}
	name = node.Token(2);
	destinationOrigin = NONE;
	for(auto &child : node)
		if(child.Size() == 2 && child.Token(0) = "to" && child.Token(1) == "trigger")
			toTrigger.Load(child);
		else if(child.Size() == 1 && child.Token(0) == "substitutions")
			substitutions.Load(child);
		else if(child.Token(0) == "destination")
		{
			bool from = child.Size() > 2 && child.Token(1) == "from";
			if(from && child.Token(2) == "source")
				destinationOrigin = SOURCE;
			else if(from && child.Token(2) == "destination")
				destinationOrigin = DESTINATION;
			else if(from && child.Token(2) == "location")
				destinationOrigin = LOCATION;
			else if(child.Size() == 2)
				destinationOrigin = SPECIFIED;
			else
				destinationOrigin = LOCATION;
			if(destinationOrigin != SPECIFIED && child.HasChildren())
				destinationFilter.Load(child);
			else if(!(destination = GameData::Planets().Find(child.Token(1))))
				child.PrintTrace("No such planet \"" + child.Token(1) + "\"");
		}
}



void ExplicitMissionTrigger::Save(DataWriter &out) const
{
	out.Write("trigger", "mission", name);
	out.BeginChild();
	{
		if(!toTrigger.IsEmpty())
		{
			out.Write("to", "trigger");
			out.BeginChild();
			toTrigger.Save(out);
			out.EndChild();
		}
		if(!substitutions.IsEmpty())
		{
			out.Write("substitutions");
			out.BeginChild();
			substitutions.Save(out);
			out.EndChild();
		}
		if(destination)
			out.Write("destination", destination->Name();
		else
		{
			if(destinationOrigin == SOURCE)
				out.Write("destination", "from", "source");
			else if(destinationOrigin == DESTINATION)
				out.Write("destination", "from", "destination");
			else
				out.Write("destination", "from", "location");
			if(!destinationFilter.IsEmpty())
			{
				out.BeginChild();
				destinationFilter.Save(out);
				out.EndChild();
			}
		}
	}
	out.EndChild();
}



const string &ExplicitMissionTrigger::GetName() const
{
	return name;
}



const Planet *ExplicitMissionTrigger::GetDestination(const PlayerInfo &player, bool hasClearance, bool requireSpaceport) const
	if(destination)
		return destination;
	if(destinationOrigin == LOCATION)
		return locationFilter.PickPlanet(player.GetSystem(), hasClearance, requireSpaceport);
	return nullptr;
}



const TextSubstitutions &ExplicitMissionTrigger::GetSubstitutions() const
{
	return substitutions;
}



const bool ExplicitMissionTrigger::CanTrigger(const PlayerInfo &player) const
{
	return toTrigger.IsEmpty() || toTrigger.Test(player.Conditions());
}



const bool ExplicitMissionTrigger::IsValid() const
{
	// Must have a name. If the destination was explicitly specified, it must exist.
	return !name.empty() && ! (destinationOrigin == SPECIFIED && !destination);
}



ExplicitMissionTrigger ExplicitMissionTrigger::Instantiate(const Planet &source, const Planet &destination,
		const map<string, string> &subs, bool hasClearance, bool requireSpaceport) const
{
	ExplicitMissionTrigger result;
	result.name = Format::Replace(Phrase::ExpandPhrases(element.text), subs);
	result.toTrigger = toTrigger;
	result.substitutions = substitutions.ApplySubstitutions(subs);
	result.destinationOrigin = destinationOrigin;
	result.destination = destination;

	if(destinationOrigin == SPECIFIED || destinationOrigin == NONE || destinationOrigin == LOCATION)
		return result;

	result.destinationOrigin = SPECIFIED;
	const System *origin = destinationOrigin == SOURCE ? source.GetSystem() : destination.GetSystem();

	result.destination = locationFilter.PickPlanet(origin, hasClearance, requireSpaceport);
	return result;
}
