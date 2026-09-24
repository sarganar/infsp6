! =====================================================================
! INFSP (Spanish Inform Library)
! =====================================================================
!        Author: Comunidad hispana
!       Version: 6.12
!      Released: 15 - Septiembre - 2026
! Serial Number: 260915
!          Note: For use with Inform 6. (Tested with Inform compiler
!                version 6.45 and Inform library 6/12).
!   Description: "An Inform library to write Interactive Fiction in
!                Spanish. INFSPR.h: Parsers Replacements/Hack."
!      Encoding: UTF-8
!       BaseLib: gitlab.com/DavidGriffith/inform6lib
! =====================================================================

 Message "Incluyendo INFSPR [INFSP 6.12]";
! Constant INFSPR_adv; !descomentar esta linea si se quiere tener los mensajes de 
                       ! reemplazo de rutinas hackeadas al compilar

! ------------------------------------
! Parser Replace Section
! ------------------------------------
! Rutinas Hackeadas:
!                   DictionaryLookup		BUG
!                   BestGuess               SP PATCH
!                   Identical               SP PATCH
!                   PrefaceByArticle        SP PATCH
!                   TryGivenObject          SP PATCH
!                   Indefart                SP PATCH
!                   ChangePlayer            SP PATCH



! Definicion de bufferaux y parseraux, usados en DictionaryLookup [001115]
#ifdef TARGET_ZCODE;
  Array bufferaux -> 123;
  Array parseaux  buffer 63;
#ifnot; ! TARGET_GLULX
  Array  bufferaux    buffer INPUT_BUFFER_LEN;
  Array parseaux  --> 1 + (MAX_BUFFER_WORDS * 3);
#endif; ! TARGET_



! Tremendo bug corregido. Antes usaba el buffer buf2, el cual usa
! tambien el parser para leer commandos incompletos del estilo de "¿a
! cual te refieres exactamente?"
! Este bug estaba en la libreria original, pero nunca aparecia porque
! apenas se usa DictionaryLookup. En cambio InformATE lo usa para
! comprobar si quitando la R a un verbo se obtiene una palabra válida.
!
! [Gracias a Presi por detectarlo]
!
#ifdef INFSPR_adv; Message "   Incluyendo reemplazo DictionaryLookup"; #endif;

[ DictionaryLookup texto longitud
    i;

 ! copiar texto a bufferaux (copiado especial)
 for (i=0:i<longitud:i++) bufferaux->(WORDSIZE+i) = texto->i;

 ! completar correctamente cabeceras de los buffers
 !	para la rutina Tokenise.
#ifdef TARGET_ZCODE;
  bufferaux->1 = longitud;
!  bufferaux->0 = longitud;
  bufferaux->0 = 120;
  parseaux-> 0 = 1;
#ifnot; ! TARGET_GLULX
  bufferaux-->0 = longitud;
#endif; ! TARGET_

            if (parser_trace>=9) { ! infsp debug
                print "    DictionaryLookup: texto a procesar: |";
                  ImprimeTodoElBuffer(bufferaux);
                print "|^";
            }
  Tokenise__(bufferaux,parseaux); ! procesar texto de bufferaux, el resultado está en parseaux-->1
  return parseaux-->1; ! retornar address del token encontrado (-1 si falla)
];


! ###############################################################################
! ----------------------------------------------------------------------------
!  BestGuess makes the best guess it can out of the match list, assuming
!  that everything in the match list is textually as good as everything else;
!  however it ignores items marked as -1, and so marks anything it chooses.
!  It returns -1 if there are no possible choices.
! ----------------------------------------------------------------------------

! Modificada en la revisión [020415] de modo que si hay varios objetos con la
! mayor puntuación, se elija uno de ellos al azar, en vez de elegir siempre al
! primero. Solo usado en I6. I7 usa uno mas elaborado.
#ifdef INFSPR_adv;Message "   Incluyendo reemplazo BestGuess"; #endif;
[ BestGuess  earliest its_score best i mejores aleat j;

  earliest=0; best=-1; mejores=0;
  for (i=0:i<number_matched:i++)
  {   if (match_list-->i >= 0)
      {   its_score=match_scores-->i;
          if (its_score>best) {
            best=its_score;
            earliest=i;
            mejores=1;
          }
          else if (its_score == best)
            mejores++;
      }
  }
  if (mejores > 1) {
    aleat = random (mejores);
    for (i = 0, j = 0: i < number_matched: i++)
    ! Antes se initializaba j = 1 (bug corregido en [020423])
    {
      if (match_list-->i >= 0)
      {
        its_score = match_scores-->i;
        if (its_score == best)
          j++;
        if (j == aleat)
        {
          earliest = i;
          break;
        }
      }
    }
  }
#ifdef DEBUG;
  if (parser_trace>=4)
  {   
      if (best < 0) print "   Best guess ran out of choices^";
      else print "   Best guess ", (the) match_list-->earliest, " (", match_list-->earliest, ")^";
  }
#endif;
  if (best<0) return -1;
  i=match_list-->earliest;
  match_list-->earliest=-1;
  bestguess_score = best;
  return i;
];

