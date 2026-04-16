/*********************************************************************
 *  SWALK snippet, by Jorge Pereira( Evren ) <si22668@ci.uminho.pt>  *
 *  permission to use and alter this code is granted provided that   *
 *  you keep this header. I would also appreciate if you could mail  *
 *  me informing that you are using this code. :)                    *
 ****************************************************** 20/DEC/1998 **/
 // tested for Envy codebase //

/* check the syntax of the speewalk command */
bool check_speedwalk( char *s )
{
   bool found_char = FALSE;
   while ( *s )
   {
      if (   !strchr( "nsewudNSEWUD ", *s )
          && !isdigit( *s ) )
         return FALSE;

      if ( !isdigit( *s ) )
         found_char = TRUE;
      s++;
   }
   return found_char;
}


void do_swalk( CHAR_DATA *ch, char *argument )
{
   int            count;
   char         * s;            		// input string
   char           buf[MAX_INPUT_LENGTH];	// buffer
   char         * b;            		// buffer pointer
   char           tmp[NAX_INPUT_LENGTH];	// temporary buffer
   char         * t;           			// temporary buffer pointer

   DESCRIPTOR_DATA *d;

   if ( !( d = ch->desc ) )
      return;

   if ( *argument == '\0' )
   {
      send_to_char( "Syntax :  swalk <list of directions>\n\r"
                    "example:  swalk 2s3eu\n\r", ch );
      return;
   }

   if ( !check_speedwalk( argument ) )
   {
      send_to_char( "That is not a valid speedwalk.\n\r", ch );
      return;
   }

   s = argument;
   *( b = buf ) = '\0';
   while ( *s )
   {
      if ( *s == '\0' )
      {
         s++;
         continue;
      }
      *(t = tmp) = '\0';
      count = 1;
      while ( isdigit( *s ) )
      {
        *t = *s;
        *(++t) = '\0';
        s++;
      }
      if ( tmp[0] != '\0' )
         count = atoi( tmp );
      while ( --count >= 0 )
      {
         if ( b - buf >= MIL-5 )
         {
            send_to_char( "Speedwalk is too long.\n\r", ch );
            return;
         }
         *b = ';';
         *(++b)=*s;
         ++b;
      }
     s++;
   }
   *b = '\0';
   strcat( buf, ";" );

   send_to_char( "Speedwalking...\n\r", ch );

   strcpy( d->flusher, buf );
   *d->incomm = '\0';

   d->flush_point = strchr( d->flusher, ';' );

   if ( d->flush_point )
   {
       *d->flush_point = '\0';
       d->flush_point++;
   }
}

