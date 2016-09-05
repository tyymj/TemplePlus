
#include "stdafx.h"
#include "util/fixes.h"
#include "common.h"
#include "obj.h"
#include "turn_based.h"
#include "critter.h"
#include "condition.h"
#include "bonus.h"
#include <pathfinding.h>
#include "ui/ui.h"
#include <combat.h>
#include <action_sequence.h>
#include <tig/tig_font.h>
#include <party.h>
#include <damage.h>

struct TigTextStyle;

// fixes size_colossal (was misspelled as size_clossal)
class SizeColossalFix : public TempleFix {
public:

	void apply() override {
		writeHex(0x10278078, "73 69 7A 65 5F 63 6F 6C 6F 73 73 61 6C");
	}
} sizeColossalFix;

// Makes Kukri Proficiency Martial
class KukriFix : public TempleFix {
public:
	void apply() override {
		writeHex(0x102BFD78 + 30*4, "00 09"); // marks Kukri as Martial in the sense that picking "Martial Weapons Proficiency" will now list Kukri
		// see rest of fix in weapon.cpp IsMartialWeapon
	}
} kukriFix;


objHndl __cdecl ItemWornAtModifiedForTumlbeCheck(objHndl objHnd, uint32_t itemWornSlot)
{
	if (critterSys.GetRace(objHnd) == race_dwarf)
	{
		return objHndl::null;
	}
	else
	{
		return objects.inventory.ItemWornAt(objHnd, itemWornSlot);
	}
}

// Allows Dwarves to tumble in heavy armor
class DwarfTumbleFix : public TempleFix
{
public:
	void apply() override {
		redirectCall(0x1008AB49, ItemWornAtModifiedForTumlbeCheck);
	}

	//
} dwarfTumbleFix;


// Prevents medium / heavy encumbrance from affecting dwarves
class DwarfEncumbranceFix: public TempleFix
{
public:
	static int EncumberedMoveSpeedCallback(DispatcherCallbackArgs args);
	static int DwarfGetMoveSpeed(DispatcherCallbackArgs args);
	void apply() override {
		replaceFunction(0x100EFEC0, DwarfGetMoveSpeed); // recreates Spellslinger fix
		replaceFunction(0x100EBAA0, EncumberedMoveSpeedCallback);
	}
} dwarfEncumbranceFix;

int DwarfEncumbranceFix::EncumberedMoveSpeedCallback(DispatcherCallbackArgs args){
	auto dispIo = dispatch.DispIOCheckIoType13(args.dispIO);
	if ( dispIo->bonlist->bonFlags  == 3) //  in case the cap has already been set (e.g. by web/entangle) - recreating the spellslinger fix
		return 0;
	if (args.subDispNode->subDispDef->data2 == 324) // overburdened
	{
		dispIo->bonlist->SetOverallCap(5, 5, 0, 324);
		dispIo->bonlist->SetOverallCap(6, 5, 0, 324);
		return 0;
	} 
	
	if (critterSys.GetRace(args.objHndCaller) == Race::race_dwarf) // dwarves do not suffer movement penalty for medium/heavy encumbrance
		return 0;

	if (dispIo->bonlist->bonusEntries[0].bonValue <= 20) // this is probably the explicit form for base speed...
	{
		bonusSys.bonusAddToBonusList(dispIo->bonlist, -5, 0, args.subDispNode->subDispDef->data2);
	} 
	else
	{
		bonusSys.bonusAddToBonusList(dispIo->bonlist, -10, 0, args.subDispNode->subDispDef->data2);
	}
	
	return 0;
}

int DwarfEncumbranceFix::DwarfGetMoveSpeed(DispatcherCallbackArgs args){

	args.dispIO->AssertType(dispIOTypeMoveSpeed);
	auto dispIo = static_cast<DispIoMoveSpeed*>(args.dispIO);

	if (dispIo->bonlist->bonFlags & 2) //  in case the cap has already been set (e.g. by web/entangle) - recreating the spellslinger fix
		return 0;

	dispIo->bonlist->SetOverallCap(2, args.GetData1(), 0, args.GetData2());

	return 0;
}

// Putting SpellSlinger's small fixes concentrated here :)
class SpellSlingerGeneralFixes : public TempleFix
{
public:

	