! ##############################################################################
! ----------------------------------------------------------------------------
!  Identical decides whether or not two objects can be distinguished from
!  each other by anything the player can type.  If not, it returns true.
! ----------------------------------------------------------------------------

! Modificado en la versión [020423] para comparar además de name, las otras
! propiedades de la librería española InformATE! que sirven también para nombrar
! los objetos: name_f, name_fp, name_mp y adjectives. Si se programan bien
! los juegos, con esto se puede solucionar el problema de la desambiguación.
#ifdef INFSPR_adv;Message "   Incluyendo reemplazo Identical"; #endif;
[ Identical o1 o2 p1 p2 n1 n2 i j flag pasada;

  if (o1==o2) rtrue;  ! This should never happen, but to be on the safe side
  if (o1==0 || o2==0) rfalse;  ! Yesmilarly
  if (parent(o1)==compass || parent(o2)==compass) rfalse; ! Saves time

!  What complicates things is that o1 or o2 might have a parsing routine,
!  so the parser can't know from here whether they are or aren't the same.
!  If they have different parsing routines, we simply assume they're
!  different.  If they have the same routine (which they probably got from
!  a class definition) then the decision process is as follows:
!
!     the routine is called (with self being o1, not that it matters)
!       with noun and second being set to o1 and o2, and action being set
!       to the fake action TheSame.  If it returns -1, they are found
!       identical; if -2, different; and if >=0, then the usual method
!       is used instead.

  if (o1.parse_name~=0 || o2.parse_name~=0) !ambos tienen propiedad parser_name?
  {   if (o1.parse_name ~= o2.parse_name) rfalse;
      parser_action=##TheSame; parser_one=o1; parser_two=o2;
      j=wn; i=RunRoutines(o1,parse_name); wn=j;
      if (i==-1) rtrue; if (i==-2) rfalse;
  }

!  This is the default algorithm: do they have the same words in their
!  "name" (i.e. property no. 1) properties.  (Note that the following allows
!  for repeated words and words in different orders.)

!  p1 = o1.&1; n1 = (o1.#1)/WORDSIZE;  ! Para mí, referirse a una propiedad
!  p2 = o2.&1; n2 = (o2.#1)/WORDSIZE;  ! con un número es una burrada

! ¿TODO? OJO: Este método compara si cada una de las propiedades de names y
! demás a buscar tiene las mismas palabras, pero entre ellas mismas (esto es,
! por ejemplo entre adjetivos de o1 y de o2), no entre sí (como, por ejemplo,
! entre las de name de o1 y adjetivos de o2).
!
! Posiblemente lo ideal fuera mirar si todas las palabras de todas estas
! propiedades de o1 están en las propiedades de o2 y viceversa.

! pasada almacenará el número de pasada por el que se va, para saber si toca
! comparar name(1), adjetivos(2), name_f(3), name_mp(4) o name_fp(5)  

  for (pasada = 1: pasada < 6: pasada++) {
    switch (pasada) {
      1:
        p1 = o1.&name; n1 = (o1.#name)/WORDSIZE;
        p2 = o2.&name; n2 = (o2.#name)/WORDSIZE;
      2:
        p1 = o1.&adjectives; n1 = (o1.#adjectives)/WORDSIZE;
        p2 = o2.&adjectives; n2 = (o2.#adjectives)/WORDSIZE;
      3:
        p1 = o1.&name_f; n1 = (o1.#name_f)/WORDSIZE;
        p2 = o2.&name_f; n2 = (o2.#name_f)/WORDSIZE;
      4:
        p1 = o1.&name_mp; n1 = (o1.#name_mp)/WORDSIZE;
        p2 = o2.&name_mp; n2 = (o2.#name_mp)/WORDSIZE;
      5:
        p1 = o1.&name_fp; n1 = (o1.#name_fp)/WORDSIZE;
        p2 = o2.&name_fp; n2 = (o2.#name_fp)/WORDSIZE;
    }  ! del switch

    !  for (i=0:i<n1:i++) { print (address) p1-->i, " "; } new_line;
    !  for (i=0:i<n2:i++) { print (address) p2-->i, " "; } new_line;

    for (i = 0: i < n1: i++)
    {
      flag = 0;
      for (j = 0: j < n2: j++)
        if (p1-->i == p2-->j)
          flag = 1;
      if (flag == 0) rfalse;
    }

    for (j = 0: j < n2: j++)
    {
      flag = 0;
      for (i = 0: i < n1: i++)
        if (p1-->i == p2-->j)
          flag = 1;
      if (flag == 0) rfalse;
    }
  }  ! del for

!  print "Which are identical!^";
  rtrue;
];


! ##############################################################################
! PrefaceByArticle actualizado a la 6/11, con hackeos de genero (a partir de INFSP0.8f)

#ifdef INFSPR_adv;Message "   Incluyendo reemplazo PrefaceByArticle"; #endif;
[ PrefaceByArticle o acode pluralise capitalise  i artform findout artval;
    if (o provides articles) {
        artval=(o.&articles)-->(acode+short_name_case*LanguageCases);
        if (artval) { ! infsp hack para admitir valor 0 en slot de 'articles' property.
         if (capitalise)
            print (Cap) artval, " ";
         else
            print (string) artval, " ";
        }
        if (pluralise) return;
        print (PSN__) o; return;
    }

   if (o provides gender){ ![infsp]'gender'(informATE exclusive) es el genero de short_name del objeto
    i=o.gender;
    if (i==1) ! infsp : esto es una chanchada, pero al cambiarse los valores de 'gender' por lo de la 
       i=0;   ! compatibilidad con I7 (donde no puede ponerse gender = 0), quedo desfasado el array
    if (i==2) ! LanguageGNAsToArticles. TODO: correjir el array y testear estabilidad.
       i=1;
   }else{ i = GetGNAOfObject(o);}

!    print "^pluralise:",pluralise; ![infsp] debug
!    print "^1 valor de i:",i; ![infsp] debug

! [infsp]: para q funcione con (el_), se agrego '>0' para el correcto funcionamiento (sino al final imprime el nombre de objeto cuando yo solo quiero imprimir su articulo)
    if (pluralise>0) { ![INFSP] ej, en ListMiscellany 19,[TODO] esto podria resolverse como los italianos
        if (i < 3 || (i >= 6 && i < 9)) i = i + 3;
    }
!    print "^2 valor de i:",i; ![infsp] debug
    i = LanguageGNAsToArticles-->i;
!    print "^3 valor de i:",i," ";! infsp debug
    
    artform = LanguageArticles
        + 3*WORDSIZE*LanguageContractionForms*(short_name_case + i*LanguageCases);

    #Iftrue (LanguageContractionForms == 2);
    if (artform-->acode ~= artform-->(acode+3)) findout = true;
    #Endif; ! LanguageContractionForms
    #Iftrue (LanguageContractionForms == 3);
    if (artform-->acode ~= artform-->(acode+3)) findout = true;
    if (artform-->(acode+3) ~= artform-->(acode+6)) findout = true;
    #Endif; ! LanguageContractionForms
    #Iftrue (LanguageContractionForms == 4);
    if (artform-->acode ~= artform-->(acode+3)) findout = true;
    if (artform-->(acode+3) ~= artform-->(acode+6)) findout = true;
    if (artform-->(acode+6) ~= artform-->(acode+9)) findout = true;
    #Endif; ! LanguageContractionForms
    #Iftrue (LanguageContractionForms > 4);
    findout = true;
    #Endif; ! LanguageContractionForms

    #Ifdef TARGET_ZCODE;
    if (standard_interpreter ~= 0 && findout) {
        StorageForShortName-->0 = SHORTNAMEBUF_LEN;
        @output_stream 3 StorageForShortName;
        if (pluralise) print (number) pluralise; else print (PSN__) o;
        @output_stream -3;
        acode = acode + 3*LanguageContraction(StorageForShortName + 2);
    }
    #Ifnot; ! TARGET_GLULX
    if (findout) {
        if (pluralise)
            PrintAnyToArray(StorageForShortName, SHORTNAMEBUF_LEN, EnglishNumber, pluralise);
        else
            PrintAnyToArray(StorageForShortName, SHORTNAMEBUF_LEN, PSN__, o);
        acode = acode + 3*LanguageContraction(StorageForShortName);
    }
    #Endif; ! TARGET_

    Cap (artform-->acode, ~~capitalise); ! print article
    if (pluralise) return;
    print (PSN__) o;!imprime el nombre del objeto
];
! #############################################################################
! ----------------------------------------------------------------------------
!  TryGivenObject tries to match as many words as possible in what has been
!  typed to the given object, obj.  If it manages any words matched at all,
!  it calls MakeMatch to say so, then returns the number of words (or 1
!  if it was a match because of inadequate input).
! ----------------------------------------------------------------------------
#ifdef INFSPR_adv;Message "   Incluyendo reemplazo TryGivenObject"; #endif;
[ TryGivenObject obj threshold k w j;
    #Ifdef DEBUG;
    if (parser_trace >= 5) print "    Trying ", (the) obj, " (", obj, ") at word ", wn, "^";
    #Endif; ! DEBUG

    dict_flags_of_noun = 0;

!  If input has run out then always match, with only quality 0 (this saves
!  time).

    if (wn > num_words) {
        if (indef_mode ~= 0)
            dict_flags_of_noun = DICT_X654;  ! Reject "plural" bit
        MakeMatch(obj,0);

        give obj ~nombreusado; ![INFSP] hack: evitar que un objeto no mencionado en este turno sea elejido (Related: ChooseObjects/ElijeObjetos)

        #Ifdef DEBUG;
        if (parser_trace >= 5) print "    Matched (0)^";
        #Endif; ! DEBUG
        return 1;
    }

!  Ask the object to parse itself if necessary, sitting up and taking notice
!  if it says the plural was used:

    if (obj.parse_name~=0) {
        parser_action = NULL; j=wn;
        k = RunRoutines(obj,parse_name);

        if (k > 0) {

            give obj nombreusado; ! infsp hack, I7: cuando coinciden en el parse_name
                                  !   sino no pueden usarse adjetivos para desambiguar
            wn=j+k;

          .MMbyPN;

            if (parser_action == ##PluralFound)
                dict_flags_of_noun = dict_flags_of_noun | DICT_PLUR;

            if (dict_flags_of_noun & DICT_PLUR) {
                if (~~allow_plurals) k = 0;
                else {
                    if (indef_mode == 0) {
                        indef_mode = 1; indef_type = 0; indef_wanted = 0;
                    }
                    indef_type = indef_type | PLURAL_BIT;
                    if (indef_wanted == 0) indef_wanted = 100;
                }
            }

            #Ifdef DEBUG;
            if (parser_trace >= 5) print "    Matched (", k, ")^";
            #Endif; ! DEBUG
            MakeMatch(obj,k);
            return k;
        }
        if (k == 0) jump NoWordsMatch;
        wn = j;
    }

    ! The default algorithm is simply to count up how many words pass the
    ! Refers test:

    parser_action = NULL;

    w = NounWord();

    if (w == 1 && player == obj) { k=1; jump MMbyPN; }

    if (w >= 2 && w < 128 && (LanguagePronouns-->w == obj)) { k = 1; jump MMbyPN; }

    j = --wn;
    threshold = ParseNoun(obj);
    if (threshold == -1) {
        LibraryExtensions.ext_number_1 = wn;    ! Set the "between calls" functionality to
        LibraryExtensions.BetweenCalls = LibraryExtensions.RestoreWN;
        threshold = LibraryExtensions.RunWhile(ext_parsenoun, -1, obj);
        LibraryExtensions.BetweenCalls = 0;     ! Turn off the "between calls" functionality
    }
    #Ifdef DEBUG;
    if (threshold >= 0 && parser_trace >= 5) print "    ParseNoun returned ", threshold, "^";
    #Endif; ! DEBUG
    ! Don't arbitrarily increase wn when ParseNoun() returns -1
    if (threshold > 0) {
        k = threshold;
        wn = j + k;
        jump MMbyPN;
    }
    ! Check wn instead of wn - 1
    if (threshold == 0 || Refers(obj,wn) == 0) {
      .NoWordsMatch;
        if (indef_mode ~= 0) {
            ! Restore wn to pre-ParseNoun() state
            k = 0; parser_action = NULL; wn = j;
            jump MMbyPN;
        }
        rfalse;
    }

    if (threshold < 0) {
        ! Set threshold to reflect any words consumed by ParseNoun()
        threshold = wn - j;
        w = NextWord();  ! Ensure w contains actual first word of noun phrase
                          ! if ParseNoun() moved wn.
        dict_flags_of_noun = (w->#dict_par1) & (DICT_X654+DICT_PLUR);
        while (Refers(obj, wn-1)) {
            threshold++;
            if (w)
               dict_flags_of_noun = dict_flags_of_noun | ((w->#dict_par1) & (DICT_X654+DICT_PLUR));
            w = NextWord();
        }
    }

    k = threshold;
    jump MMbyPN;
];


! #############################################################################
! [infsp] AskPlayer: se toma tal cual de inform6lib (Keyboard loop, deteccion
! de "PNJ, verbo" como respuesta, GetKeyBufLength/SetKeyBufLength), salvo el
! bloque que arma la pregunta ("Cual concretamente, X o Y?"). Ahi SI hay un
! patch real de INFSP que hay que preservar (a diferencia de NounDomain, que
! no tenia ninguno -- ver docs/sync-progreso.md item #10/#11): las Mensajes
! 45/46 de LanguageLM llaman a ImprimirListaDudosos(), que filtra la lista
! por match_scores para no repetir clases empatadas irrelevantes ("mas
! eficiente al listar solo los objetos realmente relevantes", comentario
! original de INFSP). El AskPlayer de inform6lib espera en cambio que esas
! Mensajes NO impriman la lista (el nunca la imprime el mismo, con un loop
! propio mas simple que no filtra por score) -- dejarlo tal cual duplicaba
! la lista completa ("la moneda de oro o la moneda de plata" DOS veces),
! bug real encontrado al verificar esto con dfrotz. Se saca el loop propio
! de inform6lib y se deja que ImprimirListaDudosos() siga haciendo el
! trabajo, exactamente como en el NounDomain viejo.
#ifdef INFSPR_adv;Message "   Incluyendo reemplazo AskPlayer"; #endif;
[ AskPlayer context  i j k l first_word answer_words;
    asking_player = true;
    if (context == CREATURE_TOKEN) L__M(##Miscellany, 45);
    else                           L__M(##Miscellany, 46);
    L__M(##Miscellany, 57);

    ! ...and get an answer:

  .WhichOne;
    #Ifdef TARGET_ZCODE;
    for (i=WORDSIZE : i<INPUT_BUFFER_LEN : i++) buffer2->i = ' ';
    #Endif; ! TARGET_ZCODE
    answer_words = Keyboard(buffer2, parse2);

    first_word = WordValue(1, parse2);
    asking_player = false;

    ! Take care of "all", because that does something too clever here to do
    ! later on:

    if (first_word == ALL1__WD or ALL2__WD or ALL3__WD or ALL4__WD or ALL5__WD) {
        if (context == MULTI_TOKEN or MULTIHELD_TOKEN or MULTIEXCEPT_TOKEN or MULTIINSIDE_TOKEN) {
            l = multiple_object-->0;
            for (i=0 : i<number_matched && l+i<63 : i++) {
                k = match_list-->i;
                multiple_object-->(i+1+l) = k;
            }
            multiple_object-->0 = i+l;
            rtrue;
        }
        L__M(##Miscellany, 47);
        jump WhichOne;
    }

    ! If the first word of the reply can be interpreted as a verb, then
    ! assume that the player has ignored the question and given a new
    ! command altogether.
    ! (This is one time when it's convenient that the directions are
    ! not themselves verbs - thus, "north" as a reply to "Which, the north
    ! or south door" is not treated as a fresh command but as an answer.)

    #Ifdef LanguageIsVerb;
    if (first_word == 0) {
        j = wn; first_word = LanguageIsVerb(buffer2, parse2, 1); wn = j;
    }
    #Endif; ! LanguageIsVerb
    if (first_word) {
        if (((first_word->#dict_par1) & DICT_VERB) && ~~LanguageVerbMayBeName(first_word)) {
            #Ifdef DEBUG;
            if (parser_trace>=9) { ! [INFSP] for debugging proposes
                print "Copiando buffer2: |";
                    ImprimeTodoElBuffer(buffer2);
                print "|^   en buffer: |";
                    ImprimeTodoElBuffer(buffer);
                print "|^";
            }
            #Endif; ! DEBUG
            CopyBuffer(buffer, buffer2);
            return REPARSE_CODE;
        }
        if (NumberWords(parse2) > 2) {
            j = WordValue(2, parse2);
            k = WordValue(3, parse2);
            if (j == ',//' && k && (k->#dict_par1) & DICT_VERB) {
                CopyBuffer(buffer, buffer2);
                return REPARSE_CODE;
            }
        }
    }

    ! Now we insert the answer into the original typed command, as
    ! words additionally describing the same object
    ! (eg, > take red button
    !      Which one, ...
    !      > music
    ! becomes "take music red button".  The parser will thus have three
    ! words to work from next time, not two.)

    k = WordAddress(match_from) - buffer;
    l = GetKeyBufLength(buffer2) +1;
    for (j=buffer + INPUT_BUFFER_LEN - 1 : j>=buffer+k+l : j--) j->0 = j->(-l);
    for (i=0 : i<l : i++) buffer->(k+i) = buffer2->(WORDSIZE+i);
    buffer->(k+l-1) = ' ';
    SetKeyBufLength(GetKeyBufLength() + l);

    ! Having reconstructed the input, we warn the parser accordingly
    ! and get out.

    return REPARSE_CODE;
];


! #############################################################################
! [infsp] Indefart: se agregó el "a " en caso de nombre propio " a Mamá"
#ifdef INFSPR_adv;Message "   Incluyendo reemplazo Indefart"; #endif;
[ Indefart o i; ! funcion (a)
    if (o == 0) { print (string) NOTHING__TX; rtrue; }
    i = indef_mode; indef_mode = true;
    if (o has proper) { indef_mode = NULL; print "a ",(PSN__) o; indef_mode = i; return; }
    if (o provides article) {
        PrintOrRun(o, article, 1); print " ", (PSN__) o; indef_mode = i;
        return;
    }
    PrefaceByArticle(o, 2); indef_mode = i;
];



! #############################################################################
! [infsp] ChangePlayer: se toma tal cual de inform6lib/parser.h (visibilidad
! transparente del nuevo cuerpo, reubicacion de location/lightflag, el
! bloque que fija el short_name de "mi antiguo yo" si el player saliente era
! selfobj). El UNICO patch real es la linea "AsignarPersona();" agregada
! justo despues de "player = obj;" -- backlog #4 (2026-09-17): el fork de
! Ricpelo (infsp6_fix_byRicpelo/INFSPR.h) ya llamaba a su version de
! AsignarPersona (con un argumento 'persona' propio) automaticamente aca
! cada vez que el juego cambiaba de objeto-jugador; este puerto no lo tenia
! porque se parte del ChangePlayer generico de inform6lib, sin ese hook, asi
! que un juego que cambiara de cuerpo tenia que acordarse de llamar
! AsignarPersona() a mano. AsignarPersona() ya es segura de llamar sin
! condicion (hace "if (~~(player provides narrative_voice)) return;" al
! entrar), asi que no hace falta ningun chequeo extra aca.
#ifdef INFSPR_adv;Message "   Incluyendo reemplazo ChangePlayer"; #endif;
[ ChangePlayer obj flag i;
    if (obj == nothing) obj = selfobj;
    if (actor == player) actor=obj;
    give player ~transparent ~concealed;
    i = obj; while (parent(i) ~= 0) {
        if (i has animate) give i transparent;
        i = parent(i);
    }
    if (player == selfobj && player provides nameless && player.nameless == true) {
        if (player provides narrative_voice) {
            if (player.narrative_voice == 1) {
                player.short_name = MYFORMER__TX;
                (player.&name)-->0 = 'my';
                (player.&name)-->1 = 'former';
                (player.&name)-->2 = 'self';
            } else if (player.narrative_voice == 2) {
                player.short_name = FORMER__TX;
                (player.&name)-->0 = 'my';
                (player.&name)-->1 = 'former';
                (player.&name)-->2 = 'self';
            }
        }
    }

    player = obj;

    ! [2026-09-17 backlog #4] unico patch real de esta rutina -- ver
    ! comentario de arriba.
    AsignarPersona();

    give player transparent concealed animate;
    i = player; while (parent(i) ~= 0) i = parent(i);
    location = i; real_location = location;
    if (parent(player) == 0) return RunTimeError(10);
    MoveFloatingObjects();
    lightflag = OffersLight(parent(player));
    if (lightflag == 0) location = thedark;
    print_player_flag = flag;
];


! ------------------------------------
! Verlib Replace Section
! ------------------------------------

! Nada por ahora

! Fin del archivo INFSPR
