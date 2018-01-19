from toee import *
from utilities import *
from py00648script_daemon import record_time_stamp, get_v, set_v, get_f, set_f, npc_set, npc_unset, npc_get, tsc, tpsts, within_rect_by_corners
from Livonya import break_free

def should_modify_CR( attachee ):
	return 0 # now done in the DLL properly!!! MWAHAHAHA
	# uses npc_get flag # 31
	#party_av_level = get_av_level()
	#if party_av_level > 10 and npc_get(attachee, 31) == 0:
	#	return 1
	#else:
	#	return 0

	
def get_av_level():
	# calculates average level of top 50% of the party
	# (rounded down; for odd-sized parties, the middle is included in the top 50%)

	# record every party member's level
	level_array = []
	for qq in game.party:
		level_array.append(qq.stat_level_get(stat_level))
	# sort
	level_array.sort()
	
	# calculate average of the top 50%
	level_sum = 0
	rr = range( len(level_array)/2 , len(level_array) )
	for qq in rr:
		level_sum = level_sum + level_array[qq]

	party_av_level = level_sum / len(rr)
		
	return party_av_level
	
		

def CR_tot_new( party_av_level, CR_tot ):
	# functions returns the desired total CR (to used for calculating new obj_f_npc_challenge_rating)
	#   such that parties with CL > 10 will get a more appropriate XP reward
	# party_av_level - the average CL to be simulated
	# CR_tot - the pre-adjusted total CR (natural CR + CL); 
	#    e.g. Rogue 15 with -2 CR mod -> CR_tot = 13; the -2 CR mod will (probably) get further adjusted by this function
	
	expected_xp = calc_xp_proper(party_av_level, CR_tot)
	
	best_CR_fit = CR_tot
	
	for qq in range(CR_tot-1, min(5, CR_tot-2) , -1):
		if abs( calc_xp_proper(10, qq) - expected_xp) < abs( calc_xp_proper(10, best_CR_fit) - expected_xp) and abs( calc_xp_proper(10, qq) - expected_xp) < abs( calc_xp_proper(10, CR_tot) - expected_xp):
			best_CR_fit = qq

	return best_CR_fit


		
def CR_mod_new( attachee, party_av_level = -1 ):
	if party_av_level == -1:
		party_av_level = get_av_level()
	CR_tot = attachee.stat_level_get(stat_level) + attachee.obj_get_int(obj_f_npc_challenge_rating)
	return ( CR_tot_new(party_av_level, CR_tot) - attachee.stat_level_get(stat_level) )

def modify_CR( attachee, party_av_level = -1  ):
	npc_set( attachee, 31 )
	if party_av_level == -1:
		party_av_level = get_av_level()
	attachee.obj_set_int(obj_f_npc_challenge_rating, CR_mod_new( attachee, party_av_level ) )


def calc_xp_proper( party_av_level, CR_tot ):
	# returns expected XP award
	xp_gain = party_av_level * 300
	xp_mult = 2**long(  abs(CR_tot - party_av_level) / 2) 
	
	if (CR_tot - party_av_level) % 2 == 1:
		xp_mult = xp_mult * 1.5
		
	if party_av_level > CR_tot:
		return long(xp_gain / xp_mult)
	else:
		return long(xp_gain * xp_mult)
	

def create_break_free_potion( attachee ):
	## creates a potion of breakfree (the workaround for making AI attempt break free)
	## ALSO, THIS IS USED FOR BREAK FREE
	while(attachee.item_find(8903) != OBJ_HANDLE_NULL):
		attachee.item_find(8903).destroy()
	#if (attachee.d20_query(Q_Is_BreakFree_Possible)): # workaround no longer necessary !
	#	create_item_in_inventory( 8903, attachee )
	##	attachee.d20_send_signal(S_BreakFree)
	
	
