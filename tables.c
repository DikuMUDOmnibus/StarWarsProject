/***************************************************************************
*                           STAR WARS REALITY 1.0                          *
*--------------------------------------------------------------------------*
* Star Wars Reality Code Additions and changes from the Smaug Code         *
* copyright (c) 1997 by Sean Cooper                                        *
* -------------------------------------------------------------------------*
* Starwars and Starwars Names copyright(c) Lucas Film Ltd.                 *
*--------------------------------------------------------------------------*
* SMAUG 1.0 (C) 1994, 1995, 1996 by Derek Snider                           *
* SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,                    *
* Scryn, Rennard, Swordbearer, Gorog, Grishnakh and Tricops                *
* ------------------------------------------------------------------------ *
* Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
* Chastain, Michael Quan, and Mitchell Tse.                                *
* Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
* Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
* ------------------------------------------------------------------------ *
* 			Table load/save Module				   *
****************************************************************************/

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include "mud.h"

/* global variables */
int top_sn;
int top_herb;

SKILLTYPE *skill_table[MAX_SKILL];
SKILLTYPE *herb_table[MAX_HERB];

const char *const skill_tname[MAX_SKILLTYPE] = { "unknown", "Spell", "Skill", "Weapon", "Tongue", "Herb", "Passive", "Unset" };

const char *const style_type[STYLE_MAX] = { "Healing", "Damage", "Buff", "Enfeeble", "Redirect", "Cleanse", "Summon", "Polymorph", "Unset" };

const char *const factor_names[MAX_FACTOR] = { "apply_factor", "stat_factor", "base_roll_factor" };

const char *const skilltype_names[MAX_TYPE] = { "skill_type", "style_type", "cost_type", "damtype_type", "target_type" };

const char *const cost_type[MAX_COST] = { "hp", "mana", "move" };

const char *const applytypes_type[MAX_APPLYTYPE] = { "join_self", "join_target", "override_self", "override_target", "no_apply" };

SPELL_FUN *spell_function( const char *name )
{
  SPELL_FUN *funHandle = 0;
  const char *error = 0;
  *(void**)( &funHandle ) = dlsym( sysdata.dlHandle, name );

  if( ( error = dlerror() ) != NULL )
    {
      bug( "Error locating %s in symbol table. %s", name, error );
      return spell_notfound;
    }

  return funHandle;
}

DO_FUN *skill_function( const char *name )
{
  const char *error = 0;
  DO_FUN *funHandle = 0;
  *(void**)( &funHandle ) = dlsym( sysdata.dlHandle, name );

  if( ( error = dlerror() ) != NULL )
    {
      bug( "Error locating %s in symbol table. %s", name, error );
      return skill_notfound;
    }

  return funHandle;
}

/*
 * Function used by qsort to sort skills
 */
int skill_comp( SKILLTYPE ** sk1, SKILLTYPE ** sk2 )
{
   SKILLTYPE *skill1 = ( *sk1 );
   SKILLTYPE *skill2 = ( *sk2 );

   if( !skill1 && skill2 )
      return 1;
   if( skill1 && !skill2 )
      return -1;
   if( !skill1 && !skill2 )
      return 0;
   if( skill1->type < skill2->type )
      return -1;
   if( skill1->type > skill2->type )
      return 1;
   return strcasecmp( skill1->name, skill2->name );
}

/*
 * Sort the skill table with qsort
 */
void sort_skill_table(  )
{
   log_string( "Sorting skill table..." );
   qsort( &skill_table[1], top_sn - 1, sizeof( SKILLTYPE * ), ( int ( * )( const void *, const void * ) )skill_comp );
}

void sort_player_skill_table(  CHAR_DATA *ch )
{
   if( IS_NPC( ch ) )
      return;

   qsort( &ch->pc_skills[1], ch->top_sn -1, sizeof( SKILLTYPE * ), ( int( * )(const void *, const void * ) )skill_comp );
   return;
}

/*
 * Write skill data to a file
 */
