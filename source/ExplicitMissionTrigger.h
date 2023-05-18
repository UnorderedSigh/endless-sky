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

#ifndef EXPLICIT_MISSION_TRIGGER_H_
#define EXPLICIT_MISSION_TRIGGER_H_

#include "LocationFilter.h"
#include "Planet.h"
#include "TextReplacements.h"

#include <map>
#include <string>

class DataNode;
class DataWriter;
class PlayerInfo;

class ExplicitMissionTrigger {
public:
	ExplicitMissionTrigger(const DataNode &node);
	void Load(const DataNode &node);
	void Save(DataWriter &out) const;
	const std::string &GetName() const;
	const Planet *GetDestination(const PlayerInfo &info) const;
	const TextSubstitutions &GetSubstitutions() const;
	bool CanTrigger(const PlayerInfo &player) const;
	bool IsValid() const;

	// Expand substitutions. Find destination (if requested) unless its search origin is the player's location.
	ExplicitMissionTrigger Instantiate(const Planet &source, const Planet &destination,
		const std::map<std::string, std::string> &subs) const;


private:
	std::string name;
	ConditionSet toTrigger;
	TextReplacements substitutions;
	enum SearchOrigin { NONE, SPECIFIED, LOCATION, SOURCE, DESTINATION };
	SearchOrigin destinationOrigin = NONE;
	Planet *destination = nullptr;
	LocationFilter destinationFilter;
};

#endif
