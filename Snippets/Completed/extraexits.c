From: gmaerki@prodigy.net

Extra Exits  ROM 2.4 (ne nw se sw)

        I am making this letter to help al the newbie coders.   First
off I would like to Thank Erwin and Ao for helping
me figure this out.   I have OLC 1.6 (mobprograms) and Lope's Colour on
my mud.... So not all the files will apply...

List of all the files that need changed
act_info
act_move
act_wiz
comm
db
fight
merc.h *
mob_cmds
olc_act
interp.c
interp.h
olc.c
olc.h
scan
update

I found out the places I need to change by looking for DOWN and adding
the extra exits to it. An example of what
you should be looking for is below....
         if ( !str_cmp( arg1, "n" ) || !str_cmp( arg1, "north" ) ) door
= 0;
    else if ( !str_cmp( arg1, "e" ) || !str_cmp( arg1, "east"  ) ) door
= 1;
    else if ( !str_cmp( arg1, "s" ) || !str_cmp( arg1, "south" ) ) door
= 2;
    else if ( !str_cmp( arg1, "w" ) || !str_cmp( arg1, "west"  ) ) door
= 3;
    else if ( !str_cmp( arg1, "u" ) || !str_cmp( arg1, "up"    ) ) door
= 4;
    else if ( !str_cmp( arg1, "d" ) || !str_cmp( arg1, "down"  ) ) door
= 5;
this is what u should add !!!!!!!!
    else if ( !str_cmp( arg1, "ne" ) || !str_cmp( arg1, "northeast"  ) )
door = 6;
    else if ( !str_cmp( arg1, "nw" ) || !str_cmp( arg1, "northwest"  ) )
door = 7;
    else if ( !str_cmp( arg1, "se" ) || !str_cmp( arg1, "southeast"  ) )
door = 8;
    else if ( !str_cmp( arg1, "sw" ) || !str_cmp( arg1, "southwest"  ) )
door = 9;

Also look for DOOR  change all the 5's to 9 and 6's to 10.... Look below
for help
              ( door = 0; door <= 5; door++ )

should be     ( door = 0; door <= 9; door++ )


IMPORTANT NOTE  !!!!!!!!!! in act_move make sure u do the rev dir.....
                           also in merc.h look for MAX_DIR and EXIT_DATA


I hope this file helps some lost souls out there.  If u have any
question the 
ROM mailing list is a good place to ask them..... or e mail me
gmaerki@prodigy.net

    
                             Happy Coding   
			
			Carnage