void fwrite_skill( FILE * fpout, SKILLTYPE * skill )
{
   FACTOR_DATA *factor;
   AFFECT_DATA *aff;
   STAT_BOOST *stat_boost;

   fprintf( fpout, "Name         %s~\n", skill->name );
   fprintf( fpout, "Type         %s\n", skill_tname[skill->type] );
   fprintf( fpout, "Flags        %d\n", skill->flags );
   if( skill->target )
      fprintf( fpout, "Target       %d\n", skill->target );
   fprintf( fpout, "Style        %d\n", skill->style );
   if( skill->minimum_position )
      fprintf( fpout, "Minpos       %d\n", skill->minimum_position );
   if( skill->saves )
      fprintf( fpout, "Saves        %d\n", skill->saves );
   if( skill->slot )
      fprintf( fpout, "Slot         %d\n", skill->slot );
   if( skill->min_hp )
      fprintf( fpout, "HP           %d\n", skill->min_hp );
   if( skill->min_mana )
      fprintf( fpout, "Mana         %d\n", skill->min_mana );
   if( skill->min_move )
      fprintf( fpout, "Move         %d\n", skill->min_move );
   fprintf( fpout, "Cost         %s\n", print_bitvector( &skill->cost ) );
   if( skill->beats )
      fprintf( fpout, "Rounds       %d\n", skill->beats );
   if( skill->charge )
      fprintf( fpout, "Charge       %f\n", skill->charge );
   if( skill->guild != -1 )
      fprintf( fpout, "Guild        %d\n", skill->guild );
   fprintf( fpout, "Dammsg       %s~\n", skill->noun_damage );
   if( skill->msg_off && skill->msg_off[0] != '\0' )
      fprintf( fpout, "Wearoff      %s~\n", skill->msg_off );

   if( skill->hit_char && skill->hit_char[0] != '\0' )
      fprintf( fpout, "Hitchar      %s~\n", skill->hit_char );
   if( skill->hit_vict && skill->hit_vict[0] != '\0' )
      fprintf( fpout, "Hitvict      %s~\n", skill->hit_vict );
   if( skill->hit_room && skill->hit_room[0] != '\0' )
      fprintf( fpout, "Hitroom      %s~\n", skill->hit_room );

   if( skill->miss_char && skill->miss_char[0] != '\0' )
      fprintf( fpout, "Misschar     %s~\n", skill->miss_char );
   if( skill->miss_vict && skill->miss_vict[0] != '\0' )
      fprintf( fpout, "Missvict     %s~\n", skill->miss_vict );
   if( skill->miss_room && skill->miss_room[0] != '\0' )
      fprintf( fpout, "Missroom     %s~\n", skill->miss_room );

   if( skill->die_char && skill->die_char[0] != '\0' )
      fprintf( fpout, "Diechar      %s~\n", skill->die_char );
   if( skill->die_vict && skill->die_vict[0] != '\0' )
      fprintf( fpout, "Dievict      %s~\n", skill->die_vict );
   if( skill->die_room && skill->die_room[0] != '\0' )
      fprintf( fpout, "Dieroom      %s~\n", skill->die_room );

   if( skill->imm_char && skill->imm_char[0] != '\0' )
      fprintf( fpout, "Immchar      %s~\n", skill->imm_char );
   if( skill->imm_vict && skill->imm_vict[0] != '\0' )
      fprintf( fpout, "Immvict      %s~\n", skill->imm_vict );
   if( skill->imm_room && skill->imm_room[0] != '\0' )
      fprintf( fpout, "Immroom      %s~\n", skill->imm_room );

   if( skill->dice && skill->dice[0] != '\0' )
      fprintf( fpout, "Dice         %s~\n", skill->dice );
   if( skill->value )
      fprintf( fpout, "Value        %d\n", skill->value );
   if( skill->threat )
      fprintf( fpout, "Threat       %d\n", skill->threat );
   if( skill->cooldown )
      fprintf( fpout, "Cooldown     %d\n", skill->cooldown );
   if( skill->difficulty )
      fprintf( fpout, "Difficulty   %d\n", skill->difficulty );
   if( skill->participants )
      fprintf( fpout, "Participants %d\n", skill->participants );
   if( skill->components && skill->components[0] != '\0' )
      fprintf( fpout, "Components   %s~\n", skill->components );
   if( skill->teachers && skill->teachers[0] != '\0' )
      fprintf( fpout, "Teachers     %s~\n", skill->teachers );
   for( aff = skill->first_affect; aff; aff = aff->next )
      fwrite_fuss_affect( fpout, aff );
   if( skill->alignment )
      fprintf( fpout, "Alignment   %d\n", skill->alignment );

   fprintf( fpout, "DamageDetails   %f\n", skill->base_roll_boost );

   for( stat_boost = skill->first_statboost; stat_boost; stat_boost = stat_boost->next )
      fprintf( fpout, "StatBoost  %d %d %f\n", stat_boost->from_id, stat_boost->location, stat_boost->modifier );

   if( !xIS_EMPTY( skill->damtype ) )
      fprintf( fpout, "Damtype         %s\n", print_bitvector( &skill->damtype ) );
   if( skill->type != SKILL_HERB )
   {
      fprintf( fpout, "Minlevel     %d\n", skill->min_level );
   }
   for( factor = skill->first_factor; factor; factor = factor->next )
      fprintf( fpout, "FactorID     %d\n", factor->id );
   fprintf( fpout, "End\n\n" );
}

