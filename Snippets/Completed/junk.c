/* Code by franzj@email.uc.edu */
void do_junk(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;

    if ( ( obj = get_obj_carry( ch, argument, ch ) ) == NULL )
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
        send_to_char( "You can't let go of it.\n\r", ch );
        return;
    }

    obj_from_char( obj );
    act( "$n junks $p.", ch, obj, NULL, TO_ROOM );
    act( "You junk $p.", ch, obj, NULL, TO_CHAR );
        extract_obj(obj);
}
