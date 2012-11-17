/* $Id$ */

/*
 * This file is part of FreeRCT.
 * FreeRCT is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * FreeRCT is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with FreeRCT. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file map.h Definition of voxels in the world. */

#ifndef MAP_H
#define MAP_H

#include "tile.h"
#include "path.h"
#include "geometry.h"
#include "sprite_store.h"
#include "people.h"
#include "bitmath.h"

#include <map>

class Viewport;

static const int WORLD_X_SIZE = 128; ///< Maximal length of the X side (North-West side) of the world.
static const int WORLD_Y_SIZE = 128; ///< Maximal length of the Y side (North-East side) of the world.
static const int WORLD_Z_SIZE =  64; ///< Maximal height of the world.


/**
 * Types of voxels.
 * @ingroup map_group
 */
enum VoxelType {
	VT_EMPTY,     ///< Empty voxel.
	VT_SURFACE,   ///< %Voxel contains path and/or earthy surface.
	VT_REFERENCE, ///< %Voxel contains part of the referenced voxel.
};

/**
 * Description of a referenced voxel.
 * For structures larger than a single voxel, only the base voxel contains a description.
 * The other voxels reference to the base voxel for their sprites.
 * @ingroup map_group
 */
struct ReferenceVoxelData {
	uint16 xpos; ///< Base voxel X coordinate.
	uint16 ypos; ///< Base voxel Y coordinate.
	uint8  zpos; ///< Base voxel Z coordinate.
};

/** Flags for the ride voxel data. */
enum RideVoxelFlags {
	RVF_NONE = 0, ///< No flags set.
	RVF_ENTRANCE_NE = 0x01, ///< Voxel has a ride entrance at the north-east side.
	RVF_ENTRANCE_SE = 0x02, ///< Voxel has a ride entrance at the south-east side.
	RVF_ENTRANCE_SW = 0x04, ///< Voxel has a ride entrance at the south-west side.
	RVF_ENTRANCE_NW = 0x08, ///< Voxel has a ride entrance at the north-west side.
};

/**
 * One voxel cell in the world.
 * @ingroup map_group
 */
struct Voxel {
public:
	/**
	 * Word 0 of a voxel.
	 * - bit  0.. 1 (2): Type of the voxel. @see VoxelType
	 * - bit  2.. 9 (8): Foundation slopes (bits 0/1 for NE, 2/3 for ES, 4/5 for SW, and 6/7 for WN edge).
	 * - bit 10..13 (4): Type of foundation. @see FoundationType
	 * - bit 14..18 (5): Imploded ground slope. @see #ExpandTileSlope
	 * - bit 19..22 (4): Ground type. @see GroundType
	 */
	uint32 w0;

	/**
	 * Word 1 of a voxel.
	 * - bit 0.. 5 (6): Imploded path slope or ride flags. @see PathSprites RideVoxelFlags
	 * - bit 6..15 (10): Path type and ride numbers. @see PathRideType
	 */
	uint32 w1;

	/**
	 * Get the type of the voxel.
	 * @return Voxel type.
	 */
	inline VoxelType GetType() const {
		return (VoxelType)GB(this->w0, 0, 2);
	}

	/* Foundation data access. */

	/**
	 * Get the foundation slope of a surface voxel.
	 * @return The foundation slope.
	 */
	inline uint8 GetFoundationSlope() const {
		assert(this->GetType() == VT_SURFACE);
		return GB(this->w0, 2, 8);
	}

	/**
	 * Get the foundation type of a surface voxel.
	 * @return The foundation type.
	 */
	inline FoundationType GetFoundationType() const {
		assert(this->GetType() == VT_SURFACE);
		uint val = GB(this->w0, 10, 4);
		return (FoundationType)val;
	}

	/**
	 * Set the foundation slope of a surface voxel.
	 * @param fnd_slope The new foundation slope.
	 */
	inline void SetFoundationSlope(uint8 fnd_slope) {
		SB(this->w0, 0, 2, VT_SURFACE);
		SB(this->w0, 2, 8, fnd_slope);
	}