/*
 * Save the skill table to disk
 */
void save_skill_table(  )
{
   int x;
   FILE *fpout;

   if( ( fpout = fopen( SKILL_FILE, "w" ) ) == NULL )
   {
      bug( "Cannot open skills.dat for writting", 0 );
      perror( SKILL_FILE );
      return;
   }

   for( x = 0; x < top_sn; x++ )
   {
      if( !skill_table[x]->name || skill_table[x]->name[0] == '\0' )
         break;
      fprintf( fpout, "#SKILL\n" );
      fwrite_skill( fpout, skill_table[x] );
   }
   fprintf( fpout, "#END\n" );
   fclose( fpout );
}

/*
 * Save the herb table to disk
 */
void save_herb_table(  )
{
   int x;
   FILE *fpout;

   if( ( fpout = fopen( HERB_FILE, "w" ) ) == NULL )
   {
      bug( "Cannot open herbs.dat for writting", 0 );
      perror( HERB_FILE );
      return;
   }

   for( x = 0; x < top_herb; x++ )
   {
      if( !herb_table[x]->name || herb_table[x]->name[0] == '\0' )
         break;
      fprintf( fpout, "#HERB\n" );
      fwrite_skill( fpout, herb_table[x] );
   }
   fprintf( fpout, "#END\n" );
   fclose( fpout );
}

/*
 * Save the socials to disk
 */
void save_socials(  )
{
   FILE *fpout;
   SOCIALTYPE *social;
   int x;

   if( ( fpout = fopen( SOCIAL_FILE, "w" ) ) == NULL )
   {
      bug( "Cannot open socials.dat for writting", 0 );
      perror( SOCIAL_FILE );
      return;
   }

   for( x = 0; x < 27; x++ )
   {
      for( social = social_index[x]; social; social = social->next )
      {
         if( !social->name || social->name[0] == '\0' )
         {
            bug( "Save_socials: blank social in hash bucket %d", x );
            continue;
         }
         fprintf( fpout, "#SOCIAL\n" );
         fprintf( fpout, "Name        %s~\n", social->name );
         if( social->char_no_arg )
            fprintf( fpout, "CharNoArg   %s~\n", social->char_no_arg );
         else
            bug( "Save_socials: NULL char_no_arg in hash bucket %d", x );
         if( social->others_no_arg )
            fprintf( fpout, "OthersNoArg %s~\n", social->others_no_arg );
         if( social->char_found )
            fprintf( fpout, "CharFound   %s~\n", social->char_found );
         if( social->others_found )
            fprintf( fpout, "OthersFound %s~\n", social->others_found );
         if( social->vict_found )
            fprintf( fpout, "VictFound   %s~\n", social->vict_found );
         if( social->char_auto )
            fprintf( fpout, "CharAuto    %s~\n", social->char_auto );
         if( social->others_auto )
            fprintf( fpout, "OthersAuto  %s~\n", social->others_auto );
         fprintf( fpout, "End\n\n" );
      }
   }
   fprintf( fpout, "#END\n" );
   fclose( fpout );
}