	void apply() override{
		// breakfree_on_entanglement_fix.txt
		writeHex(0x100D4259, "90 90 90 90  90 90");
		

		// caravan_encounter_fix.txt // looks like it has been updated since then too! 13D6 instead of 13D5
		writeHex(0x1006ED1E, "04");

		writeHex(0x1006FE1B + 1, "D6 13");
		writeHex(0x1007173C + 1, "D6 13");
		writeHex(0x1012BDF2 + 1, "D6 13");
		writeHex(0x1012C1EC + 1, "D6 13");
		writeHex(0x1012C361 + 1, "D6 13");


		// D20STDF_fix.txt

		writeHex(0x100B8454, "BA 00000000");
		writeHex(0x100B8471     , "8D4A 0D");


		// heavy_armor_prof_fix.txt

		writeHex(0x1007C49F, "6A 06");

		// NPC_usePotion_AoO_fix.txt

		writeHex(0x10098DE7+6, "44000000");

		// rep fallen paladin fix

		writeHex(0x1005481A, "00");

		// rgr_fav_enemy_fix.txt

		writeHex(0x101AD9BE, "90 90");

		// sp125_discern_lies_fix.txt // looks like this was actually forgotten to be implemented in the Co8 DLL
		writeHex(0x102D5454, "1E 00 00 00  22 00 00 00  00 2B 0D 10   00 00 00 00");


		// fixes encumbrance for polymorphed
		static int(__cdecl* orgEncumbranceQuery)(DispatcherCallbackArgs ) =
			replaceFunction<int(DispatcherCallbackArgs)>(0x100ECA30, [](DispatcherCallbackArgs args) {
			if (d20Sys.d20Query(args.objHndCaller, DK_QUE_Polymorphed))
				return 0;
			return orgEncumbranceQuery(args);
		});

	}
} spellSlingerGeneralFixes;



static uint32_t (__cdecl *OrgFragarachAnswering)(DispatcherCallbackArgs);

uint32_t __cdecl HookedFragarachAnswering(DispatcherCallbackArgs args) {
	// checks if the current TB actor is the same as the "attachee" (critter taking damage)
	// if so, aborts the answering (you can have an AoO on your turn!)
	auto dispIO = args.dispIO;
	auto curActor = tbSys.turnBasedGetCurrentActor();
	auto attachee = args.objHndCaller;
	auto tgtObj = *(objHndl*)(dispIO + 2);
	//hooked_print_debug_message("Prevented Scather AoO bug! TB Actor is %s , Attachee is %s,  target is %s", description.getDisplayName(curActor), description.getDisplayName(attachee), description.getDisplayName(tgtObj));
	if (!tgtObj || !objects.IsCritter(tgtObj) || curActor == attachee)
	{
		logger->info("Prevented Scather AoO bug! TB Actor is {}, Attachee is {},  target is {}", 
			description.getDisplayName(curActor), description.getDisplayName(attachee), description.getDisplayName(tgtObj) );
		return 0;
	}


	/*
	disable AoO effect for other identical conditions (so you don't get the 2 AoO Hang) // TODO: Makes this work like Great Cleave for maximum munchkinism
	*/
	if (conds.CondNodeGetArg(args.subDispNode->condNode, 0) == 1)
	{
		auto dispatcher = objects.GetDispatcher(args.objHndCaller);
		auto cond = &dispatcher->itemConds;
		while (*cond)
		{
			if ((*cond)->condStruct->condName == args.subDispNode->condNode->condStruct->condName
				&& (*cond) != args.subDispNode->condNode)
			{
				(*cond)->args[0] = 0;
			}
			cond = &(*cond)->nextCondNode;
		}
		auto result = OrgFragarachAnswering(args); // Call original method
		return result;
	}
	
	return 0;
	
}

// Fixes the Fragarach hang that is caused by attacking a fire creature (which deals damage to the caster 
// -> triggers the answering ability - > attempts an AoO -> but there is no one to AoO!)
class FragarachAoOFix : public TempleFix
{
public:
	void apply() override {
		OrgFragarachAnswering = (uint32_t(__cdecl*)(DispatcherCallbackArgs)) replaceFunction(0x10104330, HookedFragarachAnswering);

	}
} fragarachAoOFix;



