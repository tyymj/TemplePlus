#pragma once

#define NUM_CLASSES  (stat_level_wizard - stat_level_barbarian + 1)
#define NUM_RACES 7



enum D20ActionType : int32_t
{
	D20A_NONE = -1,
	D20A_UNSPECIFIED_MOVE = 0,
	D20A_UNSPECIFIED_ATTACK,
	D20A_STANDARD_ATTACK,
	D20A_FULL_ATTACK,
	D20A_STANDARD_RANGED_ATTACK,
	D20A_RELOAD,
	D20A_5FOOTSTEP,
	D20A_MOVE,
	D20A_DOUBLE_MOVE,
	D20A_RUN,
	D20A_CAST_SPELL,
	D20A_HEAL,
	D20A_CLEAVE,
	D20A_ATTACK_OF_OPPORTUNITY,
	D20A_WHIRLWIND_ATTACK,
	D20A_TOUCH_ATTACK,
	D20A_TOTAL_DEFENSE,
	D20A_CHARGE,
	D20A_FALL_TO_PRONE,
	D20A_STAND_UP,
	D20A_TURN_UNDEAD,
	D20A_DEATH_TOUCH,
	D20A_PROTECTIVE_WARD,
	D20A_FEAT_OF_STRENGTH,
	D20A_BARDIC_MUSIC,
	D20A_PICKUP_OBJECT,
	D20A_COUP_DE_GRACE,
	D20A_USE_ITEM,
	D20A_BARBARIAN_RAGE,
	D20A_STUNNING_FIST,
	D20A_SMITE_EVIL, // 30
	D20A_LAY_ON_HANDS_SET,
	D20A_DETECT_EVIL,
	D20A_STOP_CONCENTRATION,
	D20A_BREAK_FREE,
	D20A_TRIP,
	D20A_REMOVE_DISEASE,
	D20A_ITEM_CREATION,
	D20A_WHOLENESS_OF_BODY_SET,
	D20A_USE_MAGIC_DEVICE_DECIPHER_WRITTEN_SPELL,
	D20A_TRACK,
	D20A_ACTIVATE_DEVICE_STANDARD,
	D20A_SPELL_CALL_LIGHTNING,
	D20A_AOO_MOVEMENT,
	D20A_CLASS_ABILITY_SA,
	D20A_ACTIVATE_DEVICE_FREE,
	D20A_OPEN_INVENTORY,
	D20A_ACTIVATE_DEVICE_SPELL,  // 47
	D20A_DISABLE_DEVICE,
	D20A_SEARCH,
	D20A_SNEAK,
	D20A_TALK,
	D20A_OPEN_LOCK, // verified
	D20A_SLEIGHT_OF_HAND,
	D20A_OPEN_CONTAINER, // verified; DLL string are accurate at least up to here
	D20A_THROW,
	D20A_THROW_GRENADE,
	D20A_FEINT, // verified; this was missing in the priginal python table, so that must be where it went off track
	D20A_READY_SPELL,
	D20A_READY_COUNTERSPELL,
	D20A_READY_ENTER,
	D20A_READY_EXIT,
	D20A_COPY_SCROLL, // verified
	D20A_READIED_INTERRUPT,
	D20A_LAY_ON_HANDS_USE,
	D20A_WHOLENESS_OF_BODY_USE,
	D20A_DISMISS_SPELLS,
	D20A_FLEE_COMBAT,
	D20A_USE_POTION,
	D20A_NUMACTIONS // always keep this last. Not counting D20A_NONE since it is unused (all the d20 action functions start cycling from D20A_UNSPECIFIED_MOVE)
};

enum D20SavingThrow : uint32_t {
	D20_Save_Fortitude = 0,
	D20_Save_Reflex = 1,
	D20_Save_Will = 2
};

enum D20SavingThrowReduction : uint32_t {
	D20_Save_Reduction_None = 0,
	D20_Save_Reduction_Half = 1,
	D20_Save_Reduction_Quarter = 2
};