	/**
	 * Set the foundation type of a surface voxel.
	 * @param fnd_type The foundation type.
	 */
	inline void SetFoundationType(FoundationType fnd_type) {
		assert(fnd_type < FDT_COUNT || fnd_type == FDT_INVALID);
		SB(this->w0, 0, 2, VT_SURFACE);
		SB(this->w0, 10, 4, fnd_type);
	}

	/* Ground data access. */

	/**
	 * Get the imploded ground slope of a surface voxel.
	 * @note As steep slopes are two voxels high, they have a reference voxel above them.
	 * @return The imploded ground slope.
	 */
	inline uint8 GetGroundSlope() const {
		assert(this->GetType() == VT_SURFACE);
		return GB(this->w0, 14, 5);
	}

	/**
	 * Get the ground type of a surface voxel.
	 * @return The ground type.
	 */
	inline GroundType GetGroundType() const {
		assert(this->GetType() == VT_SURFACE);
		uint val = GB(this->w0, 19, 4);
		return (GroundType)val;
	}

	/**
	 * Set the imploded ground slope of a surface voxel.
	 * @note As steep slopes are two voxels high, they have a reference voxel above them.
	 * @param gnd_slope The new imploded ground slope.
	 */
	inline void SetGroundSlope(uint8 gnd_slope) {
		assert(gnd_slope < 19); // 15 non-steep + 4 steep sprites.
		SB(this->w0, 0, 2, VT_SURFACE);
		SB(this->w0, 14, 5, gnd_slope);
	}

	/**
	 * Set the ground type of a surface voxel.
	 * @param gnd_type The ground type.
	 */
	inline void SetGroundType(GroundType gnd_type) {
		assert(gnd_type < GTP_COUNT || gnd_type == GTP_INVALID);
		SB(this->w0, 0, 2, VT_SURFACE);
		SB(this->w0, 19, 4, gnd_type);
	}

	/* Path and ride data access. */

	/**
	 * Get the path type or ride number.
	 * @return The path type or ride number.
	 * @see PathRideType
	 */
	inline uint16 GetPathRideNumber() const {
		assert(this->GetType() == VT_SURFACE);
		return GB(this->w1, 6, 10);
	}

	/**
	 * Get the path slope or ride flags of a surface voxel (#GetPathRideNumber decides the interpretation).
	 * @return The slope or flags.
	 * @see RideVoxelFlags PathSprites
	 */
	inline uint8 GetPathRideFlags() const {
		assert(this->GetType() == VT_SURFACE);
		return GB(this->w1, 0, 6);
	}

	/**
	 * Set the path type or ride number.
	 * @param number The number to store.
	 */
	inline void SetPathRideNumber(uint16 number) {
		assert(number <= 0x3FFF);
		SB(this->w0, 0, 2, VT_SURFACE);
		SB(this->w1, 6, 10, number);
	}

	/**
	 * Set the path slope or ride flags of a surface voxel.
	 * @param flags Flags to store.
	 */
	inline void SetPathRideFlags(uint8 flags) {
		assert(flags < 64);
		SB(this->w0, 0, 2, VT_SURFACE);
		SB(this->w1, 0, 6, flags);
	}


	/**
	 * Get the referenced voxel for read-only access.
	 * @return Referenced voxel position.
	 */
	inline const ReferenceVoxelData *GetReference() const
	{
		assert(this->GetType() == VT_REFERENCE);
		return &this->reference;
	}

	/**
	 * Set the referenced voxel.
	 * @param rvd Reference data to store.
	 */
	inline void SetReference(const ReferenceVoxelData &rvd)
	{
		SB(this->w0, 0, 2, VT_REFERENCE);
		this->reference = rvd;
	}


	/** Set the voxel to empty. */
	inline void SetEmpty()
	{
		SB(this->w0, 0, 2, VT_EMPTY);
	}