uint32_t GetCourageBonus(objHndl objHnd)
{
	auto bardLvl = (int32_t)objects.StatLevelGet(objHnd, stat_level_bard);
	if (bardLvl < 8) return 1;
	if (bardLvl < 14) return 2;
	if (bardLvl < 20) return 3;
	return 4;
};

uint32_t __cdecl BardicInspiredCourageInitArgs(DispatcherCallbackArgs args)
{
	conds.CondNodeSetArg(args.subDispNode->condNode, 0, 5);
	conds.CondNodeSetArg(args.subDispNode->condNode, 1, 0);
	auto courageBonus = 1;
	if ( objects.IsCritter(args.objHndCaller) )
	{
		if (party.IsInParty(args.objHndCaller))
		{
			
		}
		courageBonus = GetCourageBonus(args.objHndCaller);
	}
	else
	{
		logger->info("Bardic Inspired Courage dispatched from non-critter! Mon seigneur {}", 
			description.getDisplayName(args.objHndCaller));
	}
	//conds.CondNodeSetArg(args.subDispNode->condNode, 3, courageBonus);
	return 0;
};

// Bardic Inspire Courage Function Replacements
class BardicInspireCourageFix : public TempleFix
{
public:

	static int BardicInspiredCourageToHit(DispatcherCallbackArgs args);
	static int BardicInspiredCourageDamBon(DispatcherCallbackArgs args);
	void apply() override
	{
		replaceFunction(0x100EA5C0, BardicInspiredCourageInitArgs);
		replaceFunction(0x100EA5F0, BardicInspiredCourageToHit);
		replaceFunction(0x100EA630, BardicInspiredCourageDamBon);
		
	}
} bardicInspireCourageFix;

// Sorcerer Spell Failure Double Debit Fix
class SorcererFailureDoubleChargeFix : public TempleFix
{
public:
	void apply() override
	{
		writeHex(0x1008D80E, "90 90 90 90 90");
	}
} sorcererSpellFailureFix;

// Crippling Strike Fix
class CripplingStrikeFix : public TempleFix
{
public:
	// fixes Str damage on crippling strike (should be 2 instead of 1)
	void apply() override
	{
		writeHex(0x100F9B70, "6A 02");
	}
} cripplingStrikeFix;

// Walk on short distances mod
class WalkOnShortDistanceMod : public TempleFix
{
public: 

	static int(__cdecl*orgSub_100437F0)(LocAndOffsets loc, int runFlag);
	static int(__cdecl *orgShouldRun)(objHndl obj);
	static int(__cdecl*orgAnimPushRunToTileWithPath)(objHndl obj, LocAndOffsets loc, Path* path);
	static int ShouldRun(objHndl obj)
	{
		return 0;
	};
	static int Sub_100437F0(LocAndOffsets loc, int runFlag)
	{
		return orgSub_100437F0(loc, runFlag);
	};

	static int AnimPushRunToTileWithPath(objHndl obj, LocAndOffsets loc, Path* path)
{
		return orgAnimPushRunToTileWithPath(obj, loc, path);
};

		void apply() override
		{
		//	replaceFunction(0x100FD1C0, sub_100FD1C0);
		//orgSub_100437F0 = replaceFunction(0x100437F0, Sub_100437F0);
		//orgShouldRun = replaceFunction(0x10014750, ShouldRun);
		//writeHex(0x1001A922, "90 90 90 90");
		//writeHex(0x1001A98F, "90 90 90 90");
		//writeHex(0x1001A338 + 1, "05");
		//writeHex(0x1001A346 + 1, "03");
		//orgAnimPushRunToTileWithPath = replaceFunction(0x1001A2E0, AnimPushRunToTileWithPath);
		}
} walkOnShortDistMod;

int(__cdecl*WalkOnShortDistanceMod::orgSub_100437F0)(LocAndOffsets loc, int runFlag);
int(__cdecl*WalkOnShortDistanceMod::orgShouldRun)(objHndl obj);
int(__cdecl*WalkOnShortDistanceMod::orgAnimPushRunToTileWithPath)(objHndl obj, LocAndOffsets loc, Path* path);

