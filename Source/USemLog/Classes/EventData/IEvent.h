// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "OwlNode.h"

/**
* Abstract class ensuring every event can be represented as an Owl Node;
*/
class IEvent
{
public:
	virtual FOwlNode ToOwlNode() const = 0;
};