/*
	Types of attack attributes that can overcome damage reduction.
*/
enum D20AttackPower : uint32_t {
	D20DAP_NORMAL = 0x1,
	D20DAP_UNSPECIFIED = 0x2,
	D20DAP_SILVER = 0x4,
	D20DAP_MAGIC = 0x8,
	D20DAP_HOLY = 0x10,
	D20DAP_UNHOLY = 0x20,
	D20DAP_CHAOS = 0x40,
	D20DAP_LAW = 0x80,
	D20DAP_ADAMANTIUM = 0x100,
	D20DAP_BLUDGEONING = 0x200,
	D20DAP_PIERCING = 0x400,
	D20DAP_SLASHING = 0x800,
	D20DAP_MITHRIL = 0x1000,
	D20DAP_COLD = 0x2000
};

// THIS IS INCORRECT! THIS ENUM IS WRONG
enum D20SavingThrowFlag : uint32_t {
	D20STD_F_MAX = 0,
	D20STD_F_NONE = 0,
	D20STD_F_REROLL = 1,
	D20STD_F_CHARM = 2,
	D20STD_F_TRAP = 3,
	D20STD_F_POISON = 4,
	D20STD_F_SPELL_LIKE_EFFECT = 5,
	D20STD_F_SPELL_SCHOOL_ABJURATION = 6,
	D20STD_F_SPELL_SCHOOL_CONJURATION = 7,
	D20STD_F_SPELL_SCHOOL_DIVINATION = 8,
	D20STD_F_SPELL_SCHOOL_ENCHANTMENT = 9,
	D20STD_F_SPELL_SCHOOL_EVOCATION = 10,
	D20STD_F_SPELL_SCHOOL_ILLUSION = 11,
	D20STD_F_SPELL_SCHOOL_NECROMANCY = 12,
	D20STD_F_SPELL_SCHOOL_TRANSMUTATION = 13,
	D20STD_F_SPELL_DESCRIPTOR_ACID = 14,
	D20STD_F_SPELL_DESCRIPTOR_CHAOTIC = 15,
	D20STD_F_SPELL_DESCRIPTOR_COLD = 16,
	D20STD_F_SPELL_DESCRIPTOR_DARKNESS = 17,
	D20STD_F_SPELL_DESCRIPTOR_DEATH = 18,
	D20STD_F_SPELL_DESCRIPTOR_ELECTRICITY = 19,
	D20STD_F_SPELL_DESCRIPTOR_EVIL = 20,
	D20STD_F_SPELL_DESCRIPTOR_FEAR = 21,
	D20STD_F_SPELL_DESCRIPTOR_FIRE = 22,
	D20STD_F_SPELL_DESCRIPTOR_FORCE = 23,
	D20STD_F_SPELL_DESCRIPTOR_GOOD = 24,
	D20STD_F_SPELL_DESCRIPTOR_LANGUAGE_DEPENDENT = 25,
	D20STD_F_SPELL_DESCRIPTOR_LAWFUL = 26,
	D20STD_F_SPELL_DESCRIPTOR_LIGHT = 27,
	D20STD_F_SPELL_DESCRIPTOR_MIND_AFFECTING = 28,
	D20STD_F_SPELL_DESCRIPTOR_SONIC = 29,
	D20STD_F_SPELL_DESCRIPTOR_TELEPORTATION = 30,
	D20STD_F_SPELL_DESCRIPTOR_AIR = 31,
	D20STD_F_SPELL_DESCRIPTOR_EARTH = 32,
	D20STD_F_SPELL_DESCRIPTOR_WATER = 33,
	D20STD_F_DISABLE_SLIPPERY_MIND = 34
};

enum D20CAF : uint32_t
{
	D20CAF_HIT = 0x1,
	D20CAF_CRITICAL = 0x2,
	D20CAF_RANGED = 0x4,
	D20CAF_ACTIONFRAME_PROCESSED = 0x8,
	D20CAF_NEED_PROJECTILE_HIT = 0x10,
	D20CAF_NEED_ANIM_COMPLETED = 0x20,
	D20CAF_ATTACK_OF_OPPORTUNITY = 0x40,
	D20CAF_CONCEALMENT_MISS = 0x80,
	D20CAF_TOUCH_ATTACK = 0x100,
	D20CAF_FREE_ACTION = 0x200,
	D20CAF_CHARGE = 0x400,
	D20CAF_REROLL = 0x800,
	D20CAF_REROLL_CRITICAL = 0x1000,
	D20CAF_TRAP = 0x2000,
	D20CAF_ALTERNATE = 0x4000,
	D20CAF_NO_PRECISION_DAMAGE = 0x8000,
	D20CAF_FLANKED = 0x10000,
	D20CAF_DEFLECT_ARROWS = 0x20000,
	D20CAF_FULL_ATTACK = 0x40000,
	D20CAF_AOO_MOVEMENT = 0x80000,
	D20CAF_BONUS_ATTACK = 0x100000,
	D20CAF_THROWN = 0x200000,

