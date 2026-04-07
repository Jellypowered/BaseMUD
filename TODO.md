# BaseMUD TODO

## do_search (TAKA / GhostMud snippet)

The `do_search` function from TAKA's GhostMud codebase could not be integrated because
the following prerequisites are absent from BaseMUD:

1. **`ITEM_HIDDEN` flag** — not defined in `src/flags.h` or `src/flags.c`.
   This object extra-flag is needed to mark items that are hidden in a room and
   only revealed by a successful search.

2. **`gsn_search` skill** — no entry in `json/config/skills.json` and no `gsn_search`
   declaration in `src/skills.h` / `src/skills.c`.
   The skill governs whether the search attempt succeeds and provides the
   `check_improve` hook for skill advancement.

### What needs to be done before do_search can be implemented

- Add `ITEM_HIDDEN` to `src/flags.h` (ITEM_* block) and register its name in
  `src/flags.c` (the `item_extra_flags` table).
- Add a `search` skill entry in `json/config/skills.json` with appropriate
  `"type": "skill"` / `"min_position"` / `"classes"` fields.
- Declare `gsn_search` in `src/skills.h` and define it in `src/skills.c`.
- Implement `do_search` in `src/act_skills.c` following the TAKA pattern:
  - NPC guard
  - Skill check via `ch->pcdata->learned[gsn_search]`
  - Loop over `ch->in_room->content_first` / `obj->content_next`
  - Strip `ITEM_HIDDEN` from matching objects
  - `check_improve(ch, gsn_search, TRUE/FALSE, 4)` and `WAIT_STATE(ch, 24)`
- Register `do_search` in `src/interp.c`.
- Add a JSON help entry in `json/help/`.

### Example of do_search
void do_search( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;

	if(IS_NPC(ch))
		return;

	if(ch->pcdata->learned[gsn_search] < 1)
	{
		send_to_char("{GYou search around making lots of noise.{x\n\r",ch);
		return;
	}

	if(number_percent() < ch->pcdata->learned[gsn_search] )
	{
		check_improve(ch,gsn_search,TRUE,4);
		send_to_char("{GYou search the room..{x\n\r",ch);
		for(obj = ch->in_room->contents; obj != NULL; obj = obj->next_content)
		{
			if(IS_SET(obj->extra_flags,ITEM_HIDDEN) )
			{
				printf_to_char(ch,"{GYou reveal {W%s{x\n\r",obj->short_descr);
				REMOVE_BIT(obj->extra_flags,ITEM_HIDDEN);
			}
		}
		send_to_char("{GYou have searched everywhere.{x\n\r",ch);
		WAIT_STATE(ch,24);
	}
	else
		send_to_char("{GYou didn't uncover anything unusual.{x\n\r",ch);

	check_improve(ch,gsn_search,FALSE,4);
	WAIT_STATE(ch,24);

	return;
}