// PartyPool UI Fix
static class PartyPoolUiLagFix : public TempleFix {
public:

	void apply() override {
		OrgUiPartyPoolLoadObj = replaceFunction(0x10025F10, UiPartyPoolLoadObj);
	}

	static int UiPartyPoolLoadObj(char *objData, objHndl *handleOut, locXY loc);
	static int (*OrgUiPartyPoolLoadObj)(char *objData, objHndl *handleOut, locXY loc);
} partyPoolUiLagFix;

int PartyPoolUiLagFix::UiPartyPoolLoadObj(char * objData, objHndl * handleOut, locXY loc)
{
	// Load the characters on a sector that is offscreen.
	locXY nullLoc{ 0, 0 };
	return OrgUiPartyPoolLoadObj(objData, handleOut, nullLoc);
}

int (*PartyPoolUiLagFix::OrgUiPartyPoolLoadObj)(char *objData, objHndl *handleOut, locXY loc);

// Remove Suggestion Fix
static class RemoveSuggestionSpellFix : public TempleFix
{
public: 
	static int RemoveSpellSuggestionNaked(SubDispNode*, objHndl obj);
	static int RemoveSpellSuggestion(DispIO*, enum_disp_type, D20DispatcherKey, SubDispNode*, objHndl);
	void apply() override 
	{
		replaceFunction(0x100D2460, RemoveSpellSuggestionNaked);
	}
} removeSuggFix;

int __declspec(naked) RemoveSuggestionSpellFix::RemoveSpellSuggestionNaked(SubDispNode*, objHndl obj)
{
	{ __asm push ecx __asm push esi __asm push ebx __asm push edi};

	//push the objHndl
	__asm mov ebx, [esp + 0x18]
	__asm mov edi, [esp + 0x1C]
	__asm push edi;
	__asm push ebx;
	// esp + 8

	// push the SubDispNode*
	__asm mov edi, [esp + 0x1C]  //0x14 + 8 from pushes
	__asm push edi
	// esp + C

	// D20DispatcherKey
	__asm push edx
	// esp + 0x10

	// enum_disp_key
	__asm push eax
	// esp + 0x14

	// DispIO*
	__asm push ecx
	// esp + 0x18
	
	__asm call RemoveSuggestionSpellFix::RemoveSpellSuggestion;
	__asm add esp, 0x18;

	{ __asm pop edi __asm pop ebx __asm pop esi __asm pop ecx}
	__asm ret
}

int RemoveSuggestionSpellFix::RemoveSpellSuggestion(DispIO* dispIo, enum_disp_type dispType, D20DispatcherKey dispKey, SubDispNode* sdn, objHndl obj)
{
	if (!dispIo){
		if (dispType == dispTypeBeginRound || dispType == dispTypeConditionAddPre || dispKey == DK_SIG_Dismiss_Spells)	{
			auto spellId = _CondNodeGetArg(sdn->condNode, 0);
			d20Sys.d20SendSignal(obj, DK_SIG_Spell_End, spellId, 0);
			critterSys.RemoveFollower(obj, 1);
			ui.UpdatePartyUi();
			return 1;
		}
		return 0;
	}

	auto spellId = _CondNodeGetArg(sdn->condNode, 0);
	SpellPacketBody spellPkt;
	if (!spellSys.GetSpellPacketBody(spellId, &spellPkt)){
		logger->warn("RemoveSpellSuggestion: Unable to retrieve spell!");
		return 0;
	}

	if (dispKey == DK_SIG_Killed ||dispKey == DK_SIG_Critter_Killed){
		d20Sys.d20SendSignal(obj, DK_SIG_Spell_End, spellId, 0);
		critterSys.RemoveFollower(obj, 1);
		ui.UpdatePartyUi();
		return 1;
	}

	auto dispIoSig = dispatch.DispIoCheckIoType6(dispIo);
	if (!dispIoSig){
		if (dispIo->dispIOType == dispIOTypeDispelCheck || dispIo->dispIOType == dispIOTypeTurnBasedStatus)	{
			d20Sys.d20SendSignal(obj, DK_SIG_Spell_End, 0, 0);
			critterSys.RemoveFollower(obj, 1);
			ui.UpdatePartyUi();
		}
		return 0;
	}

	D20Actn* d20a = reinterpret_cast<D20Actn*>(dispIoSig->data1);
	if (!d20a)
		return 1;
	
	auto condSuggestion = temple::GetPointer<CondStruct>(0x102E0158);
	if (d20a->d20ActType == D20A_CAST_SPELL){

		if (d20Sys.d20QueryWithData(obj, DK_QUE_Critter_Has_Condition, condSuggestion, 0) == 1 && d20a->spellId != spellId)	{
			d20Sys.d20SendSignal(obj, DK_SIG_Spell_End, spellPkt.spellId, 0);
			critterSys.RemoveFollower(obj, 1);
			ui.UpdatePartyUi();
			return 1;
		}	
	} else if (d20Sys.IsActionOffensive(d20a->d20ActType, d20a->d20ATarget))	{
		if (critterSys.IsFriendly(d20a->d20APerformer, d20a->d20ATarget))	{
			if (d20Sys.d20QueryWithData(obj, DK_QUE_Critter_Has_Condition, condSuggestion, 0))	{
				d20Sys.d20SendSignal(obj, DK_SIG_Spell_End, objHndl::null);
				critterSys.RemoveFollower(obj, 1);
				ui.UpdatePartyUi();
				return 1;
			}
		}
	}
		
	return 0;
}