int get_skill( const char *skilltype )
{
   int x;

   for( x = 0; x < MAX_SKILLTYPE; x++ )
      if( !str_cmp( skilltype, skill_tname[x] ) )
         return x;
   return 0;
}

/*
 * Save the commands to disk
 */
void save_commands(  )
{
   FILE *fpout;
   CMDTYPE *command;
   int x;

   if( ( fpout = fopen( COMMAND_FILE, "w" ) ) == NULL )
   {
      bug( "Cannot open commands.dat for writing", 0 );
      perror( COMMAND_FILE );
      return;
   }

   for( x = 0; x < 126; x++ )
   {
      for( command = command_hash[x]; command; command = command->next )
      {
         if( !command->name || command->name[0] == '\0' )
         {
            bug( "Save_commands: blank command in hash bucket %d", x );
            continue;
         }
         fprintf( fpout, "#COMMAND\n" );
         fprintf( fpout, "Name        %s~\n", command->name );
         fprintf( fpout, "Code        %s\n", command->fun_name?command->fun_name:"" ); // Modded to use new field - Trax
         fprintf( fpout, "Position    %d\n", command->position );
         fprintf( fpout, "Level       %d\n", command->level );
         fprintf( fpout, "Log         %d\n", command->log );
         fprintf( fpout, "End\n\n" );
      }
   }
   fprintf( fpout, "#END\n" );
   fclose( fpout );
}