	D20CAF_SAVE_SUCCESSFUL = 0x800000,
	D20CAF_SECONDARY_WEAPON = 16777216,
	D20CAF_MANYSHOT = 33554432,
	D20CAF_ALWAYS_HIT = 67108864,
	D20CAF_COVER = 134217728,
	D20CAF_COUNTERSPELLED = 268435456,
	D20CAF_THROWN_GRENADE = 536870912,
	D20CAF_FINAL_ATTACK_ROLL = 1073741824,
	D20CAF_TRUNCATED = 0x80000000,
};

static const char *d20ActionNames[] = {
	"D20A_UNSPECIFIED_MOVE",
	"D20A_UNSPECIFIED_ATTACK",
	"D20A_STANDARD_ATTACK",
	"D20A_FULL_ATTACK",
	"D20A_STANDARD_RANGED_ATTACK",
	"D20A_RELOAD",
	"D20A_5FOOTSTEP",
	"D20A_MOVE",
	"D20A_DOUBLE_MOVE",
	"D20A_RUN",
	"D20A_CAST_SPELL",
	"D20A_HEAL",
	"D20A_CLEAVE",
	"D20A_ATTACK_OF_OPPORTUNITY",
	"D20A_WHIRLWIND_ATTACK",
	"D20A_TOUCH_ATTACK",
	"D20A_TOTAL_DEFENSE",
	"D20A_CHARGE",
	"D20A_FALL_TO_PRONE",
	"D20A_STAND_UP",
	"D20A_TURN_UNDEAD",
	"D20A_DEATH_TOUCH",
	"D20A_PROTECTIVE_WARD",
	"D20A_FEAT_OF_STRENGTH",
	"D20A_BARDIC_MUSIC",
	"D20A_PICKUP_OBJECT",
	"D20A_COUP_DE_GRACE",
	"D20A_USE_ITEM",
	"D20A_BARBARIAN_RAGE",
	"D20A_STUNNING_FIST",
	"D20A_SMITE_EVIL",
	"D20A_LAY_ON_HANDS_SET",
	"D20A_DETECT_EVIL",
	"D20A_STOP_CONCENTRATION",
	"D20A_BREAK_FREE",
	"D20A_TRIP",
	"D20A_REMOVE_DISEASE",
	"D20A_ITEM_CREATION",
	"D20A_WHOLENESS_OF_BODY_SET",
	"D20A_USE_MAGIC_DEVICE_DECIPHER_WRITTEN_SPELL",
	"D20A_TRACK",
	"D20A_ACTIVATE_DEVICE_STANDARD",
	"D20A_SPELL_CALL_LIGHTNING",
	"D20A_AOO_MOVEMENT",
	"D20A_CLASS_ABILITY_SA",
	"D20A_ACTIVATE_DEVICE_FREE",
	"D20A_OPEN_INVENTORY",
	"D20A_ACTIVATE_DEVICE_SPELL",
	"D20A_DISABLE_DEVICE",
	"D20A_SEARCH",
	"D20A_SNEAK",
	"D20A_TALK",
	"D20A_OPEN_LOCK",
	"D20A_SLEIGHT_OF_HAND",
	"D20A_OPEN_CONTAINER",
	"D20A_THROW",
	"D20A_THROW_GRENADE",
	"D20A_FEINT,"
	"D20A_READY_SPELL",
	"D20A_READY_COUNTERSPELL",
	"D20A_READY_ENTER",
	"D20A_READY_EXIT",
	"D20A_COPY_SCROLL",
	"D20A_READIED_INTERRUPT",
	"D20A_LAY_ON_HANDS_USE",
	"D20A_WHOLENESS_OF_BODY_USE",
	"D20A_DISMISS_SPELLS",
	"D20A_FLEE_COMBAT",
	"D20A_USE_POTION",
};