def Spiritual_Weapon_Begone( attachee ):
	SW_BS_TIME = 2 # [ms] time lapse for the spiritual weapon being away
	SW_BS_TIME_2 = 4 # a longer interval for the second ("failsafe") call
	SW_BS_TIME_3 = 250
	
	# This script is meant to prevent the Summoned Creature/ Spiritual Weapon exploit where you distract the AI with summoned creatures (in particular, the spiritual weapon, which has lots of HP and can take quite a few blows)
	# Method:
	# 1) changes the AI strategy to "target damaged"
	# 2) applies a 1000HP damage + 1000HP boost to the intended target (closest controllable conscious party member)
	#      this causes the AI to consider the PC as being the most damaged, which is what "target damaged" goes for, while not changing the PC's remaining HP
	# 3) a timed event switches back the HP to normal state a split second later (the time event is called 3 times to bulletproof the script)
	# Notes:
	# -The scripting assumes no one will go over 1000 HP
	# -Since this involves switching out AI strategy, it could cause some problems with spell casters. 
	#  Ideally you should create duplicate strategies with the "target damaged" command instead of target closest where appropriate.
	#  For now the script manually checks for some common strategies, which should cover about 90% of critters in the game (most of them have the Default (0) strategy anyway)
	
	# Need to add two cases:
	# Ignore warded only
	# Ignore SW + warded only
	# As usual
	if (game.global_vars[451] & 2**3 != 0) or (game.global_vars[451] & 2**4 != 0): # if config set to ignore Spiritual Weapons, jones is selected from non-SW list (using the established protos! see utilities.py)
		e_s_w = 1
	else:
		e_s_w = 0

	if (game.global_vars[451] & 2**3 != 0) and ( attachee.stat_level_get(stat_intelligence) >= 3 ): # if set to ignore summons, jones is selected from non-summons (mode select 1)
		jones_mode_select = 1
	else:
		jones_mode_select = 2

	if ( attachee.stat_level_get(stat_intelligence) >= 3 ): # ignore warded party members if INT >= 3
		jones_warded_exclude = 1
	else:
		jones_warded_exclude = 0

	desired_target_for_attack = party_closest( attachee, 1, mode_select = jones_mode_select, exclude_warded = jones_warded_exclude, exclude_charmed = 1, exclude_spiritual_weapon = e_s_w) # The desired selection
	expected_target_for_attack = party_closest( attachee, 1, mode_select = 2, exclude_warded = 0, exclude_charmed = 1, exclude_spiritual_weapon = 0)
	
	# ORC_CAVE_TRIBES_NON_GRUSHK = [8201, 8202] # NB: range() does not include the last number i.e. 14936 and 14946 are excluded
	# ORC_CAVE_GRUSHKITES = [8200] #includes Grush'k himself (but not his out of cave self)
	# if attachee.name in ORC_CAVE_GRUSHKITES: # Grush'kite
		# closest_orc = target_npc( attachee, conscious_only = 1, mode_select = 0, exclude_warded = 1, exclude_charmed = 1, exclude_spiritual_weapon = 1, can_see = 0, name_list = ORC_CAVE_TRIBES_NON_GRUSHK )
		# if game.global_vars[462] == 0:
			 # # the orcs attack you in unison; no action necessary
			# dummy = 1
		# elif game.global_vars[462] == 1: # Orcs and Grushkites also fighting between themselves
			# if attachee.distance_to(closest_orc) < attachee.distance_to(desired_target_for_attack):
				# desired_target_for_attack = closest_orc
	# elif attachee.name in ORC_CAVE_TRIBES_NON_GRUSHK: 
		# closest_orc = target_npc( attachee, conscious_only = 1, mode_select = 0, exclude_warded = 1, exclude_charmed = 1, exclude_spiritual_weapon = 1, can_see = 0, name_list = ORC_CAVE_GRUSHKITES )
		# if game.global_vars[462] == 0:
			# dummy = 1 # leave as is - the orcs attack you
		# elif game.global_vars[462] == 1: # Orcs and Grushkites also fighting between themselves
			# if attachee.distance_to(closest_orc) < attachee.distance_to(desired_target_for_attack):
				# desired_target_for_attack = closest_orc
	#print str(desired_target_for_attack)
	#print str(desired_target_for_attack_2)
	
	
	if expected_target_for_attack != desired_target_for_attack: # means an undesirable is closest
		desired_target_for_attack.stat_base_set(stat_hp_max, desired_target_for_attack.stat_base_get(stat_hp_max) + 1000 )
		desired_target_for_attack.obj_set_int(obj_f_hp_damage, desired_target_for_attack.obj_get_int(obj_f_hp_damage) + 1000 )

		orig_strat = attachee.obj_get_int(obj_f_critter_strategy)
		new_strat = -1
		if orig_strat in [0, 18, 19, 99, 163, 273]: # default/reach/giant/longspear/Juggernaut
			new_strat = 215
		elif orig_strat in [128, 130]: #Flanker
			new_strat = 216
		elif orig_strat == 133: # barbarian (tumbler no ranged)
			new_strat = 217
		elif orig_strat in [116, 131, 132]: # berzerker, gnolls
			new_strat = 218
		elif orig_strat == 102: # sniper
			new_strat = 219
		elif orig_strat == 83: # Medusa
			new_strat = 221
		elif orig_strat == 140: # Raider Magic User
			new_strat = 141
		elif orig_strat == 152: # Raider Cleric
			new_strat = 154
		elif orig_strat == 91: # Flay - spell oriented
			new_strat = 93
		if new_strat != -1:
			attachee.obj_set_int(obj_f_critter_strategy, new_strat)

		game.timevent_add( timed_restore_state, ( attachee, desired_target_for_attack, orig_strat ), SW_BS_TIME, 1 )
		game.timevent_add( timed_restore_state, ( attachee, desired_target_for_attack, orig_strat ), SW_BS_TIME_2, 1 )
		game.timevent_add( timed_restore_state, ( attachee, desired_target_for_attack, orig_strat ), SW_BS_TIME_3, 1 )
		
	
	# Below script worked, but crashed the game for some people...
		
	# this temporarily moves a spiritual weapon at the beginning of the round so it doesn't get targeted by an NPC
	# because, NPCs tend to focus on them due to them being closest
	# the SW is returned to its place a split second later, so the player doesn't notice (unless he's really squinting)
	# Drawback - seems like it deprives SWs of AoOs; but that seems like a reasonable sacrifice in exchange for preventing a huge exploit
	#(maybe summoned monsters in the future too?)
	#for obj in game.obj_list_vicinity(attachee.location, OLC_NPC):
	#	if obj.name in [14370, 14604, 14621, 14629] and obj.is_friendly(attachee) == 0 and attachee.distance_to(obj) <= attachee.distance_to( party_closest( attachee ) ):
	#
	#		tizinabi = location_from_axis(999,999)
	#		prev_loc = obj.location
	#		prev_x_off = obj.obj_get_int(obj_f_offset_x)
	#		prev_y_off = obj.obj_get_int(obj_f_offset_y)


	## Dustbin - do not use
			#obj.object_flag_set(OF_OFF) # was causing PCs to lose actions - do not use
			#attachee.ai_shitlist_add(obj)

	# Uncoment block to re-enable	
			#obj.move(tizinabi, 0, 0)
			#obj.condition_add_with_args( "unconscious", 0, 0)
			#obj.obj_set_int(obj_f_critter_subdual_damage, 1000)

			#game.timevent_add( move_sw_back, ( obj, prev_loc, prev_x_off, prev_y_off , SW_BS_TIME), SW_BS_TIME, 1 )
			#game.timevent_add( move_sw_back, ( obj, prev_loc, prev_x_off, prev_y_off , SW_BS_TIME_2), SW_BS_TIME_2, 1 ) # 2nd time - to bulletproof it
			
	# end of block
	
	
	
	
			#game.timevent_add( move_sw_back, ( obj, prev_loc, prev_x_off, prev_y_off , SW_BS_TIME_3), SW_BS_TIME_3, 1 )		
	return		
			
	
def move_sw_back( obj, prev_loc, prev_x_off, prev_y_off, timeee = -1):

	#obj.object_flag_unset(OF_OFF)

	#obj.obj_set_int(obj_f_critter_subdual_damage, 0)
	
	if obj.location != prev_loc:
		obj.move(prev_loc, 0, 0 )
		obj.obj_set_int( obj_f_offset_x, prev_x_off )
		obj.obj_set_int( obj_f_offset_y, prev_y_off )
		#f = open('filename' + str(timeee) +'.txt' , 'w')
		#f.write(str( prev_x_off) + ' ' + str(prev_y_off) + ' ' + str(prev_loc) )
		#f.close()
		
def timed_restore_state( attachee, desired_target_for_attack, orig_strat ):
	if desired_target_for_attack.obj_get_int(obj_f_hp_damage) >= 1000:
		desired_target_for_attack.obj_set_int(obj_f_hp_damage, desired_target_for_attack.obj_get_int(obj_f_hp_damage) - 1000 )
		desired_target_for_attack.stat_base_set(stat_hp_max, desired_target_for_attack.stat_base_get(stat_hp_max) - 1000 )

		attachee.obj_set_int(obj_f_critter_strategy, orig_strat)
