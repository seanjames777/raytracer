/**
 * @file shape.cpp
 *
 * @author Sean James
 */

#include "shape.h"

CollisionResult::CollisionResult()
    : distance(0.0f),
      shape(NULL)
{
}

Vec3 Shape::getPosition() {
    return getBBox().center();
}

bool Shape::containedIn(AABB in) {
	return in.intersectsBbox(getBBox());
}