	ReferenceVoxelData reference; ///< Reference voxel data
	PersonList persons; ///< Persons present in this voxel.
};

/**
 * Does the given voxel contain a valid path?
 * @param v %Voxel to examine.
 * @return @c true if the voxel contains a valid path, else \c false.
 */
static inline bool HasValidPath(const Voxel *v) {
	if (v->GetType() != VT_SURFACE) return false;
	return v->GetPathRideNumber() >= PT_START && v->GetPathRideNumber() != PT_INVALID;
}

/**
 * One column of voxels.
 * @ingroup map_group
 */
class VoxelStack {
public:
	VoxelStack();
	~VoxelStack();

	void Clear();
	const Voxel *Get(int16 z) const;
	Voxel *GetCreate(int16 z, bool create);

	VoxelStack *Copy(bool copyPersons) const;
	void MoveStack(VoxelStack *old_stack);

	Voxel *voxels; ///< %Voxel array at this stack.
	int16 base;    ///< Height of the bottom voxel.
	uint16 height; ///< Number of voxels in the stack.
protected:
	bool MakeVoxelStack(int16 new_base, uint16 new_height);
};

/**
 * A world of voxels.
 * @ingroup map_group
 */
class VoxelWorld {
public:
	VoxelWorld();

	void SetWorldSize(uint16 xs, uint16 ys);
	void MakeFlatWorld(int16 z);

	VoxelStack *GetModifyStack(uint16 x, uint16 y);
	const VoxelStack *GetStack(uint16 x, uint16 y) const;
	uint8 GetGroundHeight(uint16 x, uint16 y) const;

	/**
	 * Move a voxel stack to this world. May destroy the original stack in the process.
	 * @param x X coordinate of the stack.
	 * @param y Y coordinate of the stack.
	 * @param old_stack Source stack.
	 */
	void MoveStack(uint16 x, uint16 y, VoxelStack *old_stack)
	{
		this->GetModifyStack(x, y)->MoveStack(old_stack);
	}

	/**
	 * Get a voxel in the world by voxel coordinate.
	 * @param x X coordinate of the voxel.
	 * @param y Y coordinate of the voxel.
	 * @param z Z coordinate of the voxel.
	 * @return Address of the voxel (if it exists).
	 */
	inline const Voxel *GetVoxel(uint16 x, uint16 y, int16 z) const
	{
		return this->GetStack(x, y)->Get(z);
	}

	/**
	 * Get the person list associated with a voxel.
	 * @param x X coordinate of the voxel.
	 * @param y Y coordinate of the voxel.
	 * @param z Z coordinate of the voxel.
	 * @return The person list of the voxel.
	 * @note The voxel has to exist.
	 */
	inline PersonList &GetPersonList(uint16 x, uint16 y, int16 z)
	{
		Voxel *v = this->GetCreateVoxel(x, y, z, false);
		assert(v != NULL);
		return v->persons;
	}

	/**
	 * Get a voxel in the world by voxel coordinate.
	 * @param x X coordinate of the voxel.
	 * @param y Y coordinate of the voxel.
	 * @param z Z coordinate of the voxel.
	 * @param create If the requested voxel does not exist, try to create it.
	 * @return Address of the voxel (if it exists or could be created).
	 */
	inline Voxel *GetCreateVoxel(uint16 x, uint16 y, int16 z, bool create)
	{
		return this->GetModifyStack(x, y)->GetCreate(z, create);
	}

	/**
	 * Get X size of the world.
	 * @return Length of the world in X direction.
	 */
	inline uint16 GetXSize() const
	{
		return this->x_size;
	}

	/**
	 * Get Y size of the world.
	 * @return Length of the world in Y direction.
	 */
	inline uint16 GetYSize() const
	{
		return this->y_size;
	}