SKILLTYPE *fread_skill( FILE * fp, bool Player )
{
   char buf[MAX_STRING_LENGTH];
   const char *word;
   bool fMatch;
   SKILLTYPE *skill;

   CREATE( skill, SKILLTYPE, 1 );

   skill->guild = -1;
   skill->style = -1;

   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'A':
            KEY( "Alignment", skill->alignment, fread_number( fp ) );
            if( !str_cmp( word, "Affect" ) || !str_cmp( word, "AffectData" ) )
            {
               AFFECT_DATA *aff = fread_fuss_affect( fp, word );

               if( aff )
                  LINK( aff, skill->first_affect, skill->last_affect, next, prev );
               fMatch = TRUE;
               break;
            }
            break;

         case 'C':
            KEY( "Charge", skill->charge, fread_float( fp ) );
            if ( !str_cmp( word, "Code" ) )
            {
               if( Player )
                  break;
               SPELL_FUN *spellfun;
               DO_FUN *dofun;
               const char *w = fread_word( fp );

               fMatch = TRUE;
               if( !str_cmp( word, "(null)" ) )
                  break;
               else if( !str_prefix( "do_", w ) && ( dofun = skill_function(w) ) != skill_notfound )
               {
                  skill->skill_fun = dofun;
                  skill->spell_fun = NULL;
                  skill->skill_fun_name = str_dup(w);
               }
               else if( str_prefix( "do_", w ) && ( spellfun = spell_function(w) ) != spell_notfound )
               {
                  skill->spell_fun = spellfun;
                  skill->skill_fun = NULL;
                  skill->spell_fun_name = str_dup(w);
               }
               else
               {
                  bug( "%s: unknown skill/spell %s", __FUNCTION__, w );
                  skill->spell_fun = spell_null;
               }
               break;
            }
            KEY( "Components", skill->components, fread_string( fp ) );
            KEY( "Cooldown", skill->cooldown, fread_number( fp ) );
            KEY( "Cost", skill->cost, fread_bitvector( fp ) );
            break;

         case 'D':
            KEY( "DamageDetails", skill->base_roll_boost, fread_float( fp ) );
            KEY( "Dammsg", skill->noun_damage, fread_string( fp ) );
            KEY( "Damtype", skill->damtype, fread_bitvector( fp ) );
            KEY( "Dice", skill->dice, fread_string( fp ) );
            KEY( "Diechar", skill->die_char, fread_string( fp ) );
            KEY( "Dieroom", skill->die_room, fread_string( fp ) );
            KEY( "Dievict", skill->die_vict, fread_string( fp ) );
            KEY( "Difficulty", skill->difficulty, fread_number( fp ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
               return skill;
            break;

         case 'F':
            if( !str_cmp( word, "FactorID" ) )
            {
               FACTOR_DATA *factor;
               fMatch = TRUE;
               if( ( factor = copy_factor( get_factor_from_id( fread_number( fp ) ) ) ) == NULL )
                  break;
               LINK( factor, skill->first_factor, skill->last_factor, next, prev );
               break;
            }
            KEY( "Flags", skill->flags, fread_number( fp ) );
            break;

         case 'G':
            KEY( "Guild", skill->guild, fread_number( fp ) );
            break;

         case 'H':
            KEY( "Hitchar", skill->hit_char, fread_string( fp ) );
            KEY( "Hitroom", skill->hit_room, fread_string( fp ) );
            KEY( "Hitvict", skill->hit_vict, fread_string( fp ) );
            KEY( "HP", skill->min_hp, fread_number( fp ) );
            break;

         case 'I':
            KEY( "Immchar", skill->imm_char, fread_string( fp ) );
            KEY( "Immroom", skill->imm_room, fread_string( fp ) );
            KEY( "Immvict", skill->imm_vict, fread_string( fp ) );
            break;

         case 'M':
            KEY( "Mana", skill->min_mana, fread_number( fp ) );
            KEY( "Minlevel", skill->min_level, fread_number( fp ) );
            KEY( "Minpos", skill->minimum_position, fread_number( fp ) );
            KEY( "Misschar", skill->miss_char, fread_string( fp ) );
            KEY( "Missroom", skill->miss_room, fread_string( fp ) );
            KEY( "Missvict", skill->miss_vict, fread_string( fp ) );
            KEY( "Move", skill->min_move, fread_number( fp ) );
            break;

         case 'N':
            KEY( "Name", skill->name, fread_string( fp ) );
            break;

         case 'P':
            KEY( "Participants", skill->participants, fread_number( fp ) );
            break;

         case 'R':
            KEY( "Rounds", skill->beats, fread_number( fp ) );
            break;

         case 'S':
            KEY( "Saves", skill->saves, fread_number( fp ) );
            KEY( "Slot", skill->slot, fread_number( fp ) );
            if( !str_cmp( word, "StatBoost" ) )
            {
               STAT_BOOST *stat_boost;

               fMatch = TRUE;

               CREATE( stat_boost, STAT_BOOST, 1 );
               stat_boost->from_id = fread_number( fp );
               stat_boost->location = fread_number( fp );
               stat_boost->modifier = fread_float( fp );
               LINK( stat_boost, skill->first_statboost, skill->last_statboost, next, prev );
            }
            KEY( "Style", skill->style, fread_number( fp ) );
            break;

         case 'T':
            KEY( "Target", skill->target, fread_number( fp ) );
            KEY( "Teachers", skill->teachers, fread_string( fp ) );
            KEY( "Threat", skill->threat, fread_number( fp ) );
            KEY( "Type", skill->type, get_skill( fread_word( fp ) ) );
            break;

         case 'V':
            KEY( "Value", skill->value, fread_number( fp ) );
            break;

         case 'W':
            KEY( "Wearoff", skill->msg_off, fread_string( fp ) );
            break;
      }

      if( !fMatch )
      {
         sprintf( buf, "Fread_skill: no match: %s", word );
         bug( buf, 0 );
      }
   }
}

