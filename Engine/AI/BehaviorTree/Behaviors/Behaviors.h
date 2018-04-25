/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Include file for all standard behaviors.
*****************************************************************************/

#ifndef __USG_AI_BEHAVIORS__
#define __USG_AI_BEHAVIORS__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Behaviors/IsTargetAliveBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/IsTargetWithinDistanceBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/HasTargetBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/HasNoTargetBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/TargetMaxDistanceBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/TargetMinDistanceBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/MoveTowardsPositionBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/SetDestinationToTargetBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/SetDestinationToWaypointBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/ClearDestinationBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/ApproachTargetBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/RandomPositionBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/RemoveTargetBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/IsHealthAboveBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/IsHealthBelowBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/MoveAwayFromTargetBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/CircleAroundTargetBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/SetAnimationCondition.h"
#include "Engine/AI/BehaviorTree/Behaviors/FuzzyWaitBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/FaceTargetBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/FuzzyApproachTargetBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/ObjectAvoidanceBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/SetReverseBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/CanSeeTargetBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/CanNotSeeTargetBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/IsTargetSameTeamBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/SetDestinationBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/DestinationMinDistanceBehavior.h"
#include "Engine/AI/BehaviorTree/Behaviors/VariableBehaviors.h"
#endif	//	__USG_AI_BEHAVIORS__