static class SkillMeasureFix : public TempleFix {
	
	/*
		original function had a size 4 text buffer, which is not enough for high levels where the skills may be 10.5
	*/
	static int HookedMeasure(const TigTextStyle&style, TigFontMetrics& metrics){
		auto orgMeasure = temple::GetRef<int(__cdecl)(const TigTextStyle&, TigFontMetrics&)>(0x101EA4E0);
		
		char text[10];
		memcpy(text, metrics.text, 5);
		text[5] = 0;
		if (text[4]){
			text[4] = '\r';
		}
		metrics.text = text;
		
		auto result = orgMeasure(style, metrics);
		if (metrics.width > 50 || metrics.width < 0)
		{
			metrics.width = 44;
		}
		return result;
	};


	void apply() override 	{
		redirectCall(0x101AB5AE, HookedMeasure);
	};
} skillMeasureFix;

int BardicInspireCourageFix::BardicInspiredCourageToHit(DispatcherCallbackArgs args){
	GET_DISPIO(dispIOTypeAttackBonus, DispIoAttackBonus);
	if (!args.GetCondArg(1))
		return 0;
	auto bonVal = 1;
	if (party.IsInParty(args.objHndCaller)){
		auto brdLvl = 0;
		for (auto i=0; i<party.GroupListGetLen(); i++){
			auto dude = party.GroupListGetMemberN(i);
			if (!dude)
				continue;
			auto dudeBrdLvl = objects.StatLevelGet(dude, stat_level_bard);
			if (dudeBrdLvl > brdLvl)
				brdLvl = dudeBrdLvl;
		}
		if (brdLvl < 8) 
			bonVal =1;
		else if (brdLvl < 14) 
			bonVal = 2;
		else if (brdLvl < 20) 
			bonVal = 3;
	}
	dispIo->bonlist.AddBonus(bonVal, 13, 191);
	return 0;
}

int BardicInspireCourageFix::BardicInspiredCourageDamBon(DispatcherCallbackArgs args)
{
	GET_DISPIO(dispIOTypeDamage, DispIoDamage);
	
	if (!args.GetCondArg(1))
		return 0;

	auto bonVal = 1;
	if (party.IsInParty(args.objHndCaller)) {
		auto brdLvl = 0;
		for (auto i = 0; i<party.GroupListGetLen(); i++) {
			auto dude = party.GroupListGetMemberN(i);
			if (!dude)
				continue;
			auto dudeBrdLvl = objects.StatLevelGet(dude, stat_level_bard);
			if (dudeBrdLvl > brdLvl)
				brdLvl = dudeBrdLvl;
		}
		if (brdLvl < 8)
			bonVal = 1;
		else if (brdLvl < 14)
			bonVal = 2;
		else if (brdLvl < 20)
			bonVal = 3;
	}
	dispIo->damage.AddDamageBonus(bonVal, 13, 191);
	return 0;
}