void load_disciplines(  )
{
   FILE *fp;

   if( ( fp = fopen( DISCIPLINE_FILE, "r" ) ) != NULL )
   {
      for( ;; )
      {
         char letter;
         const char *word;

         letter = fread_letter( fp );
         if( letter == '*' )
         {
            fread_to_eol( fp );
            continue;
         }

         if( letter != '#' )
         {
            bug( "Load_disciplines: # not found.", 0 );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "DISCIPLINE" ) )
         {
            DISC_DATA *discipline;
            if( ( discipline = fread_discipline( fp ) ) == NULL )
            {
               bug( "Huge error loading disciplines.", 0 );
               fclose( fp );
               return;
            }
            LINK( discipline, first_discipline, last_discipline, next, prev );
            continue;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "Load_disciplines: bad section.", 0 );
            continue;
         }
      }
      fclose( fp );
   }
   else
   {
      bug( "Cannot open discipline.dat", 0 );
      exit( 0 );
   }
   return;
}

DISC_DATA *fread_discipline( FILE * fp )
{
   char buf[MAX_STRING_LENGTH];
   DISC_DATA *disc;
   bool fMatch;
   const char *word;

   CREATE( disc, DISC_DATA, 1 );

   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;
         case '#':
            if( !str_cmp( word, "#Factor" ) )
            {
               fMatch = TRUE;
               FACTOR_DATA *factor;
               CREATE( factor, FACTOR_DATA, 1 );
               factor->id = fread_number( fp );
               factor->factor_type = fread_number( fp );
               factor->location = fread_number( fp );
               factor->affect = fread_bitvector( fp );
               factor->modifier = fread_float( fp );
               factor->apply_type = fread_number( fp );
               factor->duration = fread_float( fp );
               factor->owner = disc;
               LINK( factor, disc->first_factor, disc->last_factor, next, prev );
               break;
            }
            break;
         case 'C':
            KEY( "Cost", disc->cost, fread_bitvector( fp ) );
            break;
         case 'D':
            KEY( "Damtype", disc->damtype, fread_bitvector( fp ) );
            break;
         case 'E':
            if( !str_cmp( word, "End" ) )
               return disc;
         case 'G':
            if( !str_cmp( word, "Gains" ) )
            {
               fMatch = TRUE;
               disc->hit_gain = fread_number( fp );
               disc->move_gain = fread_number( fp );
               disc->mana_gain = fread_number( fp );
               break;
            }
            break;
         case 'I':
            KEY( "ID", disc->id, fread_number( fp ) );
         case 'M':
            KEY( "MinLevel", disc->min_level, fread_number( fp ) );
            break;

         case 'N':
            KEY( "Name", disc->name, fread_string( fp ) );
            break;
         case 'S':
            KEY( "SkillStyle", disc->skill_style, fread_bitvector( fp ) );
            KEY( "SkillType", disc->skill_type, fread_bitvector( fp ) );
            break;
         case 'T':
            KEY( "TargetType", disc->target_type, fread_bitvector( fp ) );
            break;
      }
      if( !fMatch )
      {
         sprintf( buf, "Fread_discipline: no match: %s", word );
         bug( buf, 0 );
      }
   }
}

void save_disciplines(  )
{
   FILE *fpout;
   DISC_DATA *discipline;

   if( ( fpout = fopen( DISCIPLINE_FILE, "w" ) ) == NULL )
   {
      bug( "Cannot open disciplines.dat for writting", 0 );
      perror( DISCIPLINE_FILE );
      return;
   }

   for( discipline = first_discipline; discipline; discipline = discipline->next )
   {
      if( !discipline->name || discipline->name[0] == '\0' )
         break;
      fprintf( fpout, "#DISCIPLINE\n" );
      fwrite_discipline( fpout, discipline );
   }
   fprintf( fpout, "#END\n" );
   fclose( fpout );
   return;
}