	/**
	 * Does the provided voxel exist in the world?
	 * @param x X coordinate of the voxel.
	 * @param y Y coordinate of the voxel.
	 * @param z Z coordinate of the voxel.
	 * @return Whether a voxel with content exists at the given position.
	 */
	inline bool VoxelExists(int16 x, int16 y, int16 z) const
	{
		if (x < 0 || x >= (int)this->GetXSize()) return false;
		if (y < 0 || y >= (int)this->GetYSize()) return false;
		const VoxelStack *vs = this->GetStack(x, y);
		if (z < vs->base || z >= vs->base + (int)vs->height) return false;
		return true;
	}

private:
	uint16 x_size; ///< Current max x size.
	uint16 y_size; ///< Current max y size.

	VoxelStack stacks[WORLD_X_SIZE * WORLD_Y_SIZE]; ///< All voxel stacks in the world.
};

/**
 * Ground data + modification storage.
 * @ingroup map_group
 */
struct GroundData {
	uint8 height;     ///< Height of the voxel with ground.
	uint8 orig_slope; ///< Original slope data.
	uint8 modified;   ///< Raised or lowered corners.

	GroundData(uint8 height, uint8 orig_slope);

	uint8 GetOrigHeight(TileSlope corner) const;
	bool GetCornerModified(TileSlope corner) const;
	void SetCornerModified(TileSlope corner);
};

/**
 * Map of voxels to ground modification data.
 * @ingroup map_group
 */
typedef std::map<Point32, GroundData> GroundModificationMap;

/**
 * Store and manage terrain changes.
 * @todo Enable pulling the screen min/max coordinates from it, so we can give a good estimate of the area to redraw.
 * @ingroup map_group
 */
class TerrainChanges {
public:
	TerrainChanges(const Point32 &base, uint16 xsize, uint16 ysize);
	~TerrainChanges();

	bool ChangeCorner(const Point32 &pos, TileSlope corner, int direction);
	void ChangeWorld(int direction);

	GroundModificationMap changes; ///< Registered changes.

private:
	Point32 base; ///< Base position of the smooth changing world.
	uint16 xsize; ///< Horizontal size of the smooth changing world.
	uint16 ysize; ///< Vertical size of the smooth changing world.

	GroundData *GetGroundData(const Point32 &pos);
};

/**
 * Map of x/y positions to voxel stacks.
 * @ingroup map_group
 */
typedef std::map<Point32, VoxelStack *> VoxelStackMap;

/**
 * Proposed additions to #_world.
 * Temporary buffer to make changes in the world, and show them to the user
 * without having them really in the game until the user confirms.
 * @note This world does not use the #Voxel::persons person list.
 */
class WorldAdditions {
public:
	WorldAdditions();
	~WorldAdditions();

	void Clear();
	void Commit();

	VoxelStack *GetModifyStack(uint16 x, uint16 y);
	const VoxelStack *GetStack(uint16 x, uint16 y) const;

	/**
	 * Get a voxel in the world by voxel coordinate.
	 * @param x X coordinate of the voxel.
	 * @param y Y coordinate of the voxel.
	 * @param z Z coordinate of the voxel.
	 * @return Address of the voxel (if it exists).
	 */
	inline const Voxel *GetVoxel(uint16 x, uint16 y, int16 z) const
	{
		return this->GetStack(x, y)->Get(z);
	}

	/**
	 * Get a voxel in the world by voxel coordinate.
	 * @param x X coordinate of the voxel.
	 * @param y Y coordinate of the voxel.
	 * @param z Z coordinate of the voxel.
	 * @param create If the requested voxel does not exist, try to create it.
	 * @return Address of the voxel (if it exists or could be created).
	 */
	inline Voxel *GetCreateVoxel(uint16 x, uint16 y, int16 z, bool create)
	{
		return this->GetModifyStack(x, y)->GetCreate(z, create);
	}

	void MarkDirty(Viewport *vp);

protected:
	VoxelStackMap modified_stacks; ///< Modified voxel stacks.
};

extern VoxelWorld _world;
extern WorldAdditions _additions;

void EnableWorldAdditions();
void DisableWorldAdditions();

#endif
