/*
 * Copyright (C) 2011-2012 DarkCore <http://www.darkpeninsula.eu/>
 * Copyright (C) 2011-2012 Project SkyFire <http://www.projectskyfire.org/>
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2012 MaNGOS <http://getmangos.com/>
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef DARKCORE_WAYPOINTMOVEMENTGENERATOR_H
#define DARKCORE_WAYPOINTMOVEMENTGENERATOR_H

/** @page PathMovementGenerator is used to generate movements
 * of waypoints and flight paths.  Each serves the purpose
 * of generate activities so that it generates updated
 * packets for the players.
 */

#include "MovementGenerator.h"
#include "WaypointManager.h"
#include "Path.h"

#include "Player.h"

#include <vector>
#include <set>

#define FLIGHT_TRAVEL_UPDATE  100
#define STOP_TIME_FOR_PLAYER  3 * MINUTE * IN_MILLISECONDS           // 3 Minutes
#define TIMEDIFF_NEXT_WP      250

template<class T, class P>
class PathMovementBase
{
    public:
        PathMovementBase() : i_currentNode(0), i_path(NULL) {}
        virtual ~PathMovementBase() {};

        // template pattern, not defined .. override required
        void LoadPath(T &);
        uint32 GetCurrentNode() const { return i_currentNode; }

    protected:
        P i_path;
        uint32 i_currentNode;
};

template<class T>
class WaypointMovementGenerator;

template<>
class WaypointMovementGenerator<Creature>
: public MovementGeneratorMedium< Creature, WaypointMovementGenerator<Creature> >,
public PathMovementBase<Creature, WaypointPath const*>
{
    public:
        WaypointMovementGenerator(uint32 _path_id = 0, bool _repeating = true) : i_nextMoveTime(0), path_id(_path_id), m_isArrivalDone(false), repeating(_repeating)  {}
        ~WaypointMovementGenerator() { i_path = NULL; }
        void Initialize(Creature &);
        void Finalize(Creature &);
        void Reset(Creature &);
        bool Update(Creature &, const uint32 &diff);

        void MovementInform(Creature &);

        MovementGeneratorType GetMovementGeneratorType() { return WAYPOINT_MOTION_TYPE; }

        // now path movement implmementation
        void LoadPath(Creature &c);

        bool GetResetPosition(Creature&, float& x, float& y, float& z);

    private:

        void Stop(int32 time) { i_nextMoveTime.Reset(time);}

        bool Stopped() { return !i_nextMoveTime.Passed();}

        bool CanMove(int32 diff)
        {
            i_nextMoveTime.Update(diff);
            return i_nextMoveTime.Passed();
        }

        void OnArrived(Creature&);
        bool StartMove(Creature&);

        void StartMoveNow(Creature& creature)
        {
            i_nextMoveTime.Reset(0);
            StartMove(creature);
        }

        TimeTrackerSmall i_nextMoveTime;
        bool m_isArrivalDone;
        uint32 path_id;
        bool repeating;
};

/** FlightPathMovementGenerator generates movement of the player for the paths
 * and hence generates ground and activities for the player.
 */
class FlightPathMovementGenerator
: public MovementGeneratorMedium< Player, FlightPathMovementGenerator >,
public PathMovementBase<Player, TaxiPathNodeList const*>
{
    public:
        explicit FlightPathMovementGenerator(TaxiPathNodeList const& pathnodes, uint32 startNode = 0)
        {
            i_path = &pathnodes;
            i_currentNode = startNode;
        }
        void Initialize(Player &);
        void Reset(Player &);
        void Finalize(Player &);
        bool Update(Player &, const uint32&);
        MovementGeneratorType GetMovementGeneratorType() { return FLIGHT_MOTION_TYPE; }

        TaxiPathNodeList const& GetPath() { return *i_path; }
        uint32 GetPathAtMapEnd() const;
        bool HasArrived() const { return (i_currentNode >= i_path->size()); }
        void SetCurrentNodeAfterTeleport();
        void SkipCurrentNode() { ++i_currentNode; }
        void DoEventIfAny(Player& player, TaxiPathNodeEntry const& node, bool departure);

        bool GetResetPosition(Player&, float& x, float& y, float& z);
};
#endif