void fwrite_discipline( FILE *fpout, DISC_DATA *discipline )
{
   FACTOR_DATA *factor;
   AFFECT_DATA *aff;

   fprintf( fpout, "ID         %d\n", discipline->id );
   fprintf( fpout, "Name       %s~\n", discipline->name );
   fprintf( fpout, "Gains      %d %d %d\n", discipline->hit_gain, discipline->move_gain, discipline->mana_gain );
   fprintf( fpout, "MinLevel   %d\n", discipline->min_level );
   fprintf( fpout, "Cost       %s\n", print_bitvector( &discipline->cost ) );
   fprintf( fpout, "SkillType  %s\n", print_bitvector( &discipline->skill_type ) );
   fprintf( fpout, "SkillStyle %s\n", print_bitvector( &discipline->skill_style ) );
   fprintf( fpout, "Damtype    %s\n", print_bitvector( &discipline->damtype ) );
   fprintf( fpout, "TargetType %s\n", print_bitvector( &discipline->target_type ) );

   for( factor = discipline->first_factor; factor; factor = factor->next )
   {
      fprintf( fpout, "#Factor %d %d %d %s %f %d %f\n", factor->id, factor->factor_type, factor->location, print_bitvector( &factor->affect ), factor->modifier, factor->apply_type, factor->duration );
   }

   for( aff = discipline->first_affect; aff; aff = aff->next )
      fwrite_fuss_affect( fpout, aff );

   fprintf( fpout, "\nEnd\n" );
}
void load_skill_table(  )
{
   FILE *fp;

   if( ( fp = fopen( SKILL_FILE, "r" ) ) != NULL )
   {
      top_sn = 0;
      for( ;; )
      {
         char letter;
         const char *word;

         letter = fread_letter( fp );
         if( letter == '*' )
         {
            fread_to_eol( fp );
            continue;
         }

         if( letter != '#' )
         {
            bug( "Load_skill_table: # not found.", 0 );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "SKILL" ) )
         {
            if( top_sn >= MAX_SKILL )
            {
               bug( "load_skill_table: more skills than MAX_SKILL %d", MAX_SKILL );
               fclose( fp );
               return;
            }
            skill_table[top_sn++] = fread_skill( fp, FALSE );
            continue;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "Load_skill_table: bad section.", 0 );
            continue;
         }
      }
      fclose( fp );
   }
   else
   {
      bug( "Cannot open skills.dat", 0 );
      exit( 0 );
   }
}

void load_herb_table(  )
{
   FILE *fp;

   if( ( fp = fopen( HERB_FILE, "r" ) ) != NULL )
   {
      top_herb = 0;
      for( ;; )
      {
         char letter;
         const char *word;

         letter = fread_letter( fp );
         if( letter == '*' )
         {
            fread_to_eol( fp );
            continue;
         }

         if( letter != '#' )
         {
            bug( "Load_herb_table: # not found.", 0 );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "HERB" ) )
         {
            if( top_herb >= MAX_HERB )
            {
               bug( "load_herb_table: more herbs than MAX_HERB %d", MAX_HERB );
               fclose( fp );
               return;
            }
            herb_table[top_herb++] = fread_skill( fp, FALSE );
            if( herb_table[top_herb - 1]->slot == 0 )
               herb_table[top_herb - 1]->slot = top_herb - 1;
            continue;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "Load_herb_table: bad section.", 0 );
            continue;
         }
      }
      fclose( fp );
   }
   else
   {
      bug( "Cannot open herbs.dat", 0 );
      exit( 0 );
   }
}

void fread_social( FILE * fp )
{
   char buf[MAX_STRING_LENGTH];
   const char *word;
   bool fMatch;
   SOCIALTYPE *social;

   CREATE( social, SOCIALTYPE, 1 );

   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'C':
            KEY( "CharNoArg", social->char_no_arg, fread_string_nohash( fp ) );
            KEY( "CharFound", social->char_found, fread_string_nohash( fp ) );
            KEY( "CharAuto", social->char_auto, fread_string_nohash( fp ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( !social->name )
               {
                  bug( "Fread_social: Name not found", 0 );
                  free_social( social );
                  return;
               }
               if( !social->char_no_arg )
               {
                  bug( "Fread_social: CharNoArg not found", 0 );
                  free_social( social );
                  return;
               }
               add_social( social );
               return;
            }
            break;

         case 'N':
            KEY( "Name", social->name, fread_string_nohash( fp ) );
            break;

         case 'O':
            KEY( "OthersNoArg", social->others_no_arg, fread_string_nohash( fp ) );
            KEY( "OthersFound", social->others_found, fread_string_nohash( fp ) );
            KEY( "OthersAuto", social->others_auto, fread_string_nohash( fp ) );
            break;

         case 'V':
            KEY( "VictFound", social->vict_found, fread_string_nohash( fp ) );
            break;
      }

      if( !fMatch )
      {
         sprintf( buf, "Fread_social: no match: %s", word );
         bug( buf, 0 );
      }
   }
}

void load_socials(  )
{
   FILE *fp;

   if( ( fp = fopen( SOCIAL_FILE, "r" ) ) != NULL )
   {
      for( ;; )
      {
         char letter;
         const char *word;

         letter = fread_letter( fp );
         if( letter == '*' )
         {
            fread_to_eol( fp );
            continue;
         }

         if( letter != '#' )
         {
            bug( "Load_socials: # not found.", 0 );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "SOCIAL" ) )
         {
            fread_social( fp );
            continue;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "Load_socials: bad section.", 0 );
            continue;
         }
      }
      fclose( fp );
   }
   else
   {
      bug( "Cannot open socials.dat", 0 );
      exit( 0 );
   }
}

void fread_command( FILE * fp )
{
   char buf[MAX_STRING_LENGTH];
   const char *word;
   bool fMatch;
   CMDTYPE *command;

   CREATE( command, CMDTYPE, 1 );

   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

	case 'C':
	    KEY( "Code",	command->fun_name, str_dup( fread_word( fp ) ) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if( !command->name )
		{
		   bug( "%s", "Fread_command: Name not found" );
		   free_command( command );
		   return;
		}
		if( !command->fun_name )
		{
		   bug( "fread_command: No function name supplied for %s", command->name );
		   free_command( command );
		   return;
		}
		/*
	 	 * Mods by Trax
		 * Fread in code into char* and try linkage here then
		 * deal in the "usual" way I suppose..
		 */
	      command->do_fun = skill_function( command->fun_name );
		if( command->do_fun == skill_notfound )
		{
		   bug( "Fread_command: Function %s not found for %s", command->fun_name, command->name );
		   free_command( command );
		   return;
		}
		add_command( command );
		return;
	    }
	    break;

         case 'L':
            KEY( "Level", command->level, fread_number( fp ) );
            KEY( "Log", command->log, fread_number( fp ) );
            break;

         case 'N':
            KEY( "Name", command->name, fread_string_nohash( fp ) );
            break;

         case 'P':
            KEY( "Position", command->position, fread_number( fp ) );
            break;
      }

      if( !fMatch )
      {
         sprintf( buf, "Fread_command: no match: %s", word );
         bug( buf, 0 );
      }
   }
}

void load_commands(  )
{
   FILE *fp;

   if( ( fp = fopen( COMMAND_FILE, "r" ) ) != NULL )
   {
      for( ;; )
      {
         char letter;
         const char *word;

         letter = fread_letter( fp );
         if( letter == '*' )
         {
            fread_to_eol( fp );
            continue;
         }

         if( letter != '#' )
         {
            bug( "Load_commands: # not found.", 0 );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "COMMAND" ) )
         {
            fread_command( fp );
            continue;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "Load_commands: bad section.", 0 );
            continue;
         }
      }
      fclose( fp );
   }
   else
   {
      bug( "Cannot open commands.dat", 0 );
      exit( 0 );
   }

}
